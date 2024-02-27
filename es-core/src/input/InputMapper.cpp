//
// Created by bkg2k on 15/01/2021.
//
// As part of the RECALBOX Project
// http://www.recalbox.com
//

#include <input/InputMapper.h>
#include <input/InputManager.h>

InputMapper::~InputMapper()
{
  SaveConfiguration();
}

void InputMapper::Build()
{
  LoadConfiguration();
  // Copy to pad active list
  PadList activePads(AvailablePads());

  // Remove device already fetched from configuration
  // Also add non-connected pads to the phantom list
  for(Pad& device : mPads)
    if (device.IsValid())
    {
      bool found = false;
      for(int i = 0; i < (int)activePads.size(); ++i)
        if (const Pad& connectedDevice = activePads[i]; connectedDevice.Same(device))
        {
          found = true;
          device.Copy(connectedDevice);
          activePads.erase(activePads.begin() + i);
          break;
        }
      // Device not found, move to unconnected list
      if (!found)
        { LOG(LogDebug) << "[PadMapping] Move to unconnected: " << device.AsString(); }
    }

  // Add remaining pads in unused slots
  bool assignNew = false;
  for(const Pad& connectedDevice : activePads)
    for (Pad& device : mPads)
      if (!device.IsValid())
      {
        { LOG(LogDebug) << "[PadMapping] Add connected to the list: " << connectedDevice.AsString(); }
        device.Set(connectedDevice.mName, connectedDevice.mUUID, connectedDevice.mPath, connectedDevice.mIndex);
        assignNew = true;
        break;
      }

  // Assigned? If not, take the slot from the first non connected pad
  if (!activePads.empty() && !assignNew)
    for(const Pad& connectedDevice : activePads)
      for (Pad& device : mPads)
        if (!device.IsConnected())
        {
          { LOG(LogDebug) << "[PadMapping] Add connected to the list (took unconnected slot): " << connectedDevice.AsString(); }
          device.Set(connectedDevice.mName, connectedDevice.mUUID, connectedDevice.mPath, connectedDevice.mIndex);
          assignNew = true;
          break;
        }

  // Assign position in mapper list
  { LOG(LogDebug) << "[PadMapping] Assign positions"; }
  AssignPositions();


  int index = 0;
  for(const Pad& pad : mPads)
  { LOG(LogDebug) << "[PadMapping] Pad @" << ++index << " = " << pad.AsString(); }

  // Save config
  if (assignNew)
    SaveConfiguration();
}

void InputMapper::LoadConfiguration()
{
  // Fill from configuration
  String uuid;
  String name;
  for(int i = Input::sMaxInputDevices; --i >= 0; )
    mPads[i].Reset();
  for(int i = Input::sMaxInputDevices; --i >= 0; )
    if (RecalboxConf::Instance().GetPad(i).Extract(':', uuid, name, true))
    {
      mPads[i].Set(name, uuid, Path::Empty, -1);
      { LOG(LogDebug) << "[PadMapping] Load pad @" << i << " = " << mPads[i].AsString(); }
    }
}

void InputMapper::SaveConfiguration()
{
  for(int i = Input::sMaxInputDevices; --i >= 0;)
    if (mPads[i].IsValid())
    {
      RecalboxConf::Instance().SetPad(i, String(mPads[i].mUUID).Append(':').Append(mPads[i].mName));
      { LOG(LogDebug) << "[PadMapping] Save pad @" << i << " = " << mPads[i].AsString(); }
    }
  RecalboxConf::Instance().Save();
}

InputMapper::PadList InputMapper::AvailablePads()
{
  PadList result;
  int count = InputManager::Instance().DeviceCount();
  for(int i = 0; i < count; ++i)
  {
    const InputDevice& device = InputManager::Instance().GetDeviceConfigurationFromIndex(i);
    if (device.ButtonCount() == 0) continue; // Not a true pad
    result.push_back(Pad(device.Name().ToTrim(), device.GUID(), device.UDevPath(), device.Index()));
    { LOG(LogDebug) << "[PadMapping] Available pad @" << i << " = " << result.back().AsString(); }
  }
  return result;
}

String InputMapper::GetDecoratedName(int position)
{
  int count = 0;
  const Pad& pad = mPads[position];
  if (pad.mIndex >= 0)
    for(const Pad& current : mPads)
      if (current.Same(pad))
        if (current.mIndex < pad.mIndex && !current.mPath.IsEmpty())
          count++;

  String result(pad.mName);
  if (count > 0) result.Append(LEGACY_STRING(" #")).Append(count + 1);
  if (String batteryIcon = pad.LookupPowerLevel(); !batteryIcon.empty())
    result.Append(' ').Append(batteryIcon);
  result.Insert(0, pad.mIndex < 0 ? "\u26aa" : "\u26ab")
        .Append(pad.mIndex < 0 ? "\u26aa" : "\u26ab");
  return pad.IsConnected() ? result : String::Empty;
}

void InputMapper::Swap(int position1, int position2)
{
  // Clamp indexes
  position1 = Math::clampi(position1, 0, Input::sMaxInputDevices - 1);
  position2 = Math::clampi(position2, 0, Input::sMaxInputDevices - 1);

  // Really swap?
  if ((position1 == position2) || (position1 < 0) || (position2 < 0)) return;

  // Swap
  Pad tmp          = mPads[position1];
  mPads[position1] = mPads[position2];
  mPads[position2] = tmp;
  mPads[position1].mPosition = position1;
  mPads[position2].mPosition = position2;

  // Save
  SaveConfiguration();
}

void InputMapper::PadsAddedOrRemoved(bool removed)
{
  (void)removed;
  // Rebuild all when a pad has been added or removed
  Build();
}

int InputMapper::PadIndexFromDeviceIdentifier(SDL_JoystickID identifier)
{
  int sdlIndex = InputManager::Instance().GetDeviceIndexFromId(identifier);
  int realIndex = 0;
  if (sdlIndex >= 0)
    for(int i = Input::sMaxInputDevices; --i >= 0; )
      if (const Pad& pad = mPads[i]; pad.IsConnected())
      {
        if (pad.mIndex == sdlIndex) return realIndex;
        ++realIndex;
      }
  return -1;
}

int InputMapper::ConnectedPadCount() const
{
  int result = 0;
  for(int i = Input::sMaxInputDevices; --i >= 0; )
    if (const Pad& pad = mPads[i]; pad.IsConnected())
      result++;
  return result;
}

InputMapper::PadList InputMapper::GetPads() const
{
  PadList list;
  for(const Pad& pad : mPads)
    if (pad.IsConnected())
      list.push_back(pad);
  return list;
}

void InputMapper::AssignPositions()
{
  for(int i = Input::sMaxInputDevices; --i >= 0; )
    mPads[i].mPosition = i;
}

String InputMapper::Pad::LookupPowerLevel() const
{
  if (mIndex >= 0) return String((String::Unicode )InputManager::Instance().GetDeviceConfigurationFromIndex(mIndex).BatteryLevelIcon(), 1);
  return "";
}

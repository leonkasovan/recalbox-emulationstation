#include "InputDevice.h"
#include "InputEvent.h"
#include "utils/udev/UDevDevice.h"
#include "utils/udev/UDev.h"
#include "utils/udev/UDevEnumerate.h"
#include <utils/String.h>
#include <SDL2/SDL.h>
#include <linux/input.h>
#include <fcntl.h>
#include <unistd.h>
#include <utils/Files.h>

unsigned int InputDevice::mReferenceTimes[32];

String InputDevice::EntryToString(InputDevice::Entry entry)
{
  switch(entry)
  {
    case InputDevice::Entry::None: return "none";
    case InputDevice::Entry::Start: return "start";
    case InputDevice::Entry::Select: return "select";
    case InputDevice::Entry::Hotkey: return "hotkey";
    case InputDevice::Entry::A: return "a";
    case InputDevice::Entry::B: return "b";
    case InputDevice::Entry::X: return "x";
    case InputDevice::Entry::Y: return "y";
    case InputDevice::Entry::L1: return "l1";
    case InputDevice::Entry::R1: return "r1";
    case InputDevice::Entry::L2: return "l2";
    case InputDevice::Entry::R2: return "r2";
    case InputDevice::Entry::L3: return "l3";
    case InputDevice::Entry::R3: return "r3";
    case InputDevice::Entry::Up: return "up";
    case InputDevice::Entry::Right: return "right";
    case InputDevice::Entry::Down: return "down";
    case InputDevice::Entry::Left: return "left";
    case InputDevice::Entry::Joy1AxisH: return "joystick1left";
    case InputDevice::Entry::Joy1AxisV: return "joystick1up";
    case InputDevice::Entry::Joy2AxisH: return "joystick2left";
    case InputDevice::Entry::Joy2AxisV: return "joystick2up";
    case InputDevice::Entry::VolumeUp: return "vol+";
    case InputDevice::Entry::VolumeDown: return "vol-";
    case InputDevice::Entry::BrightnessUp: return "lum+";
    case InputDevice::Entry::BrightnessDown: return "lum-";
    case InputDevice::Entry::__Count:
    default:
    {
      { LOG(LogError) << "[InputDevice] Unknown int Input entry: " << (int)entry; }
      return "error";
    }
  }
}

InputDevice::Entry InputDevice::StringToEntry(const String& entry)
{
  static HashMap<String, InputDevice::Entry> sStringToEntry
  {
    { "none", InputDevice::Entry::None },
    { "start", InputDevice::Entry::Start },
    { "select", InputDevice::Entry::Select },
    { "hotkey", InputDevice::Entry::Hotkey },
    { "a", InputDevice::Entry::A },
    { "b", InputDevice::Entry::B },
    { "x", InputDevice::Entry::X },
    { "y", InputDevice::Entry::Y },
    { "l1", InputDevice::Entry::L1 },
    { "r1", InputDevice::Entry::R1 },
    { "l2", InputDevice::Entry::L2 },
    { "r2", InputDevice::Entry::R2 },
    { "l3", InputDevice::Entry::L3 },
    { "r3", InputDevice::Entry::R3 },
    { "up", InputDevice::Entry::Up },
    { "right", InputDevice::Entry::Right },
    { "down", InputDevice::Entry::Down },
    { "left", InputDevice::Entry::Left },
    { "joystick1left", InputDevice::Entry::Joy1AxisH },
    { "joystick1up", InputDevice::Entry::Joy1AxisV },
    { "joystick2left", InputDevice::Entry::Joy2AxisH },
    { "joystick2up", InputDevice::Entry::Joy2AxisV },
    { "vol+", InputDevice::Entry::VolumeUp },
    { "vol-", InputDevice::Entry::VolumeDown },
    { "lum+", InputDevice::Entry::BrightnessUp },
    { "lum-", InputDevice::Entry::BrightnessDown },
  };
  InputDevice::Entry* result = sStringToEntry.try_get(entry.ToLowerCase());
  if (result != nullptr) return *result;
  { LOG(LogError) << "[InputDevice] Unknown string Input entry: " << entry; }
  return InputDevice::Entry::None;
}

InputDevice::InputDevice(SDL_Joystick* device, SDL_JoystickID deviceId, int deviceIndex, const String& deviceName, const SDL_JoystickGUID& deviceGUID, int deviceNbAxes, int deviceNbHats, int deviceNbButtons)
  : mDeviceName(deviceName)
  , mDeviceGUID(deviceGUID)
  , mDeviceSDL(device)
  , mLastTimeBatteryCheck(0)
  , mBatteryLevel(0)
  , mDeviceId(deviceId)
  , mDeviceIndex(deviceIndex)
  , mDeviceNbAxes(deviceNbAxes)
  , mDeviceNbHats(deviceNbHats)
  , mDeviceNbButtons(deviceNbButtons)
  , mConfigurationBits(0)
  , mPreviousHatsValues { 0 }
  , mPreviousAxisValues { 0 }
  , mNeutralAxisValues { 0 }
  , mBatteryCharging(false)
  , mIsATruePad(false)
  , mConfiguring(false)
  , mHotkeyState(false)
  , mKillSelect(false)
{
  #ifdef SDL_JOYSTICK_IS_OVERRIDEN_BY_RECALBOX
  mPath = SDL_JoystickDevicePathById(deviceIndex);
  #else
    #ifdef _RECALBOX_PRODUCTION_BUILD_
      #pragma GCC error "SDL_JOYSTICK_IS_OVERRIDEN_BY_RECALBOX undefined in production build!"
    #endif
    mPath = LookupPath();
  #endif

  mUDevDeviceName = UDevString(deviceIndex, UDevInfo::LowlevelName); // Default SDL2 name
  mUDevUniqID = UDevString(deviceIndex, UDevInfo::UniqueID);
  mUDevPowerPath = Path(UDevString(deviceIndex, UDevInfo::PowerPath));

  memset(mPreviousAxisValues, 0, sizeof(mPreviousAxisValues));
  memset(mPreviousHatsValues, 0, sizeof(mPreviousHatsValues));

  RecordAxisNeutralPosition();
}

void InputDevice::ClearAll()
{
  for(int i = (int)Entry::__Count; --i >= 0;)
    mInputEvents[i] = InputEvent();
  mConfigurationBits = 0;
}

void InputDevice::LoadFrom(const InputDevice& source)
{
  *this = source;
}

String InputDevice::NameExtented()
{
  String result(Name());
  String::Unicode powerLevel = BatteryLevelIcon();
  if (powerLevel != 0)
    result.Append(' ').AppendUTF8(powerLevel);
  return result;
}

int InputDevice::BatteryLevel()
{
  if (!mUDevPowerPath.IsEmpty())
  {
    if (int now = (int)SDL_GetTicks(); now - mLastTimeBatteryCheck > 10000)
    {
      mLastTimeBatteryCheck = now;
      mBatteryCharging = Files::LoadFile(mUDevPowerPath / "status").Trim() == "Charging";
      int level = 0;
      if (Files::LoadFile(mUDevPowerPath / "capacity").Trim().TryAsInt(level)) return mBatteryLevel = level;
    }
    return mBatteryLevel;
  }
  return -1; // Unknown / unavailable
}

String::Unicode InputDevice::BatteryLevelIcon()
{
  if (mBatteryCharging) return 0xF1b4; // in charge
  int level = BatteryLevel();
  if (level <    0) return 0;      // Unknown / Wired
  if (level == 100) return 0xF1ba; // Max
  if (level >   80) return 0xF1b7; // Full
  if (level >   40) return 0xF1b8; // Medium
  if (level >   15) return 0xF1b1; // Low
  return 0xF1b5;                   // Empty!
}

void InputDevice::Set(Entry input, InputEvent event)
{
  mInputEvents[(int)input] = event;
  mConfigurationBits |= 1 << (int)input;
}

void InputDevice::Unset(Entry input)
{
  mInputEvents[(int)input] = InputEvent();
  mConfigurationBits &= ~(1 << (int)input);
}

bool InputDevice::GetEntryConfiguration(Entry input, InputEvent& result) const
{
  if ((mConfigurationBits & (1 << (int)input)) != 0)
  {
    result = mInputEvents[(int)input];
    return true;
  }
  return false;
}

bool InputDevice::IsMatching(Entry input, InputEvent event) const
{
  if ((mConfigurationBits & (1 << (int)input)) != 0)
  {
    const InputEvent& comp = mInputEvents[(int) input];
    if (comp.Type() == event.Type() && comp.Id() == event.Id())
    {
      switch(comp.Type())
      {
        case InputEvent::EventType::Button:
        case InputEvent::EventType::Key: return true;
        case InputEvent::EventType::Hat: return (event.Value() == 0 || ((event.Value() & comp.Value()) != 0));
        case InputEvent::EventType::Axis: return event.Value() == 0 || comp.Value() == event.Value();
        case InputEvent::EventType::MouseWheel:
        case InputEvent::EventType::MouseButton:
        case InputEvent::EventType::Unknown:
        default: break;
      }
    }
  }
  return false;
}

InputDevice::Entry InputDevice::GetMatchedEntry(InputEvent input) const
{
  std::vector<String> maps;

  for (int i = (int)Entry::__Count; --i >= 0; )
    if ((mConfigurationBits & (1 << (int)i)) != 0)
    {
      const InputEvent& chk = mInputEvents[i];

      if(chk.Device() == input.Device() && chk.Type() == input.Type() && chk.Id() == input.Id())
      {
        if(chk.Type() == InputEvent::EventType::Hat)
        {
          if (input.Value() == 0 || ((input.Value() & chk.Value()) != 0))
            return (Entry) i;
        }
        else if(input.Type() == InputEvent::EventType::Axis)
        {
          if(input.Value() == 0 || chk.Value() == input.Value())
            return (Entry)i;
        }
        else
        {
          return (Entry)i;
        }
      }
    }

  return Entry::None;
}

bool InputDevice::LoadAutoConfiguration(const String& configuration)
{
  bool result = false;

  { LOG(LogInfo) << "[InputDevice] Autoconfiguration from " << configuration; }
  String::List mappingList = configuration.Split(',');
  for(const String& mapping : mappingList)
    if (String key, value; mapping.Extract(':', key, value, false))
    {
      static HashMap<String, Entry> mapper
      ({
         { "dpup",          Entry::Up },
         { "dpright",       Entry::Right },
         { "dpdown",        Entry::Down },
         { "dpleft",        Entry::Left },
         { "a",             Entry::B }, // A <=> B and X <=> Y to match ES buttons.
         { "b",             Entry::A },
         { "x",             Entry::Y },
         { "y",             Entry::X },
         { "back",          Entry::Select },
         { "start",         Entry::Start },
         { "guide",         Entry::Hotkey },
         { "leftshoulder",  Entry::L1 },
         { "rightshoulder", Entry::R1 },
         { "lefttrigger",   Entry::L2 },
         { "righttrigger",  Entry::R2 },
         { "leftstick",     Entry::L3 },
         { "rightstick",    Entry::R3 },
         { "leftx",         Entry::Joy1AxisH },
         { "lefty",         Entry::Joy1AxisV },
         { "rightx",        Entry::Joy2AxisH },
         { "righty",        Entry::Joy2AxisV },
      });

      Entry* entry = mapper.try_get(key);
      if (entry == nullptr || value.length() < 2)
      {
        { LOG(LogError) << "[InputDevice] Unknown mapping: " << mapping; }
        continue;
      }

      // Default values
      InputEvent::EventType typeEnum = InputEvent::EventType::Unknown;
      int id = -1;
      int val = 0;
      int code = -1;
      int sign = 0;
      int parserIndex = 0;

      // Sign?
      switch(value[parserIndex])
      {
        case '-': { sign = -1; ++parserIndex; break; }
        case '+': { sign = +1; ++parserIndex; break; }
        default: break;
      }

      bool parsed = false;
      switch(value[parserIndex])
      {
        case 'a':
        {
          if (value.TryAsInt(parserIndex + 1, id))
          {
            typeEnum = InputEvent::EventType::Axis;
            if (sign == 0) val = -1; // non-signed axes are affected to joysticks: always left or up
            else val = sign;         // Otherwise, take the sign as-is
            #ifdef SDL_JOYSTICK_IS_OVERRIDEN_BY_RECALBOX
              code = SDL_JoystickAxisEventCodeById(mDeviceIndex, id);
            #else
              #ifdef _RECALBOX_PRODUCTION_BUILD_
                #pragma GCC error "SDL_JOYSTICK_IS_OVERRIDEN_BY_RECALBOX undefined in production build!"
              #endif
            #endif
            parsed = true;
            { LOG(LogInfo) << "[InputDevice] Auto-Assign " << EntryToString(*entry) << " to Axis " << id << " (Code: " << code << ')'; }
          }
          break;
        }
        case 'b':
        {
          if (value.TryAsInt(parserIndex + 1, id))
          {
            typeEnum = InputEvent::EventType::Button;
            val = 1;
            #ifdef SDL_JOYSTICK_IS_OVERRIDEN_BY_RECALBOX
            code = SDL_JoystickButtonEventCodeById(mDeviceIndex, id);
            #else
              #ifdef _RECALBOX_PRODUCTION_BUILD_
                #pragma GCC error "SDL_JOYSTICK_IS_OVERRIDEN_BY_RECALBOX undefined in production build!"
              #endif
            #endif
            parsed = true;
            { LOG(LogInfo) << "[InputDevice] Auto-Assign " << EntryToString(*entry) << " to Button " << id << " (Code: " << code << ')'; }
          }
          break;
        }
        case 'h':
        {
          if (value.TryAsInt(parserIndex + 1, '.', id))
            if (value.TryAsInt(parserIndex + 3, val))
            {
              typeEnum = InputEvent::EventType::Hat;
              #ifdef SDL_JOYSTICK_IS_OVERRIDEN_BY_RECALBOX
              code = SDL_JoystickHatEventCodeById(mDeviceIndex, id);
              #else
                #ifdef _RECALBOX_PRODUCTION_BUILD_
                  #pragma GCC error "SDL_JOYSTICK_IS_OVERRIDEN_BY_RECALBOX undefined in production build!"
                #endif
              #endif
              parsed = true;
              { LOG(LogInfo) << "[InputDevice] Auto-Assign " << EntryToString(*entry) << " to Hat " << id << '/' << val << " (Code: " << code << ')'; }
            }
          break;
        }
        default:
        {
        }
      }
      if (!parsed)
      {
        { LOG(LogError) << "[InputDevice] Error parsing mapping: " << mapping; }
        continue;
      }

      Set(*entry, InputEvent(mDeviceId, typeEnum, id, val, code));
      result = true; // At least one mapping has been done
    }

  // Manage pads with only one analog joystick and no dpad
  if (mInputEvents[(int)Entry::Up].Type() == InputEvent::EventType::Unknown)
    if (mInputEvents[(int)Entry::Right].Type() == InputEvent::EventType::Unknown)
      if (mInputEvents[(int)Entry::Down].Type() == InputEvent::EventType::Unknown)
        if (mInputEvents[(int)Entry::Left].Type() == InputEvent::EventType::Unknown)
          if (mInputEvents[(int)Entry::Joy1AxisH].Type() != InputEvent::EventType::Unknown)
            if (mInputEvents[(int)Entry::Joy1AxisV].Type() != InputEvent::EventType::Unknown)
            {
              const InputEvent& V = mInputEvents[(int)Entry::Joy1AxisV];
              const InputEvent& H = mInputEvents[(int)Entry::Joy1AxisH];

              // Add DPAD configuration
              mInputEvents[(int)Entry::Up] = V;
              mInputEvents[(int)Entry::Down] = InputEvent(mDeviceId, V.Type(), V.Id(), -V.Value(), V.Code());
              mInputEvents[(int)Entry::Left] = H;
              mInputEvents[(int)Entry::Right] = InputEvent(mDeviceId, H.Type(), H.Id(), -H.Value(), H.Code());
              mConfigurationBits |= (1 << (int)Entry::Up) | (1 << (int)Entry::Right) |
                                    (1 << (int)Entry::Down) | (1 << (int)Entry::Left);

              // Remove left analog configuration
              mInputEvents[(int)Entry::Joy1AxisV] = mInputEvents[(int)Entry::Joy1AxisH] = InputEvent();
              mConfigurationBits &= ~((1 << (int)Entry::Joy1AxisV) | (1 << (int)Entry::Joy1AxisH));

              { LOG(LogInfo) << "[InputDevice] No DPAD detected. First analog joystick moved to DPAD"; }
            }

  // Still no HK? Use select
  if (mInputEvents[(int)Entry::Hotkey].Type() == InputEvent::EventType::Unknown)
  {
    { LOG(LogInfo) << "[InputDevice] Not hotkey button guessed. Using SELECT as Hotkey"; }
    mInputEvents[(int) Entry::Hotkey] = mInputEvents[(int) Entry::Select];
  }

  return result;
}

int InputDevice::LoadFromXml(pugi::xml_node root)
{
  ClearAll();

  int loaded = 0;
  for (pugi::xml_node input = root.child("input"); input != nullptr; input = input.next_sibling("input"))
  {
    String name = input.attribute("name").as_string();
    String type = input.attribute("type").as_string();
    InputEvent::EventType typeEnum = InputEvent::StringToType(type);

    if(typeEnum == InputEvent::EventType::Unknown)
    {
      { LOG(LogError) << "[InputDevice] InputConfig load error - input of type \"" << type << "\" is invalid! Skipping input \"" << name << "\".\n"; }
      continue;
    }

    int id = input.attribute("id").as_int();
    int value = input.attribute("value").as_int();
    int code = input.attribute("code").as_int();

    if(value == 0)
    { LOG(LogWarning) << "[InputDevice] WARNING: InputConfig value is 0 for " << type << ' ' << id << "!\n"; }

    // Adapt seamlessly old l1/r1
    if (name == "pageup") name = "l1";
    if (name == "pagedown") name = "r1";
    Entry entry = StringToEntry(name);
    if (entry != Entry::None)
    {
      Set(entry, InputEvent(mDeviceId, typeEnum, id, value, code));
      loaded++;
    }
    else { LOG(LogError) << "[InputDevice] Unknown Joystick configuration entry: " << name << " of type " << type << "!\n"; }
  }
  return loaded;
}

void InputDevice::SaveToXml(pugi::xml_node parent) const
{
  pugi::xml_node cfg = parent.append_child("inputConfig");

  if(mDeviceId == InputEvent::sKeyboardDevice)
  {
    cfg.append_attribute("type") = "keyboard";
  }else{
    cfg.append_attribute("type") = "joystick";
  }

  cfg.append_attribute("deviceGUID") = GUID().c_str();
  cfg.append_attribute("deviceNbAxes") = mDeviceNbAxes;
  cfg.append_attribute("deviceNbHats") = mDeviceNbHats;
  cfg.append_attribute("deviceNbButtons") = mDeviceNbButtons;

  for (int i = (int)Entry::__Count; --i >= 0; )
  {
    if ((mConfigurationBits & (1 << (int)i)) == 0) continue;
    const InputEvent& entry = mInputEvents[i];

    pugi::xml_node input = cfg.append_child("input");
    input.append_attribute("name") = EntryToString((Entry)i).c_str();
    input.append_attribute("type") = InputEvent::TypeToString(entry.Type()).c_str();
    input.append_attribute("id").set_value(entry.Id());
    input.append_attribute("value").set_value(entry.Value());
    input.append_attribute("code").set_value(entry.Code());
  }
}

void InputDevice::StartEntry(InputCompactEvent::Entry entry, InputCompactEvent::Entry& bits)
{
  bits |= entry;
  int pos = __builtin_ffs((int)entry) - 1;
  if (mReferenceTimes[pos] == 0)
    mReferenceTimes[pos] = SDL_GetTicks();
}

int InputDevice::StopEntry(InputCompactEvent::Entry entry, InputCompactEvent::Entry& bits)
{
  bits |= entry;
  int pos = __builtin_ffs((int)entry) - 1;
  int result = mReferenceTimes[pos] != 0 ? (int)(SDL_GetTicks() - mReferenceTimes[pos]) : 0;
  mReferenceTimes[pos] = 0;
  return result;
}

int InputDevice::SetEntry(InputCompactEvent::Entry entry, bool active, InputCompactEvent::Entry& on,
                          InputCompactEvent::Entry& off)
{
  if (active) { StartEntry(entry, on); return 0; }
  return StopEntry(entry, off);
}

int InputDevice::ConvertButtonToOnOff(int button, bool pressed, InputCompactEvent::Entry& on, InputCompactEvent::Entry& off)
{
  int elapsed = 0;
  for (int i = (int)Entry::__Count; --i >= 0; )
    if ((mConfigurationBits & (1 << (int)i)) != 0)
      if (const InputEvent& config = mInputEvents[i]; config.Type() == InputEvent::EventType::Button)
        if (config.Id() == button)
          elapsed = SetEntry(ConvertEntry((Entry)i), pressed, on, off);
  return elapsed;
}

int InputDevice::ConvertKeyToOnOff(int key, bool pressed, InputCompactEvent::Entry& on, InputCompactEvent::Entry& off)
{
  int elapsed = 0;
  for (int i = (int)Entry::__Count; --i >= 0; )
    if ((mConfigurationBits & (1 << (int)i)) != 0)
      if (const InputEvent& config = mInputEvents[i]; config.Type() == InputEvent::EventType::Key)
        if (config.Id() == key)
          elapsed = SetEntry(ConvertEntry((Entry)i), pressed, on, off);
  return elapsed;
}

int InputDevice::ConvertHatToOnOff(int hat, int bits, InputCompactEvent::Entry& on, InputCompactEvent::Entry& off)
{
  int elapsed = 0;
  int previousBits = mPreviousHatsValues[hat];
  for (int i = (int)Entry::__Count; --i >= 0; )
    if ((mConfigurationBits & (1 << (int)i)) != 0)
      if (const InputEvent& config = mInputEvents[i]; config.Type() == InputEvent::EventType::Hat && config.Id() == hat)
      {
        // Hat bits are matching, set the target direction
        if ((bits & config.Value()) == config.Value())
          StartEntry(ConvertEntry((Entry)i), on);
        // Otherwise, if previous value matched,
        else if ((previousBits & config.Value()) == config.Value()) // Previous value match bits?
          elapsed = StopEntry(ConvertEntry((Entry)i), off);
      }

  // Record previous value
  mPreviousHatsValues[hat] = bits;
  return elapsed;
}

int InputDevice::ConvertAxisToOnOff(int axis, int value, InputCompactEvent::Entry& on, InputCompactEvent::Entry& off)
{
  int elapsed = 0;
  for (int i = (int)Entry::__Count; --i >= 0; )
    if ((mConfigurationBits & (1 << (int)i)) != 0)
      if (const InputEvent& config = mInputEvents[i]; config.Type() == InputEvent::EventType::Axis && config.Id() == axis)
      {
        // For axis, we must distinguish target axis from other binary targets.
        // Axis to axis are converted to negative/center/positive.
        // Axis to binaries are converted to on only for the right sign. off otherwise
        // That is, axis on triggers will be properly converted to on/off buttons.
        if (InputCompactEvent::Entry targetEntry = ConvertEntry((Entry)i); targetEntry >= InputCompactEvent::Entry::J1Left)
        {
          InputCompactEvent::Entry targetOpposite = (InputCompactEvent::Entry)((int)targetEntry << 1);
          // Since configured event are negatives, if we got a positive value in current config, that means
          // the joystick is inverted.
          // Note: values are already normalized to -1/0/+1
          value = (config.Value() > 0) ? -value : value;
          int previousValue = mPreviousAxisValues[axis];
          if (previousValue != value)
          switch(value)
          {
            case -1: // configured axis direction?
            {
              StartEntry(targetEntry, on);
              if (previousValue > 0) elapsed = StopEntry(targetOpposite, off); // In case joystick has been quickly moved from one direction to another
              break;
            }
            case 1: // Opposite of configured axis direction?
            {
              StartEntry(targetOpposite, on);
              if (previousValue < 0) elapsed = StopEntry(targetEntry, off); // In case joystick has been quickly moved from one direction to another
              break;
            }
            default: // Axis centered again
            {
              elapsed = StopEntry(previousValue < 0 ? targetEntry : targetOpposite, off);
              break;
            }
          }
        }
        // Axis has a binary on/off
        else if (value == config.Value() || mPreviousAxisValues[axis] == config.Value())
          elapsed = SetEntry(targetEntry, value == config.Value(), on, off);
      }

  // Record previous value
  mPreviousAxisValues[axis] = value;
  return elapsed;
}

int InputDevice::ConvertMouseButtonToOnOff(int button, bool pressed, InputCompactEvent::Entry& on,
                                           InputCompactEvent::Entry& off)
{
  int elapsed = 0;
  for (int i = (int)Entry::__Count; --i >= 0; )
    if ((mConfigurationBits & (1 << (int)i)) != 0)
      if (const InputEvent& config = mInputEvents[i]; config.Type() == InputEvent::EventType::MouseButton)
        if (config.Id() == button)
          elapsed = SetEntry(ConvertEntry((Entry)i), pressed, on, off);
  return elapsed;
}

int InputDevice::ConvertMouseWheelToOnOff(int wheel, int direction, InputCompactEvent::Entry& on,
                                          InputCompactEvent::Entry& off)
{
  bool stop = (direction & 0x1) == 0;
  if (stop) direction >>= 1;
  int elapsed = 0;
  for (int i = (int)Entry::__Count; --i >= 0; )
    if ((mConfigurationBits & (1 << (int)i)) != 0)
      if (const InputEvent& config = mInputEvents[i]; config.Type() == InputEvent::EventType::MouseWheel)
        if (config.Id() == wheel)
          if (config.Value() == direction)
            elapsed = SetEntry(ConvertEntry((Entry)i), !stop, on, off);
  return elapsed;
}

InputCompactEvent InputDevice::ConvertToCompact(const InputEvent& event)
{
  //{ LOG(LogError) << "[InputEvent] " << event.ToString(); }
  InputCompactEvent::Entry on  = InputCompactEvent::Entry::Nothing;
  InputCompactEvent::Entry off = InputCompactEvent::Entry::Nothing;
  int elapsed = 0;

  // Need configuration?
  if (!IsConfigured())
    off = (on |= InputCompactEvent::Entry::NeedConfiguration);

  switch(event.Type())
  {
    case InputEvent::EventType::Axis: elapsed = ConvertAxisToOnOff(event.Id(), event.Value(), on, off); break;
    case InputEvent::EventType::Hat: elapsed = ConvertHatToOnOff(event.Id(), event.Value(), on, off); break;
    case InputEvent::EventType::Button: elapsed = ConvertButtonToOnOff(event.Id(), event.Value() != 0, on, off); break;
    case InputEvent::EventType::Key: elapsed = ConvertKeyToOnOff(event.Id(), event.Value() != 0, on, off); break;
    case InputEvent::EventType::MouseButton: elapsed = ConvertMouseButtonToOnOff(event.Id(), event.Value() != 0, on, off); break;
    case InputEvent::EventType::MouseWheel: elapsed = ConvertMouseWheelToOnOff(event.Id(), event.Value(), on, off); break;
    case InputEvent::EventType::Unknown:
    default: { LOG(LogError) << "[InputDevice] Abnormal InputEvent::EventType: " << (int)event.Type(); break; }
  }

  // Process extended
  if ((on & InputCompactEvent::Entry::Hotkey) != 0)
  {
    mHotkeyState = true;
    if (IsHotKeyAndSelectKeysTheSame() && (on & InputCompactEvent::Entry::Select) != 0)
      on &= ~InputCompactEvent::Entry::Select;
  }
  if (mHotkeyState)
  {
    // Keep special keys in the low dword
    on  = ( on & InputCompactEvent::Entry::AllSpecials) | (( on << 32) & ~InputCompactEvent::Entry::HotkeyAllSpecials);
    off = (off & InputCompactEvent::Entry::AllSpecials) | ((off << 32) & ~InputCompactEvent::Entry::HotkeyAllSpecials);
  }
  if ((off & InputCompactEvent::Entry::Hotkey) != 0)
  {
    mHotkeyState = false;
    if (IsHotKeyAndSelectKeysTheSame() && (off & InputCompactEvent::Entry::Select) != 0)
    {
      if (mKillSelect) off &= ~InputCompactEvent::Entry::Select; // Kill select from the off events
      else on |= InputCompactEvent::Entry::Select;  // Add select to the on event to make on/off in the same event
    }
  }

  // Build final object
  InputCompactEvent result = { on, off, elapsed, *this, event };
  //{ LOG(LogError) << "[CompactInputEvent] " << result.ToString(); }
  return result;
}

InputCompactEvent::Entry InputDevice::ConvertEntry(InputDevice::Entry entry)
{
  switch(entry)
  {
    case Entry::Start: return InputCompactEvent::Entry::Start;
    case Entry::Select: return InputCompactEvent::Entry::Select;
    case Entry::Hotkey: return InputCompactEvent::Entry::Hotkey;
    case Entry::A: return InputCompactEvent::Entry::A;
    case Entry::B: return InputCompactEvent::Entry::B;
    case Entry::X: return InputCompactEvent::Entry::X;
    case Entry::Y: return InputCompactEvent::Entry::Y;
    case Entry::L1: return InputCompactEvent::Entry::L1;
    case Entry::R1: return InputCompactEvent::Entry::R1;
    case Entry::L2: return InputCompactEvent::Entry::L2;
    case Entry::R2: return InputCompactEvent::Entry::R2;
    case Entry::L3: return InputCompactEvent::Entry::L3;
    case Entry::R3: return InputCompactEvent::Entry::R3;
    case Entry::Up: return InputCompactEvent::Entry::Up;
    case Entry::Right: return InputCompactEvent::Entry::Right;
    case Entry::Down: return InputCompactEvent::Entry::Down;
    case Entry::Left: return InputCompactEvent::Entry::Left;
    case Entry::Joy1AxisH: return InputCompactEvent::Entry::J1Left;
    case Entry::Joy1AxisV: return InputCompactEvent::Entry::J1Up;
    case Entry::Joy2AxisH: return InputCompactEvent::Entry::J2Left;
    case Entry::Joy2AxisV: return InputCompactEvent::Entry::J2Up;
    case Entry::VolumeUp: return InputCompactEvent::Entry::VolUp;
    case Entry::VolumeDown: return InputCompactEvent::Entry::VolDown;
    case Entry::BrightnessUp: return InputCompactEvent::Entry::LumUp;
    case Entry::BrightnessDown: return InputCompactEvent::Entry::LumDown;
    case Entry::None:
    case Entry::__Count:
    default: break;
  }
  { LOG(LogError) << "[InputDevice] Unknown entry! " << (int)entry; }
  return InputCompactEvent::Entry::Nothing;
}

bool InputDevice::EqualsTo(const InputDevice& to) const
{
  return (mDeviceName == to.mDeviceName) &&
         (memcmp(&mDeviceGUID, &to.mDeviceGUID, sizeof(SDL_JoystickGUID)) == 0) &&
         (mDeviceNbAxes == to.mDeviceNbAxes) &&
         (mDeviceNbHats == to.mDeviceNbHats) &&
         (mDeviceNbButtons == to.mDeviceNbButtons)
         ;
}

bool InputDevice::CheckNeutralPosition() const
{
  // Check buttons
  for(int i = mDeviceNbButtons; --i >= 0; )
    if (SDL_JoystickGetButton(mDeviceSDL, i) != 0)
      return false;
  // Check hats
  for(int i = mDeviceNbHats; --i >= 0; )
    if (SDL_JoystickGetHat(mDeviceSDL, i) != 0)
      return false;
  // Check axis
  for(int i = mDeviceNbAxes; --i >= 0; )
  {
    int axis = SDL_JoystickGetAxis(mDeviceSDL, i);
    axis = axis < -sJoystickDeadZone ? -1 : (axis > sJoystickDeadZone ? 1 : 0);
    if (axis != mNeutralAxisValues[i])
      return false;
  }
  return true;
}

String InputDevice::UDevString(int index, UDevInfo info)
{
  if (index >= 0)
  {
    UDev udev;
    UDevEnumerate::DeviceList list = UDevEnumerate(udev).MatchSysname(mPath.Filename()).List();
    if (list.size() == 1)
    {
      const UDevDevice& device = list[0];
      mIsATruePad = device.Property("ID_INPUT_JOYSTICK") == "1";
      switch(info)
      {
        case UDevInfo::LowlevelName:
        {
          //! ID_VENDOR_ENC + ID_MODEL_ENC for bluetooth pads
          if (device.Property("ID_BUS") == "bluetooth")
          {
            String vendor(device.PropertyDecode("ID_VENDOR_ENC"));
            String model(device.PropertyDecode("ID_MODEL_ENC"));
            if (!model.Trim().empty())
            {
              if (!vendor.Trim().empty()) model.Insert(0, ' ').Insert(0, vendor);
              if (!model.Trim().empty()) return model;
            }
          }
          //! ATTRS{name} for all others
          String name(device.Parent().Sysattr("name"));
          if (!name.empty()) return name;

          // Default: SDL2 name
          return mDeviceName;
        }
        case UDevInfo::UniqueID:
        {
          String uniq(device.Parent().Sysattr("uniq"));
          return uniq;
        }
        case UDevInfo::PowerPath:
        {
          // mUDevUniqueID must have been filled before - don't reorder the inits!
          for(const Path& path : Path("/sys/class/power_supply").GetDirectoryContent())
            if (path.IsDirectory())
              if (path.ToString().EndsWith(mUDevUniqID))
                return path.ToString();
          break;
        }
        default: return String::Empty;
      }
    }
    else { LOG(LogError) << "[InputDevice] Cannot get udev info " << (int)info <<" for pad index " << index; }
  }
  return "";
}

void InputDevice::RecordAxisNeutralPosition()
{
  // Fill neutral values
  for (int i = mDeviceNbAxes; --i >= 0;)
  {
    Sint16 state = 0;
    if (SDL_JoystickGetAxisInitialState(mDeviceSDL, i, &state) == SDL_TRUE)
      mNeutralAxisValues[i] = state < -sJoystickDeadZone ? -1 : (state > sJoystickDeadZone ? 1 : 0);
  }
}

#ifndef SDL_JOYSTICK_IS_OVERRIDEN_BY_RECALBOX
Path InputDevice::LookupPath()
{
  UDev udev;
  for(int i = 0; i < 64; ++i)
  {
    // Build path
    Path devicePath(String("/dev/input/event").Append(i));
    if (!devicePath.Exists()) continue;
    // Lookup udev device
    UDevEnumerate::DeviceList list = UDevEnumerate(udev).MatchSysname(Path(devicePath).Filename()).List();
    if (list.empty()) continue;
    // Get device
    const UDevDevice& device = list[0];
    // Is it a true pad?
    if (device.Property("ID_INPUT_JOYSTICK") != "1") continue;
    // Get first parent
    const UDevDevice& parent = device.Parent();
    // Extract IDs used in SDL2 GUID
    int busType = String('$').Append(parent.Sysattr("id/bustype")).AsInt();
    int product = String('$').Append(parent.Sysattr("id/product")).AsInt();
    int vendor  = String('$').Append(parent.Sysattr("id/vendor")).AsInt();
    int version = String('$').Append(parent.Sysattr("id/version")).AsInt();
    // Build GUIDs - use a 4 little indian 32bits int for comparison, masked to use only LSW
    union InternalGUID
    {
      SDL_JoystickGUID Sdl;
      int Native[4];
    };
    InternalGUID Sdl { .Sdl = mDeviceGUID };
    InternalGUID Dev { .Native { busType, vendor, product, version } };
    // Compare
    bool match = true;
    for(int n = (int)(sizeof(InternalGUID::Native) / sizeof(InternalGUID::Native[0])); --n >= 0;)
      if ((Sdl.Native[n] & 0xFFFF) != (Dev.Native[n] & 0xFFFF))
      {
        match = false;
        break;
      }
    if (match) return devicePath;
  }
  { LOG(LogError) << "[InputDevice] Cannot lookup device path for pad " << Name(); }
  return Path::Empty;
}
#endif
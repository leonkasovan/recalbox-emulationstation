//
// Created by Bkg2k on 17/09/2023.
//

#include "UDevDevice.h"
#include "UDev.h"
#include <libudev.h>

UDevDevice::UDevDevice(UDev& context, const String& path)
  : mUDev(context)
  , mDevice(udev_device_new_from_syspath(mUDev.Context(), path.data()))
{
}

UDevDevice::UDevDevice(const UDevDevice& source)
  : mUDev(source.mUDev)
  , mDevice(nullptr)
{
  if (source.mDevice != nullptr)
    mDevice = udev_device_new_from_syspath(mUDev.Context(), udev_device_get_syspath(source.mDevice));
}

UDevDevice::UDevDevice(UDevDevice&& source) noexcept
  : mUDev(source.mUDev)
  , mDevice(source.mDevice)
{
  source.mDevice = nullptr;
}

UDevDevice::~UDevDevice()
{
  if (mDevice != nullptr)
    udev_device_unref(mDevice);
}

UDevDevice UDevDevice::Parent() const
{
  udev_device* device = udev_device_get_parent(mDevice);

  // Make a "deep copy" so that parent will have its own lifetime
  if (device != nullptr)
    device = udev_device_new_from_syspath(mUDev.Context(), udev_device_get_syspath(device));

  return UDevDevice(mUDev, device);
}

UDevDevice UDevDevice::Parent(const String& subsystem, const String& type) const
{
  udev_device* device = udev_device_get_parent_with_subsystem_devtype(mDevice, subsystem.data(), type.empty() ? nullptr : type.data());

  // Make a "deep copy" so that parent will have its own lifetime
  if (device != nullptr)
    device = udev_device_new_from_syspath(mUDev.Context(), udev_device_get_syspath(device));

  return UDevDevice(mUDev, device);
}

UDevDevice::ActionType UDevDevice::Action() const
{
  String action = ToString(udev_device_get_action(mDevice));
  return action == "add" ? ActionType::added : action == "remove" ? ActionType::removed : ActionType::other;
}

void UDevDevice::Decode(String& source)
{
  for(;;)
    if (int pos = source.Find("\\x"); pos >= 0 && pos <= source.Count() - 4)
    {
      char h = (char)(source[pos + 2] | 0x20); // high quartet lowercase
      char l = (char)(source[pos + 3] | 0x20); // low quartet lowercase
      source.replace(pos, 4, 1, (char)(((h <= 0x39 ? h - 0x30 : (h - 'a') + 10) << 4) | (l <= 0x39 ? l - 0x30 : (l - 'a') + 10)));
    }
    else break;
}

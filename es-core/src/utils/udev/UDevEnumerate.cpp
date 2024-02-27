//
// Created by Bkg2k on 17/09/2023.
//

#include "UDevEnumerate.h"
#include "UDev.h"
#include "UDevDevice.h"

UDevEnumerate::UDevEnumerate(UDev& context)
  : mUDev(context)
  , mEnumeration(udev_enumerate_new(mUDev.Context()))
{
}

UDevEnumerate::~UDevEnumerate()
{
  if (mEnumeration != nullptr)
    udev_enumerate_unref(mEnumeration);
}

UDevEnumerate& UDevEnumerate::ResetFilters()
{
  if (mEnumeration != nullptr)
    udev_enumerate_unref(mEnumeration);
  mEnumeration = udev_enumerate_new(mUDev.Context());

  return *this;
}

UDevEnumerate& UDevEnumerate::MatchParent(const UDevDevice& device)
{
  udev_enumerate_add_match_parent(mEnumeration, device.mDevice);

  return *this;
}

UDevEnumerate::DeviceList UDevEnumerate::List()
{
  DeviceList devices;
  udev_enumerate_scan_devices(mEnumeration);

  udev_list_entry* e;
  udev_list_entry_foreach(e, udev_enumerate_get_list_entry(mEnumeration))
    if(const char* path = udev_list_entry_get_name(e); path != nullptr)
      devices.push_back(UDevDevice(mUDev, path));

  return devices;
}
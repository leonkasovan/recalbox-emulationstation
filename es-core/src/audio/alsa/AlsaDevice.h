//
// Created by bkg2k on 13/08/2020.
//
#pragma once

#include <utils/String.h>
#include "audio/NameFiltering.h"

class AlsaDevice
{
  private:
    // Internal references
    String mDeviceName; //!< Device name
    int mDeviceId;           //!< Device ID
    int mSubDeviceCount;     //!< Sub device copunt

  public:
    //! Default constructor
    AlsaDevice(int id, const String& name, int subdevices)
    {
      mDeviceId = id;
      mDeviceName = NameFiltering::Filter(name, NameFiltering::Source::Device);
      mSubDeviceCount = subdevices;
    }

    //! Get mixer identifier
    int Identifier() const { return mDeviceId; }
    //! Get mixer name
    const String& Name() const { return mDeviceName; }
};

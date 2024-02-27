//
// Created by Bkg2k on 17/09/2023.
//
#pragma once

#include <libudev.h>

class UDev
{
  public:
    //! Constructor
    UDev();

    //! Destructor
    ~UDev();

    //! Get udev context
    [[nodiscard]] udev* Context() const { return mContext; }

  private:
    //! udev Context
    udev* mContext;

};

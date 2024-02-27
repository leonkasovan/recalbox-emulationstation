//
// Created by bkg2k on 20/12/2021.
//
// As part of the RECALBOX Project
// http://www.recalbox.com
//
#pragma once

#include <hardware/crt/ICrtInterface.h>

class CrtPi2Scart : public ICrtInterface
{
  public:
    //! Constructor
    explicit CrtPi2Scart(bool automaticallyDetected, BoardType boardType) : ICrtInterface(automaticallyDetected, boardType) {}

    //! Yes a CRT adapter is attached!
    bool IsCrtAdapterAttached() const override { return true; }

    //! This adapter is a Pi2Scart
    CrtAdapterType GetCrtAdapter() const override { return CrtAdapterType::Pi2Scart; }

    //! No 31khz support
    bool Has31KhzSupport() const override { return false; }

    //! This adapter is stuck on 15khz
    HorizontalFrequency GetHorizontalFrequency() const override { return ICrtInterface::HorizontalFrequency::KHz15; }

    //! This adapter has no support of forced 50hz
    bool HasForced50hzSupport() const override { return false; }

    //! This adapter cannot force 50hz
    bool MustForce50Hz() const override { return false; }

    //! The comment is here to tell you that the name will be returned bby this methode named Name()
    std::string& Name() const override { static std::string adapterString("Pi2Scart"); return adapterString; }

    std::string& ShortName() const override { static std::string adapterShortString("pi2scart"); return adapterShortString; }

};

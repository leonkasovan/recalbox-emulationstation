//
// Created by bkg2k on 19/12/2021.
//
// As part of the RECALBOX Project
// http://www.recalbox.com
//
#pragma once

#include <hardware/crt/ICrtInterface.h>
#include <utils/Files.h>
#include "CrtConf.h"

class CrtRGBDual : public ICrtInterface
{
  public:
    //! Constructor
    explicit CrtRGBDual(bool automaticallyDetected, BoardType boardType) : ICrtInterface(automaticallyDetected, boardType) {}

    //! An RGB Dual is attached
    bool IsCrtAdapterAttached() const override
    {
      #ifdef OPTION_RECALBOX_SIMULATE_RRGBD
      return true;
      #else
      return Files::LoadFile(Path(vgaCardConnectedPi4)) == "connected\n"
             || Files::LoadFile(Path(vgaCardConnectedPi3)) == "connected\n";
      #endif
    }

    //! This adapter is an RGB Dual
    CrtAdapterType GetCrtAdapter() const override { return CrtAdapterType::RGBDual; }

    //! RGB Dual has support for 31khz
    bool Has31KhzSupport() const override { return true; }

    //! RGB Dual has support for 120hz modes
    bool Has120HzSupport() const override { return true; }

    //! Return select output frequency
    HorizontalFrequency GetHorizontalFrequency() const override {
      return MultiSyncEnabled()? ICrtInterface::HorizontalFrequency::KHzMulti :
             (GetRGBDual31khzSwitchState() ? HorizontalFrequency::KHz31 :
             (CrtConf::Instance().GetSystemCRTScreen31kHz() ? HorizontalFrequency::KHz31 : HorizontalFrequency::KHz15));
    }

    //! Return multisync enabled
    bool MultiSyncEnabled() const override { return CrtConf::Instance().GetSystemCRTScreenMultiSync(); }

    //! This adapter has support of forced 50hz
    bool HasForced50hzSupport() const override { return true; }

    //! Get 50hz switch state
    bool MustForce50Hz() const override { return GetRGBDual50hzSwitchState(); }

    //! The comment is here to tell you that the name will be returned bby this methode named Name()
    std::string& Name() const override { static std::string adapterString("Recalbox RGB Dual"); return adapterString; }

    std::string& ShortName() const override { static std::string adapterShortString("recalboxrgbdual"); return adapterShortString; }
  private:
    static constexpr const char* sRGBDual31khzSwitch = "/sys/devices/platform/recalboxrgbdual/dipswitch-31khz/value";
    static constexpr const char* sRGBDual50hzSwitch = "/sys/devices/platform/recalboxrgbdual/dipswitch-50hz/value";
    static constexpr const char* vgaCardConnectedPi4 = "/sys/class/drm/card1-VGA-1/status";
    static constexpr const char* vgaCardConnectedPi3 = "/sys/class/drm/card0-VGA-1/status";

    //! Get 31khz switch state on RGB dual board
    static bool GetRGBDual31khzSwitchState() { return Files::LoadFile(Path(sRGBDual31khzSwitch)) == "0\n"; }
    //! Get 50hz switch state on RGB dual board
    static bool GetRGBDual50hzSwitchState() { return Files::LoadFile(Path(sRGBDual50hzSwitch)) == "0\n"; }
};




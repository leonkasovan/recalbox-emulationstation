//
// Created by bkg2k on 26/09/23.
//
#pragma once

#include <resources/Font.h>
#include <guis/Gui.h>
#include "components/ImageComponent.h"
#include "BaseOSD.h"
#include "PadOSD.h"
#include "FpsOSD.h"
#include "BatteryOSD.h"
#include "BluetoothOSD.h"
#include <bluetooth/DeviceStatus.h>
#include <bluetooth/IBluetoothDeviceStatusListener.h>

class OSDManager : public Gui
{
  public:
    //! Constructor
    explicit OSDManager(WindowManager& window);

    /*
     * Accesors
     */

    //! Get Pad OSD
    PadOSD& GetPadOSD() { return mPadOSD; }

    //! Get Pad OSD
    FpsOSD& GetFpsOSD() { return mFpsOSD; }

    //! Get Pad OSD
    BluetoothOSD& GetBluetoothOSD() { return mBluetoothOSD; }

    /*
     * Component override
     */

    //! Process input
    bool ProcessInput(const InputCompactEvent& event) override;

    //! Update fadings
    void Update(int deltaTime) override;

    //! Draw
    void Render(const Transform4x4f& parentTrans) override;

  private:
    //! Bluetooth OSD
    BluetoothOSD mBluetoothOSD;
    //! Pad OSD
    PadOSD mPadOSD;
    //! Fps OSD
    FpsOSD mFpsOSD;
    //! Battery OSD
    BatteryOSD mBatteryOSD;

    //! OSD List
    Array<BaseOSD*> mOSDList;
};

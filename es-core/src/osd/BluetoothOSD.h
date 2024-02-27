//
// Created by bkg2k on 03/10/23.
//
#pragma once

#include <memory>
#include <osd/BaseOSD.h>
#include <utils/gl/Rectangle.h>
#include <components/ImageComponent.h>
#include <utils/gl/Colors.h>
#include <bluetooth/IBluetoothDeviceStatusListener.h>

class BluetoothOSD : public BaseOSD
                   , public IBluetoothDeviceStatusListener
{
  public:
    /*!
     * @brief Constructor
     * @param window Window manager
     */
    explicit BluetoothOSD(WindowManager& window, Side side);

    /*
     * Component override
     */

    //! Update fadings
    void Update(int deltaTime) override;

    //! Draw
    void Render(const Transform4x4f& parentTrans) override;

    /*
     * BaseOSD implementation
     */

    //! Get max width
    [[nodiscard]] int OSDAreaWidth() const override { return (int)mBTArea.Width(); }

    //! Get height
    [[nodiscard]] int OSDAreaHeight() const override { return (int)mBTArea.Height(); }

    //! Visible?
    [[nodiscard]] bool IsActive() const override { return mRemaining > 0; }

  private:
    //! FPS rectangle
    Rectangle mBTArea;
    //! BT Controller icon
    ImageComponent mControllerIcon;
    //! BT network icon top
    ImageComponent mBtTop;
    //! BT network icon middle
    ImageComponent mBtMiddle;
    //! BT network icon bottom
    ImageComponent mBtBottom;
    //! BT Progress bar position
    Rectangle mProgressBar;
    //! BT Remaining time in ms
    int mRemaining;
    //! BT Total time in ms
    int mTotal;
    //! Background color
    Colors::ColorRGBA mBarBackColor;
    //! Fore color
    Colors::ColorRGBA mBarForeColor;

    /*
     * IBluetoothDeviceStatusListener implementation
     */

    /*!
     * @brief Receive latest device status
     * @param status DeviceStatus instance
     */
    void ReceiveBluetoothDeviceStatus(DeviceStatus& status) final
    {
      mRemaining = status.RemainingTime() * 1000;
      mTotal = status.TotalTime() * 1000;
    }
};

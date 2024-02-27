//
// Created by bkg2k on 03/10/23.
//
#pragma once

#include "BaseOSD.h"
#include <utils/gl/Colors.h>
#include <utils/gl/Rectangle.h>
#include <RecalboxConf.h>
#include <resources/Font.h>
#include <memory>

class BatteryOSD : public BaseOSD
                 , public IRecalboxConfChanged
{
  public:
    /*!
     * @brief Constructor
     * @param window Window manager
     */
    explicit BatteryOSD(WindowManager& window, Side side);

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
    [[nodiscard]] int OSDAreaWidth() const override { return (int)mBatteryArea.Width(); }

    //! Get height
    [[nodiscard]] int OSDAreaHeight() const override { return (int)mBatteryArea.Height(); }

    //! Visible?
    [[nodiscard]] bool IsActive() const override { return mHasBattery && mVisible; }

  private:
    //! FPS Font
    std::shared_ptr<Font> mFont;
    //! FPS rectangle
    Rectangle mBatteryArea;
    //! Icon
    String::Unicode mIcon;
    //! C olor
    Colors::ColorRGBA mColor;
    //! Has battery?
    bool mHasBattery;
    //! Visible?
    bool mVisible;

    //! Callback when pad OSD option changes
    void ConfigurationChanged(const String& key) override { (void)key; mVisible = !RecalboxConf::Instance().GetBatteryHidden(); }
};
//
// Created by bkg2k on 13/02/2021.
//
// As part of the RECALBOX Project
// http://www.recalbox.com
//

#pragma once

#include <utils/locale/LocaleHelper.h>
#include "WizardLanguage.h"

class WizardRG353X : public WizardLanguage
{
  public:
    enum class Pages
    {
        Intro,
        VolumeButtons,
        FunctionKey,
        BritghtnessButtons,
        PowerSuspend,
        PowerOff,
        Final,
        Count,
    };

    explicit WizardRG353X(WindowManager& window)
    : WizardLanguage(window, _("WELCOME TO RECALBOX!"), int(Pages::Count))
    {
    }

  private:
    static constexpr const char* sImagePath           = "/recalbox/system/resources/wizard/rg353x";
    static constexpr const char* sIntroImageFile      = "intro.jpg";
    static constexpr const char* sFunctionKey         = "function.jpg";
    static constexpr const char* sVolumeImageFile     = "volume.jpg";

    //! Called when a page image is required
    Path OnImageRequired(int page) override;

    //! Called when a page text is required
    String OnTextRequired(int page) override;

    //! Called when z key event is received
    Move OnKeyReceived(int page, const InputCompactEvent& event) override;
};

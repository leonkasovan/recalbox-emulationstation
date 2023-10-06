//
// Created by bkg2k on 02/06/23.
//
#pragma once

#include <utils/locale/LocaleHelper.h>
#include "WizardLanguage.h"

class WizardDidYouKnow : public WizardLanguage
{
  public:
    /*!
     * @brief Build a wizard window
     * @param window Window manager
     * @param pageTexts Page texts
     */
    WizardDidYouKnow(WindowManager& window, const String::List& pageTexts)
      : WizardLanguage(window, _("DID YOU KNOW?"), (int)pageTexts.size())
      , mPagesTexts(pageTexts)
    {
    }

  private:
    static constexpr const char* sImagePath           = ":/wizard/didyouknow.png";

    String::List mPagesTexts;

    //! Called when a page image is required
    Path OnImageRequired(int page) override { (void)page; return Path(sImagePath); }

    //! Called when a page text is required
    String OnTextRequired(int page) override { return mPagesTexts[page]; }

    //! Called when z key event is received
    Move OnKeyReceived(int page, const InputCompactEvent& event) override;
};

//
// Created by bkg2k on 13/02/2021.
//
// As part of the RECALBOX Project
// http://www.recalbox.com
//

#pragma once

#include "SimpleWizardBase.h"

class WizardLanguage : public SimpleWizardBase
{
  public:
    /*!
     * @brief Constructor
     * @param window
     * @param title
     * @param pagecount
     */
    explicit WizardLanguage(WindowManager& window, const String& title, int pagecount)
    : SimpleWizardBase(window, title, pagecount)
    {
    }

    //! Called when rebuilding the help bar
    void OnHelpRequired(int page, Help& help) override;

    /*!
     * @brief Change current language
     * @param increment True to go forward in the language list, false to go backward
     */
    void ChangeLanguage(bool increment);
};




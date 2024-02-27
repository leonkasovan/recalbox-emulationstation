//
// Created by bkg2k on 11/10/23.
//
#pragma once

#include "WizardBase.h"
#include "utils/locale/LocaleHelper.h"

class WizardLite : public WizardBase
{
  public:
    enum class Pages
    {
      Intro,
      HowTo,
      AddGames,
      Update,
      Count,
    };

    explicit WizardLite(WindowManager& window)
    : WizardBase(window, _("WELCOME TO RECALBOX!"), int(Pages::Count), true)
    {
    }

  private:

    //! Called when a key event is received
    Move OnKeyReceived(int page, const InputCompactEvent& event) override;

    /*!
     * @brief Called when a new page is displayed
     * @param page Page to display
     * @param componentIndex Request component at index
     * @param x Grid x coordinate
     * @param y Grid y coordinate
     * @param w width in cell
     * @param h height in cell
     * @param component Component instance
     * @return True if a component is available at the given index, false otherwise
     */
    bool OnComponentRequired(int page, int componentIndex, [[out]] Rectangle& where, std::shared_ptr<Component>& component) override;

    /*!
     * @brief Called when a new page is displayed to get additionnal buttons (next is always displayed) in the grid
     * This method is called with an index starting from 0 and incrementing on each call.
     * Returning false ends the loop
     * @param page Page to display
     * @param buttonIndex Request button at index
     * @param button Text in the button
     * @return True if a component is available at the given index, false otherwise
     */
    bool OnButtonRequired(int page, int buttonIndex, String& buttonText) override;

    /*!
     * @brief Called when a button other than 'next' is clicked allowing to execute custom operations
     * @param page Current page
     * @param buttonIndex
     * @return
     */
    Move OnButtonClick(int page, int buttonIndex) override;

    /*!
     * @brief Called when the help bar is being to be refreshed
     * @param page Page to display
     * @param help Help to fill-in
     */
    void OnHelpRequired(int page, Help& help) override { (void)page; (void)help; }
};

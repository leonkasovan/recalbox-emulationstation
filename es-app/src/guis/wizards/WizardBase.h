//
// Created by bkg2k on 11/10/23.
//
#pragma once

#include <memory>
#include "guis/Gui.h"
#include "components/TextComponent.h"
#include "components/ButtonComponent.h"
#include "components/ComponentGrid.h"
#include "components/NinePatchComponent.h"

class WizardBase: public Gui
{
  public:
    enum class Move
    {
      None,
      Backward,
      Forward,
      Close,
    };

    /*!
     * @brief Constructor
     * @param window Main window
     * @param title Wizard title
     * @param pageCount Total page count
     * @param autoNext True to let wizard managing next/close buttons automatically on each page
     * false to manage all button manually
     */
    WizardBase(WindowManager& window, const String& title, int pageCount, bool autoNext);

    /*!
     * @brief Called when a new page is displayed to get components and their coordinates in the grid
     * This method is called with an index starting from 0 and incrementing on each call.
     * Returning false ends the loop
     * @param page Page to display
     * @param componentIndex Request component at index
     * @param x Grid x coordinate
     * @param y Grid y coordinate
     * @param w width in cell
     * @param h height in cell
     * @param component Component instance
     * @return True if a component is available at the given index, false otherwise
     */
    virtual bool OnComponentRequired(int page, int componentIndex, [[out]] Rectangle& where, std::shared_ptr<Component>& component) = 0;

    /*!
     * @brief Called when a new page is displayed to get additionnal buttons (next is always displayed) in the grid
     * This method is called with an index starting from 0 and incrementing on each call.
     * Returning false ends the loop
     * @param page Page to display
     * @param buttonIndex Request button at index
     * @param button Text in the button
     * @return True if a component is available at the given index, false otherwise
     */
    virtual bool OnButtonRequired(int page, int buttonIndex, String& buttonText) = 0;

    /*!
     * @brief Called when a button other than 'next' is clicked allowing to execute custom operations
     * @param page Current page
     * @param buttonIndex
     * @return
     */
    virtual Move OnButtonClick(int page, int buttonIndex) = 0;

    /*!
     * @brief Called when the help bar is being to be refreshed
     * @param page Page to display
     * @param help Help to fill-in
     */
    virtual void OnHelpRequired(int page, Help& help) = 0;

    /*!
     * @brief Ask the wizard to react to the given input event
     * @param page Current page
     * @param event Input event
     * @return None if the event is of no interest, Forward or Backward if the wizard must move, Close to close the wizard
     */
    virtual Move OnKeyReceived(int page,const InputCompactEvent& event) = 0;

    /*!
     * @brief Force a refresh of the current page
     */
    void UpdatePage();

  private:
    //! "Window" background
    NinePatchComponent mBackground;
    //! Global component grid for easy positioning
    ComponentGrid mGrid;

    //! Title
    std::shared_ptr<TextComponent> mTitle;
    //! Components
    std::vector<std::shared_ptr<Component>> mComponents;

    //! Page count
    int mPageCount;
    //! Current page index
    int mCurrentPage;

    //! NEXT/CLOSE butotn managed automatically?
    bool mAutoNext;

    /*!
     * @brief Process input events
     * @param event Input event
     * @return True if it has been processed
     */
    bool ProcessInput(const InputCompactEvent& event) final;

    /*!
     * @brief Update frame, used to start the first page
     * @param deltaTime Elapsed time since last page
     */
    void Update(int deltaTime) final;

    /*!
     * @brief Fill the help bar
     * @param help Help bar
     * @return True
     */
    bool getHelpPrompts(Help& help) final;

    /*!
     * @brief Set the active page of the wizard
     * @param page new active page index
     */
    void SetPage(int page);

    /*!
     * @brief Callback from button bar
     * @param index ButtonIndex
     */
    void DoButtonClick(int index);
};

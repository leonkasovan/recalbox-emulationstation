//
// Created by bkg2k on 31/10/2020.
//
#pragma once

#include <components/MenuComponent.h>
#include <components/OptionListComponent.h>
#include <guis/menus/GuiMenuBase.h>
#include <components/SwitchComponent.h>

class GuiMenuThemeOptions : public GuiMenuBase
                          , private ISwitchComponent
                          , private IOptionListComponent<String>
{
  public:
    /*!
     * @brief Default constructor
     * @param window Global window
     */
    explicit GuiMenuThemeOptions(WindowManager& window);

    //! Destructor
    ~GuiMenuThemeOptions() override;

  private:
    enum class Components
    {
      Carousel,
      Transition,
      Theme,
    };

    //! Carousel
    std::shared_ptr<SwitchComponent> mCarousel;
    //! Transition
    std::shared_ptr<OptionListComponent<String>> mTransition;
    //! Teeme
    std::shared_ptr<OptionListComponent<String>> mTheme;

    //! Carousel original value
    bool mOriginalCaroussel;
    //! Transition original value
    String mOriginalTransition;
    //! Theme original value
    String mOriginalTheme;
    //! Recalbox theme indx
    int mRecalboxThemeIndex;

    //! Get O/C List
    std::vector<ListEntry<String>> GetTransitionEntries();
    //! Get Theme List
    std::vector<ListEntry<String>> GetThemeEntries();

    /*
     * IOptionListComponent<String> implementation
     */

    void OptionListComponentChanged(int id, int index, const String& value) override;

    /*
     * ISwitchComponent implementation
     */

    void SwitchComponentChanged(int id, bool& status) override;
};

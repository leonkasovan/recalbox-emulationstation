//
// Created by bkg2k on 31/10/2020.
//
#pragma once

#include <components/MenuComponent.h>
#include <components/OptionListComponent.h>
#include <guis/menus/GuiMenuBase.h>

class GuiMenuThemeConfiguration : public GuiMenuBase
                                , private IOptionListComponent<String>
{
  public:
    /*!
     * @brief Default constructor
     * @param window Global window
     */
    GuiMenuThemeConfiguration(WindowManager& window, const String& theme);

    //! Destructor
    ~GuiMenuThemeConfiguration() override;

  private:
    enum class Components
    {
      ColorSet,
      IconSet,
      MenuSet,
      SystemView,
      GamelistView,
      GameClipView,
      Region,
    };

    typedef HashMap<String, String> StringMaps;
    typedef std::shared_ptr<OptionListComponent<String>> OptionList;
    typedef std::function<void()> Callback;

    //! Theme name
    String mThemeName;
    //! Changed flag
    bool mReloadRequired;

    //! Color Set
    OptionList mColorSet;
    //! Icon Set
    OptionList mIconSet;
    //! Menu Set
    OptionList mMenuSet;
    //! System View
    OptionList mSystemView;
    //! Gamelist View
    OptionList mGameListView;
    //! Gameclip View
    OptionList mGameClipView;
    //! Region
    OptionList mRegion;

    //! Color Set
    String mOriginalColorSet;
    //! Icon Set
    String mOriginalIconSet;
    //! Menu Set
    String mOriginalMenuSet;
    //! System View
    String mOriginalSystemView;
    //! Gamelist View
    String mOriginalGameListView;
    //! Gameclip View
    String mOriginalGameClipView;
    //! Region
    String mOriginalRegion;

    /*!
     * @brief Build an option menu
     * @param label Menu label
     * @param help Help msg
     * @param selected Currently selected item
     * @param items All items
     * @param id Menu id
     * @return OptionList component
     */
    OptionList BuildSelector(const String& label, const String& help, const String& selected, const String::List& items, Components id, String& original);

    static bool TrySortNumerically(std::vector<ListEntry<String>>& list);

    /*
     * IOptionListComponent<String> implementation
     */

    void OptionListComponentChanged(int id, int index, const String& value) override;};

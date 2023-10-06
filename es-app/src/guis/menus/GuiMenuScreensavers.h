//
// Created by bkg2k on 31/10/2020.
//
#pragma once

#include <guis/menus/GuiMenuBase.h>
#include <components/MenuComponent.h>
#include <components/SliderComponent.h>
#include <components/OptionListComponent.h>

class GuiMenuScreensavers : public GuiMenuBase
                          , private ISliderComponent
                          , private IOptionListComponent<RecalboxConf::Screensaver>
                          , private IOptionListMultiComponent<String>
{
  public:
    /*!
     * @brief Default constructor
     * @param window Global window
     */
    explicit GuiMenuScreensavers(WindowManager& window, SystemManager& systemManager);

  private:
    enum class Components
    {
        Time,
        Type,
        SystemList,
    };

    //! System Manager reference
    SystemManager& mSystemManager;

    //! Get Screensaver type List
    static std::vector<ListEntry<RecalboxConf::Screensaver>> GetTypeEntries();

    //! Get System List
    std::vector<ListEntry<String>> GetSystemEntries();

    /*
     * ISliderComponent implementation
     */

    void SliderMoved(int id, float value) override;

    /*
     * IOptionListComponent<String> implementation
     */

    void OptionListComponentChanged(int id, int index, const RecalboxConf::Screensaver& value) override;

    /*
     * IOptionListMultiComponent<String> implementation
     */

    void OptionListMultiComponentChanged(int id, const String::List& value) override;
};


//
// Created by bkg2k on 04/01/2021.
//
// As part of the RECALBOX Project
// http://www.recalbox.com
//
#pragma once

#include <guis/menus/GuiMenuBase.h>

// Forward declaration
class SystemManager;
class SystemData;

class GuiMenuSystemList : public GuiMenuBase
                        , private IGuiMenuBase
{
  public:
    /*!
     * @brief Default constructor
     * @param window Global window
     */
    explicit GuiMenuSystemList(WindowManager& window, SystemManager& systemManager);

  private:
    //! System manager
    SystemManager& mSystemManager;

    // Submenu list
    HashMap<SystemData*, std::shared_ptr<TextComponent>> mMenus;

    /*
     * IGuiMenuBase implementation
     */

    void SubMenuSelected(int id) override;

    /*
     * Component Override
     */

    void onShow() override;
};




//
// Created by bkg2k on 08/01/2021.
//
// As part of the RECALBOX Project
// http://www.recalbox.com
//
#pragma once

#include <guis/menus/GuiMenuBase.h>

// Forward declaration
class SystemManager;

class GuiMenuVirtualSystemPerGenre : public GuiMenuBase
                                   , public ISwitchComponent
{
  public:
    /*!
     * @brief Default constructor
     * @param window Global window
     */
    explicit GuiMenuVirtualSystemPerGenre(WindowManager& window, SystemManager& systemManager);

  private:
    //! System manager reference
    SystemManager& mSystemManager;

    //! List of activated genre virtual system
    HashMap<String, bool> mGenres;

    //! Switch component changed
    void SwitchComponentChanged(int id, bool& status) override;
};




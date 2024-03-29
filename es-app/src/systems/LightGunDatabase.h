//
// Created by bkg2k on 08/03/2021.
//
// As part of the RECALBOX Project
// http://www.recalbox.com
//
#pragma once

#include <games/IFilter.h>
#include "SystemData.h"

class LightGunDatabase : public IFilter
{
  public:
    //! Constructor
    LightGunDatabase();

    //! Current selected system
    void SetCurrentSystem(const SystemData& system);

  private:
    //! Xml path
    static constexpr const char* sXmlPath = "/recalbox/share_init/system/.emulationstation/lightgun.cfg";

    //! All game list
    HashMap<String, String::List> mSystemLists;

    //! Current system
    const SystemData* mCurrentSystem;
    //! Game list available in the current system
    String::List* mCurrentList;

    //! Get simplified game name
    static String GetSimplifiedName(const String& name);

    //! Load the whole database
    void LoadDatabase();

    /*
     * IFilter implementation
     */

    //! Return true if the given FileGata math a game in the current database system
    bool ApplyFilter(const FileData& file) override;
};




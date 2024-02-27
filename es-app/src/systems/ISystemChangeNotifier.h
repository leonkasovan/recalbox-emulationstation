//
// Created by bkg2k on 08/07/23.
//
#pragma once

#include "ISlowSystemOperation.h"

// Forward declaration
class SystemData;
class FileData;

class ISystemChangeNotifier
{
  public:
    //! Destructor
    virtual ~ISystemChangeNotifier() = default;

    //! System must show
    virtual void ShowSystem(SystemData* system) = 0;

    //! System must hide
    virtual void HideSystem(SystemData* system) = 0;

    //! System must be updated (games have been updated inside)
    virtual void UpdateSystem(SystemData* system) = 0;

    //! Move to selected systems if possible
    virtual void SelectSystem(SystemData* system) = 0;

    /*!
     * @brief Slow operation requested to high level UI class
     * @param interface Threaded operations methods
     * @param systems System list to work on
     * @param autoSelectMonoSystem If the list contains only one system, tell the GUI to move onto this system)
     */
    virtual void RequestSlowOperation(ISlowSystemOperation* interface, ISlowSystemOperation::List systems, bool autoSelectMonoSystem) = 0;

    /*!
     * @brief The system manager notify the UI a system has been requested to show but has no game
     * So it is initialized and will automatically show up as soon as it has games
     * @param system Target system
     */
    virtual void SystemShownWithNoGames(SystemData* system) = 0;
};

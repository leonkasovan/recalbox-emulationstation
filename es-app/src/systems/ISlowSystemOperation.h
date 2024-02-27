//
// Created by bkg2k on 03/08/23.
//
#pragma once

#include <utils/storage/Array.h>

// Forward declaration
class SystemData;

/*!
 * @brief This interface is implemented in SystemManager and delayed to high level GUI class so that
 * when the system manager has to do some slow operation it can ask the GUI to process tasks
 * in delayed threads and notify the user that one or more systems are loading/populating
 */
class ISlowSystemOperation
{
  public:
    //! Convenient alias for System list - must be compatible with SystemManager::List
    typedef Array<SystemData*> List;

    //! Default constructor
    virtual ~ISlowSystemOperation() = default;

    //! Populate operation
    virtual void SlowPopulateExecute(const List& listToPopulate) = 0;

    /*!
     * @brief Operation coimpleted
     * @param listToPopulate Populated system list
     * @param autoSelectMonoSystem If the list contains only one system, tell the GUI to move onto this system)
     */
    virtual void SlowPopulateCompleted(const List& listToPopulate, bool autoSelectMonoSystem) = 0;
};
//
// Created by bkg2k on 08/07/23.
//
#pragma once

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
};

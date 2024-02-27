//
// Created by bkg2k on 14/01/2021.
//
// As part of the RECALBOX Project
// http://www.recalbox.com
//
#pragma once

class IInputChange
{
  public:
    /*!
     * @brief Called by the InputManager right after the device list is refreshed
     * because of a pad added or removed
     * @param True if a pad has been removed, false if a pad has been added
     */
    virtual void PadsAddedOrRemoved(bool removed) = 0;
};

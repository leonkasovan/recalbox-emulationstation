//
// Created by bkg2k on 15/07/23.
//
#pragma once

class ISaveStateSlotNotifier
{
  public:
    //! Virtual destructor
    virtual ~ISaveStateSlotNotifier() = default;

    /*!
     * @brief Notifie a slot has been selected
     * @param slot Slot number or -1
     */
    virtual void SaveStateSlotSelected(int slot) = 0;
};

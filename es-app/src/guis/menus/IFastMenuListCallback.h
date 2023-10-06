//
// Created by bkg2k on 12/07/23.
//
#pragma once

class IFastMenuListCallback
{
  public:
    //! Default destructor
    virtual ~IFastMenuListCallback() = default;

    /*!
     * @brief Main callback
     * @param menuIndex Menu identifier to identify multiple menu in a single callback
     * @param itemIndex Item index, starting from
     */
    virtual void FastMenuLineSelected(int menuIndex, int itemIndex) = 0;
};

//
// Created by bkg2k on 02/10/23.
//
#pragma once

#include <utils/String.h>

class IRecalboxConfChanged
{
  public:
    //! Default virtal destructor
    virtual ~IRecalboxConfChanged() = default;

    /*!
     * @brief Callback when a watched configuration changed
     * @param key Changed key
     */
    virtual void ConfigurationChanged(const String& key) = 0;
};
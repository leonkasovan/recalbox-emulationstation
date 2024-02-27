//
// Created by bkg2k on 07/06/23.
//
#pragma once

#include <vector>
#include <utils/String.h>

class ArcadeVirtualSystems
{
  public:
    static constexpr const char* sAllOtherManufacturers = "allothers";

    /*!
     * @brief Return top list of arcade manufacturer
     * @return Arcade driver list
     */
    static const String::List& GetVirtualArcadeSystemList();

    /*!
     * @brief Return top list of arcade manufacturer, associated to their user-friendly names
     * @return Arcade driver list
     */
    static const std::vector<std::pair<String, String>>& GetVirtualArcadeSystemListExtended();
};

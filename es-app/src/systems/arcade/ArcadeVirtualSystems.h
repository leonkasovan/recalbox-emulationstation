//
// Created by bkg2k on 07/06/23.
//
#pragma once

#include <vector>
#include <utils/String.h>

class ArcadeVirtualSystems
{
  public:
    static constexpr const char* sAllOtherDriver = "allothers";
    /*!
     * @brief Return known list of arcade driver
     * @return Arcade driver list
     */
    static const String::List& GetVirtualArcadeSystemList();

    /*!
     * @brief Get a friendly name from an arcade driver
     * @param driverName Arcade driver
     * @return Freindly name or driver name if no match is found
     */
    static String GetRealName(const String& driverName);
};

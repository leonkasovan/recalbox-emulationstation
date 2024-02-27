//
// Created by bkg2k on 07/06/23.
//
#pragma once

#include <vector>
#include <utils/String.h>
#include <systems/arcade/ArcadeDatabase.h>

class IArcadeGamelistInterface
{
  public:
    //! Destructor
    virtual ~IArcadeGamelistInterface() = default;

    /*!
     * @brief Retrive the system the interface is attached to
     * @return System
     */
    [[nodiscard]] virtual const SystemData& GetAttachedSystem() const = 0;

    /*!
     * @brief Check if the Arcade game list has a valid database for the current emulator/core
     * @return
     */
    [[nodiscard]] virtual bool HasValidDatabase() const = 0;

    /*!
     * @brief Get manufacturer list (arcade view only)
     * @return Manufacturer list
     */
    [[nodiscard]] virtual std::vector<ArcadeDatabase::Manufacturer> GetManufacturerList() const = 0;

    /*!
     * @brief Get the current emulator name for the current folder
     * @return Emulator name
     */
    [[nodiscard]] virtual String GetCurrentEmulatorName() const = 0;

    /*!
     * @brief Get the current core name for the current folder
     * @return Core name
     */
    [[nodiscard]] virtual String GetCurrentCoreName() const = 0;

    /*!
     * @brief Get the number of games attached to the given manufacturer, in the current gamelist
     * @param manufacturerIndex Manufacturer index
     * @return Game count
     */
    [[nodiscard]] virtual int GetGameCountForManufacturer(int manufacturerIndex) const = 0;

    /*!
     * @brief Check if there is more than one manufacturer so that they can be filtered by the user
     * @return
     */
    [[nodiscard]] virtual bool CanBeFiltered() const = 0;
};
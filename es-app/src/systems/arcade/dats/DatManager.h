//
// Created by bkg2k on 17/06/23.
//
#pragma once

#include "systems/SystemData.h"
#include "DatContent.h"
#include "ScanResult.h"

class DatManager
{
  public:
    /*!
     * @brief Constructor
     * @param system System for witch to load dat files
     */
    explicit DatManager(const SystemData& system);

    // Destructor
    ~DatManager();

    /*!
     * @brief Scan the given game and return a ScanResult struture
     * @param game Game to scan
     * @return Scan results
     */
    ScanResult Scan(const FileData& game);

  private:
    //! Hold dar contents per emulator/core couple
    HashMap<String, DatContent*> mDatPerEmulatorCore;

    /*!
     * @brief Get a unique key per couple of emulator/core
     * @param emulator Emulator name
     * @param core Core name
     * @return Unique key
     */
    static String Key(const String& emulator, const String& core) { return String(emulator).Append('|').Append(core); }

    /*!
     * @brief Load all dats available for all emulator/core in this system
     * @param system target System
     */
    void LoadAllDats(const SystemData& system);

    /*!
     * @brief Try to load dat file for the given emulator/core couple and store it into the map
     * @param emulator
     * @param core
     * @param databasePath flat database filename
     */
    void LoadDat(const String& emulator, const String& core, const String& databaseFilename);
};

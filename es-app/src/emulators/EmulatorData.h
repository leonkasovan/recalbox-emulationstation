//
// Created by Bkg2k on 19/02/2020.
//
#pragma once

#include <utils/String.h>

struct EmulatorData
{
  private:
    //! Emulator
    String mEmulator;
    //! Core
    String mCore;

  public:
    /*!
     * @brief Build an EmulatorData with the given emulator name and core name
     * @param emulator Emulator name
     * @param core Core name
     */
    EmulatorData(const String& emulator, const String& core)
      : mEmulator(emulator),
        mCore(core)
    {
    }

    /*
     * Accessors
     */

    //! Get emulator
    [[nodiscard]] const String& Emulator() const { return mEmulator; }

    //! Get emulator
    [[nodiscard]] const String& Core() const { return mCore; }

    //! Valid?
    [[nodiscard]] bool IsValid() const { return !mEmulator.empty() && !mCore.empty(); }

    //! Is libretro
    [[nodiscard]] bool IsLibretro() const { return "libretro" == mEmulator; }
};
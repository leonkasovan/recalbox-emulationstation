//
// Created by bkg2k on 26/12/2021.
//
// As part of the RECALBOX Project
// http://www.recalbox.com
//
#pragma once

#include <emulators/run/NetPlayData.h>
#include <emulators/run/CrtData.h>
#include <emulators/run/PatchData.h>
#include "SuperGameBoyData.h"
#include <emulators/run/SaveStateData.h>
#include "JammaData.h"

class GameLinkedData
{
  public:
    /*!
     * @brief Configure Netplay data as client
     * @param coreName Libretro core
     * @param ip Target ip
     * @param port Target port
     * @param playerPassword Password for players
     * @param viewerPassword Password for viewers
     * @param asViewer True to oin as viewer, false to join as player
     */
    GameLinkedData(const String& coreName, const String& ip, int port, const String& playerPassword, const String& viewerPassword, bool asViewer)
      : mNetPlayData(coreName, ip, port, playerPassword, viewerPassword, asViewer)
    {
    }

    /*!
     * @brief Configure netplay data as host
     * @param port Listening port
     * @param playerPassword Password for players
     * @param viewerPassword Password for viewers
     */
    GameLinkedData(int port, const String& playerPassword, const String& viewerPassword)
      : mNetPlayData(port, playerPassword, viewerPassword)
    {
    }

    //! Default constructor
    GameLinkedData() = default;

    /*
     * Accessors
     */

    //! Get Netplay object
    [[nodiscard]] const NetPlayData& NetPlay()const { return mNetPlayData; }

    //! Get writable Crt object
    CrtData& ConfigurableCrt() { return mCrtData; }

    //! Get read only Crt object
    [[nodiscard]] const CrtData& Crt() const { return mCrtData; }

    //! Get writable Patch data
    PatchData& ConfigurablePatch() { return mPatchData; }

    //! Get writable savestate data
    SaveStateData& ConfigurableSaveState() { return mSaveSateData; }

    //! Get read only Patch data
    [[nodiscard]] const PatchData& Patch() const { return mPatchData; }

    //! Get writable Supergameboy data
    SuperGameBoyData& ConfigurableSuperGameBoy() { return mSuperGameBoyData; }

    //! Get read only Supergameboy data
    [[nodiscard]] const SuperGameBoyData& SuperGameBoy() const { return mSuperGameBoyData; }

    //! Get read only savestate data
    [[nodiscard]] const SaveStateData& SaveState() const { return mSaveSateData; }

    //! Get read only Jamma data
    const JammaData& Jamma() const { return mJammaData; }

  private:
    //! Netplay data
    NetPlayData mNetPlayData;

    //! CRT data
    CrtData mCrtData;

    PatchData mPatchData;

    //! SuperGameBoyData
    SuperGameBoyData mSuperGameBoyData;

    SaveStateData mSaveSateData;
    JammaData mJammaData;
};

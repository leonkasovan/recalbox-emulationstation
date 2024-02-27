//
// Created by bkg2k on 26/12/2021.
//
// As part of the RECALBOX Project
// http://www.recalbox.com
//
#pragma once

#include <hardware/Board.h>
#include <CrtConf.h>
#include "../EmulatorData.h"

class CrtData
{
  public:
    //! Video system
    enum class CrtRegion
    {
      AUTO, //!< Automatic selection
      JP,  //!< Forced Japan
      US, //!< Forced US
      EU, //!< Forced Europe
    };

    enum class CrtVideoStandard
    {
        AUTO, //!< Automatic selection
        PAL,  //!< Forced Pal
        NTSC, //!< Forced Ntsc
    };

    //! Default constructor
    CrtData()
      : mCrt(&Board::Instance().CrtBoard())
      , mConf(&CrtConf::Instance())
      , mRegionOrVideoStandardConfigured(false)
      , mHighResolutionConfigured(false)
      , mVideoStandard(CrtVideoStandard::AUTO)
      , mRegion(CrtRegion::AUTO)
      , mHighResolution(Board::Instance().CrtBoard().GetHorizontalFrequency() == ICrtInterface::HorizontalFrequency::KHz31)
    {
    }

    /*!
     * @brief Check if there is a CRT board and the user requested to choose individual 480 or 240 options
     * @return True if the class needs to be configured, false otherwise
     */
    [[nodiscard]] bool IsResolutionSelectionConfigured() const
    {
      if (!mHighResolutionConfigured)
        if (mCrt->IsCrtAdapterAttached())
          if (mConf->GetSystemCRTGameResolutionSelect())
            return true;
      return false;
    }

    /*!
     * @brief Check if there is a CRT board and the user requested to choose individual NTSC options
     * @return True if the class needs to be configured, false otherwise
     */
    [[nodiscard]] bool IsRegionOrStandardConfigured() const
    {
      if (!mRegionOrVideoStandardConfigured)
        if (mCrt->IsCrtAdapterAttached())
          if (mConf->GetSystemCRTGameRegionSelect())
            return true;
      return false;
    }

    /*!
     * @brief Configure crt data
     * @param ntsc True for NTSC, false for PAL
     */
    void ConfigureVideoStandard(CrtVideoStandard standard)
    {
        mVideoStandard = standard;
        mRegionOrVideoStandardConfigured = true;
    }

    void ConfigureRegion(CrtRegion region)
    {
        mRegion = region;
        mRegionOrVideoStandardConfigured = true;
    }

    /*!
     * @brief Configure crt data
     * @param highRez True for 480, false for 240
     */
    void ConfigureHighResolution(bool highRez)
    {
      if (!mHighResolutionConfigured)
      {
        mHighResolution = highRez;
        mHighResolutionConfigured = true;
      }
    }
    /*!
     * @brief Auto configure high resolution depending on the mode
     * @param highRez True for 480, false for 240
     */
    void AutoConfigureHighResolution(SystemData& system)
    {
      if (!mHighResolutionConfigured)
        ConfigureHighResolution((system.Descriptor().CrtHighResolution() && Board::Instance().CrtBoard().HasInterlacedSupport()) || Board::Instance().CrtBoard().GetHorizontalFrequency() == ICrtInterface::HorizontalFrequency::KHz31);
    }

    /*!
     * @brief Check if the target system requires choosing between PAL or NTSC
     * @param system target system
     * @return True if the choice is required, false otherwise
     */
    [[nodiscard]] bool MustChoosePALorNTSC(const SystemData& system) const
    {
      return system.Descriptor().CrtMultiRegion() &&        // System must support multi-region
             mCrt->IsCrtAdapterAttached() &&
             !mCrt->MustForce50Hz() && // & hardware must not force 50hz
             mCrt->GetHorizontalFrequency() == ICrtInterface::HorizontalFrequency::KHz15; // & and we are 15khz
    }

    /*!
     * @brief Check if the target system requires choosing between 240 or 480
     * @param game target game
     * @return True if the choice is required, false otherwise
     */
    [[nodiscard]] bool MustChooseHighResolution(FileData* game, const EmulatorData& emulator) const
    {
      bool gameCanRunInHd = game->System().Descriptor().CrtHighResolution();
      if(game->System().IsArcade())
      {
        String emu = emulator.Emulator();
        String core =  emulator.Core();
        const ArcadeDatabase* database = game->System().ArcadeDatabases().LookupDatabase(*game, emu, core);
        if (database != nullptr){
          const ArcadeGame* arcade = database->LookupGame(*game);
          if(arcade != nullptr)
            gameCanRunInHd |= (arcade->ScreenRotation() == ArcadeGame::Rotation::Noon && arcade->Height() >= 480);
        }
      }
      // If 15Khz, the system must support high rez and the interlaced must be supported by board
      // If 31khz, the board must support 120Hz
      // If multisync, return true
      return (gameCanRunInHd && Board::Instance().CrtBoard().GetHorizontalFrequency() == ICrtInterface::HorizontalFrequency::KHz15 && Board::Instance().CrtBoard().HasInterlacedSupport())
      || (Board::Instance().CrtBoard().GetHorizontalFrequency() == ICrtInterface::HorizontalFrequency::KHz31 && Board::Instance().CrtBoard().Has120HzSupport())
      || (Board::Instance().CrtBoard().MultiSyncEnabled());
    }

    /*
     * Accessors
     */

    [[nodiscard]] bool HighResolution() const { return mHighResolution; }
    [[nodiscard]] CrtScanlines Scanlines(const SystemData& system) const
    {
      return (HighResolution() && !system.Descriptor().CrtHighResolution() &&
                    (Board::Instance().CrtBoard().GetHorizontalFrequency() == ICrtInterface::HorizontalFrequency::KHz31 ||
                     Board::Instance().CrtBoard().GetHorizontalFrequency() == ICrtInterface::HorizontalFrequency::KHzMulti)) ?
                   CrtConf::Instance().GetSystemCRTScanlines31kHz() : CrtScanlines::None;
    }
    [[nodiscard]] CrtVideoStandard VideoStandard() const { return mVideoStandard; }
    [[nodiscard]] CrtRegion Region() const { return mRegion; }

  private:
    //! ICrtInterface reference
    ICrtInterface* mCrt;
    //! Configuration
    CrtConf* mConf;
    //! NTSC configured
    bool mRegionOrVideoStandardConfigured;
    //! 480i configured
    bool mHighResolutionConfigured;
    //! Video system (default: auto
    CrtVideoStandard mVideoStandard;
    CrtRegion mRegion;
    //! 480? (default: 240p)
    bool mHighResolution;
};
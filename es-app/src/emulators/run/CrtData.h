//
// Created by bkg2k on 26/12/2021.
//
// As part of the RECALBOX Project
// http://www.recalbox.com
//
#pragma once

#include <hardware/Board.h>
#include <CrtConf.h>

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
      , mVideoStandard(CrtVideoStandard::AUTO)
      , mRegion(CrtRegion::AUTO)
      , mHighResoution(false)
    {
    }

    /*!
     * @brief Check if there is a CRT board and the user requested to choose individual 480 or 240 options
     * @return True if the class needs to be configured, false otherwise
     */
    [[nodiscard]] bool IsHighResolutionConfigured() const
    {
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
    }

    void ConfigureRegion(CrtRegion region)
    {
      mRegion = region;
    }

    /*!
     * @brief Configure crt data
     * @param highRez True for 480, false for 240
     */
    void ConfigureHighResolution(bool highRez)
    {
      mHighResoution = highRez;
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
     * @param system target system
     * @return True if the choice is required, false otherwise
     */
    [[nodiscard]] bool MustChooseHighResolution(const SystemData& system) const
    {
      return system.Descriptor().CrtHighResolution();
    }
/*
  *//*!
  * @brief Check if high resolution is a progressive one
  * @return True if high resolution is a progressive one, false otherwise
  *//*
  bool HighResolutionIsProgressive() const
  {

    if (mCrt.GetHorizontalFrequency() == ICrtInterface::HorizontalFrequency::KHz31)

  }*/

    /*
     * Accesors
     */

    [[nodiscard]] bool HighResolution() const { return mHighResoution; }

    [[nodiscard]] CrtVideoStandard VideoStandard() const { return mVideoStandard; }
    [[nodiscard]] CrtRegion Region() const { return mRegion; }

  private:
    //! ICrtInterface reference
    ICrtInterface* mCrt;
    //! Configuration
    CrtConf* mConf;
    //! Video system (default: auto
    CrtVideoStandard mVideoStandard;
    CrtRegion mRegion;
    //! 480? (default: 240p)
    bool mHighResoution;
};
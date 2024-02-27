//
// Created by bkg2k on 19/12/2021.
//
// As part of the RECALBOX Project
// http://www.recalbox.com
//
#pragma once

#include <hardware/crt/CrtAdapterType.h>
#include <hardware/BoardType.h>
#include <string>

class ICrtInterface
{
  public:
    enum class HorizontalFrequency
    {
      KHz15, //!< 15 Khz
      KHz31, //!< 31 Khz
      KHzMulti, //!< MultiSync
    };

    /*!
     * @brief Default constructor
     * @param automaticallyDetected True if the board has been automatically detected
     */
    explicit ICrtInterface(bool automaticallyDetected, BoardType boardType) : mAutomaticallyDetected(automaticallyDetected), mBoardType(boardType) {}

    //! Default destructor
    virtual ~ICrtInterface() = default;

    /*!
     * @brief Check if a CRT adpater is attached
     * @return True if a CRT adapter is attached, false otherwise
     */
    virtual bool IsCrtAdapterAttached() const = 0;

    /*!
     * @brief Get CRT adapter
     * @return CRT adater
     */
    virtual CrtAdapterType GetCrtAdapter() const = 0;

    /*!
     * @brief Check if the adapter supports 31khz
     * @return True if the adapter supports 31khz
     */
    virtual bool Has31KhzSupport() const = 0;

    /*!
     * @brief Check if the adapter supports 120hz modes
     * @return True if the adapter supports 120hz modes
     */
    virtual bool Has120HzSupport() const { return false; }


    /*!
     * @brief MultiSync support
     * @return True if the multisync is enabled
     */
    virtual bool MultiSyncEnabled() const { return false; }

    /*!
     * @brief Check if the adapter supports multisync 15/31khz
     * @return True if the adapter supports multisync 15/31khz
     */
    virtual bool HasMultiSyncSupport() const { return false; }

    /*!
     * @brief Get horizontal frequency
     * @return Horitontal frequency
     */
    virtual HorizontalFrequency GetHorizontalFrequency() const = 0;

    /*!
     * @brief Check if the adapter has forced 50hs support
     * @return True if the adapter support forcing 50hz mode, false otherwise
     */
    virtual bool HasForced50hzSupport() const = 0;

    /*!
     * @brief Check if the adapter force 50hz mode
     * @return True if the adapter force 50hz mode, otherwize automatic mode
     */
    virtual bool MustForce50Hz() const = 0;

    /*!
     * @brief Returns the name of the adapter
     * @return the name of the adapter
     */
    virtual std::string& Name() const = 0;

    /*!
     * @brief Returns the short name of the adapter
     * @return the short name of the adapter
     */
    virtual std::string& ShortName() const = 0;
    /*!
     * @brief Check if this board has been automatically detected
     * @return True of the board has been automatically detected, false otherwise
     */
    bool HasBeenAutomaticallyDetected() const { return mAutomaticallyDetected; }

    /*!
     * @brief Check if combo adapter/board supports interlaced
     */
    [[nodiscard]] bool HasInterlacedSupport() const { return mBoardType != BoardType::Pi5; }

  private:
    //! Automatically detected?
    bool mAutomaticallyDetected;
    BoardType mBoardType;
};

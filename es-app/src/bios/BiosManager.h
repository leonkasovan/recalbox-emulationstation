//
// Created by bkg2k on 28/12/2019.
//
#pragma once

#include <vector>
#include <utils/os/system/Thread.h>
#include <utils/sync/SyncMessageSender.h>
#include <utils/cplusplus/StaticLifeCycleControler.h>
#include "BiosList.h"
#include "IBiosScanReporting.h"

struct BiosMessage
{
  IBiosScanReporting* mReporting;
  int mListIndex;
  int mIndex;
  bool mComplete;

  /*!
   * @brief Statically build a single bios report message
   * @param report Reporting interface
   * @param list List index
   * @param index Bios index
   * @return Bios message
   */
  static BiosMessage SingleBiosReport(IBiosScanReporting* report, int list, int index)
  {
    return { .mReporting = report, .mListIndex = list, .mIndex = index, .mComplete = false };
  }
  /*!
   * @brief Statically build a bios report completion message
   * @param report Reporting interface
   * @return Bios message
   */
  static BiosMessage EndBiosReport(IBiosScanReporting* report)
  {
    return { .mReporting = report, .mListIndex = 0, .mIndex = 0, .mComplete = true };
  }
};

class BiosManager : public StaticLifeCycleControler<BiosManager>
                  , private Thread
                  , public ISyncMessageReceiver<BiosMessage>
{
  private:
    //! Path to bios.xml file
    static constexpr const char* sBiosFilePath = "system/.emulationstation/es_bios.xml";
    //! Path to report file
    static constexpr const char* sReportPath = "bios/missing_bios_report.txt";

    //! Bios list per system
    std::vector<BiosList> mSystemBiosList;
    //! Sync'ed event sender
    SyncMessageSender<BiosMessage> mSender;
    //! Current scan's reporting interface (also flag for an already running scan)
    IBiosScanReporting* mReporting;
    //! Protector
    Mutex& mLocker;

    /*
     * Thread implementation
     */

    /*!
     * Threaded scan implementation
     */
    void Run() override;

    /*
     * ISynchronousInterface
     */

    /*!
     * @brief Receive synchronous SDL2 event
     * @param event SDL event with .user populated by the sender
     */
    void ReceiveSyncMessage(const BiosMessage& event) override;

  public:
    enum LookupResult
    {
      NotFound,      //!< Bios name not found or MD5 does not match any known md5
      Found,         //!< Bios has been found either by name or MD5
      AlreadyExists, //!< The given bios already exists with the same
    };

    /*!
     * @brief Default constructor
     */
    BiosManager();

    /*!
     * @brief Default destructor
     */
    ~BiosManager() override { Thread::Join(); delete &mLocker; }

    /*!
     * @brief Load all bios from bios.xml
     */
    void LoadFromFile();

    //! Get system count
    [[nodiscard]] int SystemCount() const
    {
      Mutex::AutoLock locker(mLocker);
      return (int)mSystemBiosList.size();
    }

    /*!
     * @brief Get bios list from the given system index
     * @param index System index
     * @return BiosList object
     */
    [[nodiscard]] const BiosList& SystemBios(int index) const
    {
      Mutex::AutoLock locker(mLocker);
      return mSystemBiosList[index];
    }

    /*!
     * @brief Lookup bios list for the given system
     * @param name System name
     * @return Bios list or nullptr if not found
     */
    [[nodiscard]] const BiosList& SystemBios(const String& name) const;

    /*!
     * @brief Start scanning all bios and report result using the given interface
     * @param reporting Reporting interface
     */
    void Scan(IBiosScanReporting* reporting, bool sync = false);

    /*!
     * @brief Try to lookup the given bios in all systems
     * @param name Bios name
     * @param md5 Bios md5
     * @return LookupResult and outputPath set to bios path or empty path
     */
    LookupResult Lookup(const std::string& name, const std::string& md5, const Bios*& outputBios) const;

    /*!
     * @brief Generate missing bios report in bios root folder
     */
    void GenerateReport() const;

    //! Global move status
    [[nodiscard]] bool Moved() const;
    //! Global move error status
    [[nodiscard]] bool MoveError() const;
};

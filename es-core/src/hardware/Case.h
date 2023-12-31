//
// Created by digitalLumberjack on 17/06/2022.
//
#pragma once

#include <utils/String.h>

#define CASE_DETECTION_AUTOMATIC true
#define CASE_ROTATION_SUPPORTED true
#define MENU_SHUTDOWN_ENABLED true

/*!
* Case management
*
*/
class Case
{
  public:
    enum class CaseModel
    {
      // Automatic
      GPiV1,
      GPiV2,
      GPiV3,
      GPi2,
      GPi2W,
      Nuxii,
      PiBoyDMG,
      PiBoyXRS,
      // Retroflag auto
      Nespi4Case,
      // Retroflag Manual
      Nespi4CaseManual,
      SuperPi4Case,
      NespiCasePlus,
      PiStation,
      SuperPiCase,
      MegaPiCase,
      ArgonOne,
      RaspberryPiTouchDisplay,
      RecalboxRGBDualOrRGBHat,
      None,
    };

    static constexpr const char* sPiboyBatteryCapacityPath = "/sys/kernel/xpi_gamecon/percent";
    static constexpr const char* sPiboyAmpsPath = "/sys/kernel/xpi_gamecon/amps";

    /*!
     * @brief Return the current model enum of this case
     * @return the current model enum of this case
     */
    [[nodiscard]] CaseModel Model() const { return mModel; }

    /*!
     * @brief Some cases are automatically detected by the system
     * @return true if the case is automatically detexted by the system
     */
    [[nodiscard]] bool Automatic() const { return mAutomatic; }

    /*!
     * @brief Some cases have an on/off button with a state
     * @return true if the case allows shutting down recalbox from the menu
     */
    [[nodiscard]] bool CanShutdownFromMenu() const { return mMenuShutdownEnabled; }

    /*!
     * @brief Case name should be human readable
     * @return a nice and hamun readable name
     */
    [[nodiscard]] const String& DisplayName() const { return mDisplayName; }

    /*!
     * @brief The case name in Recalbox system (used in config files and so)
     * @return the short name of the case
     */
    [[nodiscard]] const String& ShortName() const { return mShortName; }

    /*!
     * @brief Install the case on the system. If the case is None, it will uninstall cases.
     */
    [[nodiscard]] bool Install() const;

    /*!
     * @brief Uninstall the case
     */
    bool Uninstall() const;

    /*!
     * @brief Returns the install message
     * @return the installe message
     */
    [[nodiscard]] String GetInstallMessage() const { return mInstallMessage; }


    /*!
     * @brief A case can support rotation or not
     * @return true if supports rotation
     */
    [[nodiscard]] bool RotationSupported() const { return mRotationSupported; }

    /*!
     * @brief Get the case from short name
     * @param the short name of the case
     * @return the case or a None case if not found
     */
    static Case FromShortName(const String& value);

    /*!
     * @brief Factory constructor by model
     * @param the model id
     * @return the case instance of the model
     */
    static Case Create(CaseModel model);

    /*!
     * @brief Returns the installed case (auto or manual)
     * @return the installed case
     */
    static Case CurrentCase();

    /*!
     * @brief Each board can support its own cases
     * @return supported cases depending on the current board
     */
    static std::vector<Case> SupportedManualCases();

  private:
    /*!
     * @brief Private constructor
     * @param model Case model
     * @param automatic Detected automatically?
     * @param menuShutdownEnabled Display or not shutdown option in the menu
     * @param displayName Displayable name
     * @param shortName Internal name
     */
    Case(CaseModel model, bool automatic, bool menuShutdownEnabled, bool rotationSupported, const String& displayName, const String& shortName, const String& installMessage)
      : mDisplayName(displayName)
      , mShortName(shortName)
      , mInstallMessage(installMessage)
      , mModel(model)
      , mAutomatic(automatic)
      , mMenuShutdownEnabled(menuShutdownEnabled)
      , mRotationSupported(rotationSupported)
     {}

    static bool SetCaseInBoot(const String& theCase);
    const String mDisplayName;
    const String mShortName;
    const String mInstallMessage;
    const enum CaseModel mModel;
    const bool mAutomatic;
    const bool mMenuShutdownEnabled;
    const bool mRotationSupported;
};

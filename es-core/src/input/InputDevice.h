#pragma once

#include <utils/String.h>
#include <input/InputEvent.h>
#include <input/InputCompactEvent.h>
#include <SDL2/SDL_joystick.h>
#include <pugixml/pugixml.hpp>

/*!
 * @brief Hold input configurations for a given device
 */
class InputDevice
{
  public:
    //! Joystick deadzone, in the 0-32767 range
    static constexpr int sJoystickDeadZone = 23000;

    //! Udev information
    enum class UDevInfo
    {
      LowlevelName, //!< Name in ID_VENDOR_END - ID_MODEL_ENC
      UniqueID,     //!< ATTRS{uniq}
      PowerPath,    //!< Path to battery information in /sys/class/power_supply/device-ATTRS{uniq}
    };

    //! Input entry
    enum class Entry
    {
      None,
      Start,
      Select,
      Hotkey,
      A,
      B,
      X,
      Y,
      L1,
      R1,
      L2,
      R2,
      L3,
      R3,
      Up,
      Right,
      Down,
      Left,
      Joy1AxisH,
      Joy1AxisV,
      Joy2AxisH,
      Joy2AxisV,
      VolumeUp,
      VolumeDown,
      BrightnessUp,
      BrightnessDown,
      __Count,
    };

    //! Maximum Axis
    static constexpr int sMaxAxis = 8;
    //! Maximum hats
    static constexpr int sMaxHats = 2;
    //! Maximum buttons
    static constexpr int sMaxButtons = 32;

  private:
    InputEvent mInputEvents[(int)Entry::__Count]; //!< Entry configurations
    String mDeviceName;                           //!< Device name: Keyboard or Pad/joystick name
    String mUDevDeviceName;                       //!< Device name as declared in low level udev device
    String mUDevUniqID;                           //!< Device unique id as in ATTRS{uniq} from parent udev device
    Path mUDevPowerPath;                          //!< Battery information path
    Path mPath;                                   //!< udev Path
    SDL_JoystickGUID mDeviceGUID;                 //!< GUID
    SDL_Joystick* mDeviceSDL;                     //!< SDL2 structure
    int mLastTimeBatteryCheck;                    //!< Last time the battery level has been read
    int mBatteryLevel;                            //!< Battery level from 0 to 100
    int mDeviceId;                                //!< SDL2 Joystick Identifier
    int mDeviceIndex;                             //!< SDL2 Joystick Index
    int mDeviceNbAxes;                            //!< Axis count
    int mDeviceNbHats;                            //!< Hat count
    int mDeviceNbButtons;                         //!< Button count
    int mConfigurationBits;                       //!< Configured entries bitflag
    int mPreviousHatsValues[sMaxHats];            //!< Recorded hat value for move detection
    int mPreviousAxisValues[sMaxAxis];            //!< Recorded axis value for move detection
    int mNeutralAxisValues[sMaxAxis];             //!< Neutral axis values
    bool mBatteryCharging;                        //!< Pad's battery is in charge
    bool mIsATruePad;                             //!< Allow elimination of SDL2 false pads

    //! Reference time of activated entries
    static unsigned int mReferenceTimes[32];

    //! This special flags is managed by configuration UI to keep the
    //! "configure" flag in compact event while entries are configured
    bool mConfiguring;

    //! Hotkey state
    bool mHotkeyState;
    //! Select has been used to send hotkey events, don't send select!
    bool mKillSelect;

    /*!
     * @brief Convert Entry to Compact Entry.
     * Joysticks are converted to their negative position (Up, Left)
     * @param source source entry
     * @return compact entry
     */
    static InputCompactEvent::Entry ConvertEntry(Entry source) __attribute((const));

    /*!
     * @brief Build a new Button event to on/off event flags regarding current device configuration
     * @param button Button id
     * @param pressed Button state - true = pressed, false = released
     * @param on output activated entries
     * @param off output deactivated entries
     * @return elapsed time
     */
    int ConvertButtonToOnOff(int button, bool pressed, InputCompactEvent::Entry& on, InputCompactEvent::Entry& off);

    /*!
     * @brief Build a new Button event to on/off event flags regarding current device configuration
     * @param button id
     * @param pressed Key state - true = pressed, false = released
     * @param on output activated entries
     * @param off output deactivated entries
     * @return elapsed time
     */
    int ConvertMouseButtonToOnOff(int button, bool pressed, InputCompactEvent::Entry& on, InputCompactEvent::Entry& off);

    /*!
     * @brief Build a new Mouse wheel event to on/off event flags regarding current device configuration
     * @param wheel x/y
     * @param direction wheel direction 1/0/-1
     * @param on output activated entries
     * @param off output deactivated entries
     * @return elapsed time
     */
    int ConvertMouseWheelToOnOff(int wheel, int direction, InputCompactEvent::Entry& on, InputCompactEvent::Entry& off);

    /*!
     * @brief Build a new key event to on/off event flags regarding current device configuration
     * @param key Key id
     * @param pressed Key state - true = pressed, false = released
     * @param on output activated entries
     * @param off output deactivated entries
     * @return elapsed time
     */
    int ConvertKeyToOnOff(int key, bool pressed, InputCompactEvent::Entry& on, InputCompactEvent::Entry& off);

    /*!
     * @brief Build a new hat event to on/off event flags regarding current device configuration
     * @param hat Hat id
     * @param bits Hat current state
     * @param on output activated entries
     * @param off output deactivated entries
     * @return elapsed time
     */
    int ConvertHatToOnOff(int hat, int bits, InputCompactEvent::Entry& on, InputCompactEvent::Entry& off);

    /*!
     * @brief Build a new axis event to on/off event flags regarding current device configuration
     * @param axis Axis id
     * @param value Axis current value
     * @param on output activated entries
     * @param off output deactivated entries
     * @return
     */
    int ConvertAxisToOnOff(int axis, int value, InputCompactEvent::Entry& on, InputCompactEvent::Entry& off);

    /*!
     * @brief Record time reference or get elapsed time of a global entry regarding the active flag
     * @param entry Entry tu record/get elapsed time
     * @param active Activate/deactivate entry
     * @param on Activated bit flags
     * @param off Deactivated bit flags
     * @return Elapsed time or 0 if the action is activated
     */
    static int SetEntry(InputCompactEvent::Entry entry, bool active, InputCompactEvent::Entry& on, InputCompactEvent::Entry& off);

    /*!
     * @brief Record global reference time the first time en entry is activated
     * Once an entry is recorded, the same entry from another pad is ignored
     * Thus, only the very firest entry is recorded and will give the longer time
     * @param entry source entry to record
     * @param bits target entries bitflag
     */
    static void StartEntry(InputCompactEvent::Entry entry, InputCompactEvent::Entry& bits);

    /*!
     * @brief Stop recording time of the given entry
     * Only the very first release will give a recording time. Any other release from other pads
     * after an entry has been already stopped will give a time of 0
     * @param entry source entry to stop recording
     * @param bits target entries bitflag
     * @return Elapsed time between entry activation and deactivation
     */
    static int StopEntry(InputCompactEvent::Entry entry, InputCompactEvent::Entry& bits);

    /*!
     * @brief Check if Hotkey and select are assigned to the same key
     * @return True if both Hotkey and Select are mapped to the same key
     */
    bool IsHotKeyAndSelectKeysTheSame() { return mInputEvents[(int)Entry::Hotkey].EqualsTo(mInputEvents[(int)Entry::Select]); }

  public:
    /*!
     * @brief Default constructor
     * @return Empty InputDevice
     */
    InputDevice()
      : mDeviceGUID {}
      , mDeviceSDL(nullptr)
      , mLastTimeBatteryCheck(0)
      , mBatteryLevel(-1)
      , mDeviceId(0)
      , mDeviceIndex(0)
      , mDeviceNbAxes(0)
      , mDeviceNbHats(0)
      , mDeviceNbButtons(0)
      , mConfigurationBits(0)
      , mPreviousHatsValues{ 0 }
      , mPreviousAxisValues{ 0 }
      , mNeutralAxisValues{ 0 }
      , mBatteryCharging(false)
      , mIsATruePad(false)
      , mConfiguring(false)
      , mHotkeyState(false)
      , mKillSelect(false)
    {
    }

    /*!
     * @brief Constructor
     * @param deviceId Device identifier
     * @param deviceIndex Device index
     * @param deviceName Device name
     * @param deviceGUID Device GUID
     * @param deviceNbAxes Number of axises
     * @param deviceNbHats Number of hats
     * @param deviceNbButtons Numper of buttons
     */
    InputDevice(SDL_Joystick* device, SDL_JoystickID deviceId, int deviceIndex, const String& deviceName,
                const SDL_JoystickGUID& deviceGUID, int deviceNbAxes, int deviceNbHats, int deviceNbButtons);

    /*!
     * @brief Copy constructor
     * @param source source object to copy
     */
    InputDevice(const InputDevice& source) = default;

    /*!
     * @brief Default assignment operator
     * @param source source object to copy
     * @return Self
     */
    InputDevice& operator =(const InputDevice& source) = default;

    /*
     * Accessors
     */

    //[[nodiscard]] String Name() const { return mDeviceName; }   // Original naming
    [[nodiscard]] const String& Name() const { return !mUDevDeviceName.empty() ? mUDevDeviceName : mDeviceName; } // New naming
    [[nodiscard]] String NameExtented();
    [[nodiscard]] const Path& UDevPath() const { return mPath; }
    [[nodiscard]] String GUID() const { char sguid[64]; SDL_JoystickGetGUIDString(mDeviceGUID, sguid, sizeof(sguid)); return sguid; }
    [[nodiscard]] const SDL_JoystickGUID& RawGUID() const { return mDeviceGUID; }
    [[nodiscard]] int Identifier()   const { return mDeviceId; };
    [[nodiscard]] int Index()        const { return mDeviceIndex; };
    [[nodiscard]] int AxeCount()     const { return mDeviceNbAxes; };
    [[nodiscard]] int HatCount()     const { return mDeviceNbHats; };
    [[nodiscard]] int ButtonCount()  const { return mDeviceNbButtons; };
    [[nodiscard]] bool HasBatteryLevel() const { return !mUDevPowerPath.IsEmpty(); }
    [[nodiscard]] int BatteryLevel();
    [[nodiscard]] String::Unicode BatteryLevelIcon();

    [[nodiscard]] bool IsKeyboard() const { return mDeviceId == InputEvent::sKeyboardDevice; }
    [[nodiscard]] bool IsPad()      const { return mDeviceId != InputEvent::sKeyboardDevice && mDeviceId != InputEvent::sMouseDevice; }

    /*!
     * @brief Get recorded axis value for a particular axis
     * @param axis Axis index
     * @return Recorded value
     */
    [[nodiscard]] int PreviousAxisValues(int axis) const { return axis < sMaxAxis ? mPreviousAxisValues[axis] : 0; };

    /*!
     * @brief Get neutral axis value for a particular axis
     * @param axis Axis index
     * @return neutral value
     */
    [[nodiscard]] int NeutralAxisValue(int axis) const { return axis < sMaxAxis ? mNeutralAxisValues[axis] : 0; };

    /*!
     * @brief Check if the whole pad is in neutral position
     * @return True if the pad is in neutral position, false otherwise
     */
    [[nodiscard]] bool CheckNeutralPosition() const;

    /*!
     * @brief Record neutral position
     */
    void RecordAxisNeutralPosition();

    /*!
     * @brief Reset all InputEvent
     */
    void ClearAll();

    /*!
     * @brief Load configuration from another config objecvt
     * @param source source object to copy configuration
     */
    void LoadFrom(const InputDevice& source);

    /*
     * Non SDL2 LowLevel
     */

    //! Get low level name or return SDL2 name if it fails
    String UDevString(int index, UDevInfo info);

    #ifndef SDL_JOYSTICK_IS_OVERRIDEN_BY_RECALBOX
    /*!
     * @brief Lookup /dev/input/ path when using a non patched SDL2
     * @return
     */
    Path LookupPath();
    #endif

/*
     * Manage entries
     */

    /*!
     * @brief Check is a particular entry is set
     * @param input
     * @return
     */
    [[nodiscard]] bool IsSet(Entry input) const { return ((mConfigurationBits & (1 << (int)input)) != 0); }

    /*!
     * @brief Configure a new entry
     * @param input input entry
     * @param event event to map to input entry
     */
    void Set(Entry input, InputEvent event);

    /*!
     * @brief Remove an entry configuration
     * @param input input entry to unconfigure
     */
    void Unset(Entry input);

    /*
     * Manage the whole configuration
     */

    /*!
     * @brief Load from SDL configuration string
     * @param configuration Configuration string
     */
    bool LoadAutoConfiguration(const String& configuration);

    /*!
     * @brief Load the configuration from the given XML
     * @param root Root node to read configuration from
     * @return Number of loaded configuration entry
     */
    int LoadFromXml(pugi::xml_node root);

    /*!
     * @brief Save the configuration to the given XML
     * @param parent Root node to write the configuration to
     */
    void SaveToXml(pugi::xml_node parent) const;

    /*!
     * @brief Set the "in configuration" state on/off
     * @param state true if the Device is in configuration process
     */
    void SetConfiguringState(bool state) { mConfiguring = state; }

    /*!
     * @brief Check if at least one entry has been configured
     * @return True if at least one entry has been configured
     */
    [[nodiscard]] bool IsConfigured() const { return mConfigurationBits != 0 && !mConfiguring; }

    /*!
     * @brief Check if the given event match the entry.
     * This is the most generic matching method
     * @param entry Entry me check for matching
     * @param event Event to check for matching
     * @return True if the event match the given entry
     */
    [[nodiscard]] bool IsMatching(Entry entry, InputEvent event) const;

    /*!
     * @brief Check if the given event match at least a configured entry
     * @param event Event to check
     * @return The first matching entry
     */
    [[nodiscard]] InputDevice::Entry GetMatchedEntry(InputEvent event) const;

    /*!
     * @brief Get the configuration of a particular entry
     * @param entry Entry to get configuration
     * @param result Resulting input configuration
     * @return True if the entry was found and result contains valid data
     */
    bool GetEntryConfiguration(Entry entry, InputEvent& result) const;

    /*
     * Converters
     */

    /*!
     * @brief Convert this input event to its compact form
     * @param event source event
     * @return Compact event
     */
    InputCompactEvent ConvertToCompact(const InputEvent& event);

    /*
     * Helpers
     */

    /*!
     * @brief Convert entry to its string representation
     * @param entry entry to convert
     * @return converted string
     */
    static String EntryToString(Entry entry);

    /*!
     * @brief Convert a string entry representation to the matching entry
     * Any unknown string is converted to Entry::None
     * @param entry String entry
     * @return converted entry
     */
    static Entry StringToEntry(const String& entry);

    /*!
     * @brief Compare the current device to the given one
     * @param to compare to this device
     * @return True if name, uid, buttons, hat and axis are matching. False otherwise
     */
    [[nodiscard]] bool EqualsTo(const InputDevice& to) const;
};

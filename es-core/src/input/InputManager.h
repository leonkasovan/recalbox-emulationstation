#pragma once

#include <SDL2/SDL.h>
#include <utils/String.h>
#include <input/InputDevice.h>
#include <input/OrderedDevices.h>
#include <utils/os/fs/watching/FileSystemWatcher.h>
#include <utils/os/fs/watching/IFileSystemWatcherNotification.h>
#include <utils/os/fs/watching/FileNotifier.h>
#include "IInputChange.h"
#include <input/InputMapper.h>

class WindowManager;

class InputManager : public StaticLifeCycleControler<InputManager>
                   , public IFileSystemWatcherNotification
{
  public:
    /*!
     * @brief Default constructor
     */
    InputManager();

    /*!
     * @brief Default destructor
     */
    virtual ~InputManager() = default;

    /*!
     * @brief Initialize the InputManager
     */
    void Initialize();

    /*!
     * Finalize the input manager and free all resources
     */
    void Finalize();

    /*!
     * @brief Refresh configurations. Reload all joysticks
     * @param window Main window
     */
    void Refresh(WindowManager* window, bool padplugged);

    //! Mapper accessor
    InputMapper& Mapper() { return mMapper; }

    /*!
     * Get number of initialized devices
     */
    [[nodiscard]] int DeviceCount() const { return (int)mIdToDevices.size(); }

    /*!
     * @brief Parse an SDL event and generate an InputCompactEvent accordingly
     * @param ev SDL event
     * @param resultEvent InputCompactEvent to fill with event information
     * @return True if the resultEvent is valid, false otherwise
     */
    InputCompactEvent ManageSDLEvent(WindowManager* window, const SDL_Event& ev);

    /*!
     * @brief Get number of configured controllers, either manually or from Xml configuration file
     * @return Configured controllers count, not counting the keyboard
     */
    int ConfiguredControllersCount();

    /*!
     * @brief Get configuration path
     * @return Configuration path
     */
    static Path ConfigurationPath();

    /*!
     * @brief Write device configuration to Xml configuration file
     * @param device
     */
    static void WriteDeviceXmlConfiguration(InputDevice& device);

    /*!
     * @brief Get device by index
     * @param index Device index
     * @return Device configuration
     */
    InputDevice& GetDeviceConfigurationFromIndex(int index) { return GetDeviceConfigurationFromId(mIndexToId[index]); }

    /*!
     * @brief Get device by SDL Identifier
     * @param deviceId Device identifier
     * @return Device configuration
     */
    InputDevice& GetDeviceConfigurationFromId(SDL_JoystickID deviceId);

    /*!
     * @brief Generate an ordered device list in function of player devices configuratons
     * @return OrderedDevice object
     */
    OrderedDevices GetMappedDeviceList(const InputMapper& mapper);

    /*!
     * @brief Generate all player configurations into a single string
     * ready to be used in the configgen
     * @return Configuration string
     */
    String GetMappedDeviceListConfiguration(const InputMapper& mapper);

    /*!
     * @brief Lookup Xml configuration for a particular device, lookinf for matching
     * guid and/or name
     * @param device Device to look for configuration
     * @return
     */
    static bool LookupDeviceXmlConfiguration(InputDevice& device);

    /*!
     * @brief Log a detailled report of the raw input event
     * @param event Input event
     */
    static void LogRawEvent(const InputEvent& event);

    /*!
     * @brief Log a detailled report of the input compact event
     * @param event compact event
     */
    static void LogCompactEvent(const InputCompactEvent& event);

    /*!
     * @brief Add a new interface to call when a pad is added or removed
     * @param interface New interface
     */
    void AddNotificationInterface(IInputChange* interface);

    /*!
     * @brief Remove a notofocation interface
     * Does nothing if the given interface has not been previously added
     * @param interface Interface to remove
     */
    void RemoveNotificationInterface(IInputChange* interface);

    /*!
     * @brief Initialize SDL's joysticks
     */
    static void InitializeSDL2JoystickSystem();

    /*!
     * @brief Finalize SDL's joysticks
     */
    static void FinalizeSDL2JoystickSystem();

    /*!
     * @brief Check if there is a move in /dev/input
     */
    void WatchJoystickAddRemove(WindowManager* window);

    /*!
     * @brief Get device name by SDL Identifier
     * @param deviceId Device identifier
     * @return Device name
     */
    String GetDeviceNameFromId(SDL_JoystickID id);

    /*!
     * @brief Get device name by SDL Identifier
     * @param deviceId Device identifier
     * @return Device name
     */
    int GetDeviceIndexFromId(SDL_JoystickID id);

  private:
    //! Device list
    typedef Array<InputDevice> InputDeviceList;

    //! Index to SDL Identifiers
    SDL_JoystickID mIndexToId[Input::sMaxInputDevices];
    //! SDL Identifier to Joystick structures
    HashMap<SDL_JoystickID, SDL_Joystick*> mIdToSdlJoysticks;
    //! SDL Identifier to device configurations
    HashMap<SDL_JoystickID, InputDevice> mIdToDevices;
    //! Default Keyboard
    InputDevice mKeyboard;
    //! Default Mousse
    InputDevice mMousse;

    //! Raw key events
    bool mScancodeStates[0x100];
    //! Previous raw key events
    bool mScancodePreviousStates[0x100];

    //! Notification interfaces
    Array<IInputChange*> mNotificationInterfaces;

    //! Input mapper (must be initialized after mNotificationInterfaces)
    InputMapper mMapper;

    //! /dev/input watcher
    FileNotifier mFileNotifier;
    //! Joystick change pendings
    bool mJoystickChangePending;
    //! joystick change pending - added or removed?
    bool mJoystickChangePendingRemoved;

    /*!
     * @brief Load default keyboard configuration
     * Event if the user deletes es_input.cfg, he's still able to reconfigure using a keyboard
     */
    void LoadDefaultKeyboardConfiguration();

    /*!
     * @brief Get the initialization state
     * @return True if the manager is initialized
     */
    [[nodiscard]] bool IsInitialized() const { return !mIdToDevices.empty(); }

    /*!
     * @brief Get the GUID string of an SDL joystik
     * @param joystick SDL Joystick handle
     * @return GUID string
     */
    static String DeviceGUIDString(SDL_Joystick* joystick);

    /*!
     * @brief Clear all configurations
     */
    void ClearAllConfigurations();

    /*!
     * @brief Build current jostick list
     * @return Joystick list
     */
    std::vector<InputDevice> BuildCurrentDeviceList();

    /*!
     * @brief Remove from both lists, devices that are in both lists, keeping only unique devices in left or right lists
     * @param left Left list
     * @param right Right list
     */
    static void KeepDifferentPads(std::vector<InputDevice>& left, std::vector<InputDevice>& right);

    /*!
     * @brief Load all joystick and load configurations
     */
    void LoadAllJoysticksConfiguration(std::vector<InputDevice> previous, WindowManager* window, bool padplugged);

    /*!
     * @brief Load joystick configuration (by index)
     * @param index Joystick index from to 0 to available joysticks-1
     */
    void LoadJoystickConfiguration(int index);

    /*!
     * @brief Process an Axis SDL event and generate an InputCompactEvent accordingly
     * @param axis SDL event
     * @return InputCompactEvent filled with event information
     */
    InputCompactEvent ManageAxisEvent(const SDL_JoyAxisEvent& axis);

    /*!
     * @brief Process a button SDL event and generate an InputCompactEvent accordingly
     * @param button SDL event
     * @return InputCompactEvent filled with event information
     */
    InputCompactEvent ManageButtonEvent(const SDL_JoyButtonEvent& button);

    /*!
     * @brief Process a hat SDL event and generate an InputCompactEvent accordingly
     * @param hat SDL event
     * @return InputCompactEvent filled with event information
     */
    InputCompactEvent ManageHatEvent(const SDL_JoyHatEvent& hat);

    /*!
     * @brief Process a keyboard SDL event and generate an InputCompactEvent accordingly
     * @param keyboard SDL event
     * @return InputCompactEvent filled with event information
     */
    InputCompactEvent ManageKeyEvent(const SDL_KeyboardEvent& key, bool down);

    /*!
     * @brief Process a mouse SDL event and generate an InputCompactEvent accordingly
     * @param button SDL event
     * @return InputCompactEvent filled with event information
     */
    InputCompactEvent ManageMouseButtonEvent(const SDL_MouseButtonEvent& button, bool down);

    /*!
     * @brief Process a mouse wheel SDL event and generate an InputCompactEvent accordingly
     * @param wheel SDL event
     * @return InputCompactEvent filled with event information
     */
    InputCompactEvent ManageMouseWheelEvent(const SDL_MouseWheelEvent& wheel);

    /*
     * IFileSystemWatcherNotification implementation
     */

    /*!
     * @brief Receive file event
     * @param event Event type
     * @param path Path
     * @param time Timestamp
     */
    void FileSystemWatcherNotification(EventType event, const Path& path, const DateTime& time) override;
};

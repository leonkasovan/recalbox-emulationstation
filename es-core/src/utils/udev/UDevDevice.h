//
// Created by Bkg2k on 17/09/2023.
//
#pragma once

#include <libudev.h>
#include "utils/String.h"

// Forward declaration
class UDev;

class UDevDevice
{
public:
    enum class ActionType
    {
        added,
        removed,
        other
    };

    /*!
     * @brief Create a device from its path
     * @param context udev context
     * @param path Path ( /dev/input/event3 for example )
     */
    UDevDevice(UDev& context, const String& path);

    //! Copy constructor
    UDevDevice(const UDevDevice& source);

    //! Move constructor
    UDevDevice(UDevDevice&& source) noexcept ;

    //! Destructor
    ~UDevDevice();

    /*!
     * @brief Check if this device is valid
     * @return True if this device is valid, false otherwise
     */
    [[nodiscard]] bool IsValid() const noexcept { return mDevice != nullptr; }

    /*!
     * @brief Get parent device from the current device
     * @return Device instance (check validity with IsValid())
     */
    [[nodiscard]] UDevDevice Parent() const;
    /*!
     * @brief Find nex t parents device that match subsystem and devtype values
     * @return Device instance (check validity with IsValid())
     */
    [[nodiscard]] UDevDevice Parent(const String& subsystem, const String& devtype) const;

    /*!
     * @brief Return device's subsystem
     * @return Device's subsystem
     */
    [[nodiscard]] String Subsystem() const { return ToString(udev_device_get_subsystem(mDevice)); }
    /*!
     * @brief Return device's devtype
     * @return Device's devtype
     */
    [[nodiscard]] String Devtype() const { return ToString(udev_device_get_devtype(mDevice)); }
    /*!
     * @brief Return device's syspath
     * @return Device's syspath
     */
    [[nodiscard]] String Syspath() const { return ToString(udev_device_get_syspath(mDevice)); }
    /*!
     * @brief Return device's sysname
     * @return Device's sysname
     */
    [[nodiscard]] String Sysname() const { return ToString(udev_device_get_sysname(mDevice)); }
    /*!
     * @brief Return device's sysnum
     * @return Device's sysnum
     */
    [[nodiscard]] String Sysnum() const { return ToString(udev_device_get_sysnum(mDevice)); }
    /*!
     * @brief Return device's devnode
     * @return Device's devnode
     */
    [[nodiscard]] String Devnode() const { return ToString(udev_device_get_devnode(mDevice)); }

    /*!
     * @brief Return device's property according to the given key
     * @return Device property in the form of a String (invalid key returns empty Strings)
     */
    [[nodiscard]] String Property(const String& propertyKey) const { return ToString(udev_device_get_property_value(mDevice, propertyKey.data())); }
    /*!
     * @brief Return device's property according to the given key - Apply special cchar decoding
     * @return Device property in the form of a String (invalid key returns empty Strings)
     */
    [[nodiscard]] String PropertyDecode(const String& propertyKey) const
    {
      String s = ToString(udev_device_get_property_value(mDevice, propertyKey.data()));
      Decode(s);
      return s;
    }
    /*!
     * @brief Return device's driver
     * @return Device driver
     */
    [[nodiscard]] String Driver() const { return ToString(udev_device_get_driver(mDevice)); }

    /*!
     * @brief Return device's action linked with this device when it comes from UDevMonitor class
     * @return ActionType
     */
    [[nodiscard]] ActionType Action() const;
    /*!
     * @brief Return device's sys attr of the given name
     * @return Device sys attr (or empty string)
     */
    [[nodiscard]] String Sysattr(const String& name) const  { return ToString(udev_device_get_sysattr_value(mDevice, name.data())); }
    /*!
     * @brief Check if device has the given tag
     * @return True of the device has the given tag, false otherwise
     */
    [[nodiscard]] bool HasTag(const String& tag) const { return udev_device_has_tag(mDevice, tag.data()) != 0; }

private:
    //! udev Context
    UDev& mUDev;
    //! Device structure or nullptr
    udev_device* mDevice;

    /*!
     * @brief Direct constructor
     * @param context udev contaxt
     * @param device Device structure
     */
    UDevDevice(UDev& context, udev_device* device)
      : mUDev(context)
      , mDevice(device)
    {
    }

    // Allow Enumerate & Monitor to create direct device using the constructor above
    friend class UDevEnumerate;
    friend class UDevMonitor;

    /*!
     * @brief Convert a char* pointer to a string, taking care of null pointers
     * @param s c-string pointer
     * @return String object
     */
    static String ToString(const char* s) { return s != nullptr ? String(s) : String(); }

    /*!
     * @brief Decode udev encoded names
     * @param source Encoded source name
     */
    static void Decode(String& source);
};

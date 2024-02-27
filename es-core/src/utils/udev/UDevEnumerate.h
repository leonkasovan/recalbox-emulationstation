//
// Created by Bkg2k on 17/09/2023.
//
#pragma once

#include "utils/String.h"
#include <libudev.h>

// Forward declaration
class UDev;
class UDevDevice;

class UDevEnumerate
{
  public:
    //! Convenient type
    typedef std::vector<UDevDevice> DeviceList;

    /*!
     * @brief Constructor
     * @param reference udev Context reference
     */
    explicit UDevEnumerate(UDev& reference);

    //! Destructor
    ~UDevEnumerate();

    /*!
     * @brief Add a filter to match the given subsystem
     * @param subsystem Subsystem to match
     * @return This
     */
    UDevEnumerate& MatchSubsystem(const String& subsystem) { udev_enumerate_add_match_subsystem(mEnumeration, subsystem.data()); return *this; }

    /*!
     * @brief Add a filter to match all subsystem but the given one
     * @param subsystem Subsystem not to match
     * @return This
     */
    UDevEnumerate& NoMatchSubsystem(const String& subsystem) { udev_enumerate_add_nomatch_subsystem(mEnumeration, subsystem.data()); return *this; }

    /*!
     * @brief Add a filter to match the given sysattr value
     * @param name sysattr name
     * @param value sysattr value to match
     * @return This
     */
    UDevEnumerate& MatchSysattr(const String& name, const String& value) { udev_enumerate_add_match_sysattr(mEnumeration, name.data(), value.data()); return *this; }

    /*!
     * @brief Add a filter to match all sysattr value but the given one
     * @param name sysattr name
     * @param value sysattr value not to match
     * @return This
     */
    UDevEnumerate& NoMatchSysattr(const String& name, const String& value) { udev_enumerate_add_nomatch_sysattr(mEnumeration, name.data(), value.data()); return *this; }

    /*!
     * @brief Add a filter to match the given property value
     * @param name property name
     * @param value property value to match
     * @return This
     */
    UDevEnumerate& MatchProperty(const String& name, const String& value) { udev_enumerate_add_match_property(mEnumeration, name.data(), value.data()); return *this; }

    /*!
     * @brief Add a filter to match the given subname
     * @param subname Subname to match
     * @return This
     */
    UDevEnumerate& MatchSysname(const String& sysname) { udev_enumerate_add_match_sysname(mEnumeration, sysname.data()); return *this; }

    /*!
     * @brief Add a filter to match the given tag
     * @param tag tag to match
     * @return This
     */
    UDevEnumerate& MatchTag(const String& tag) { udev_enumerate_add_match_tag(mEnumeration, tag.data()); return *this; }

    /*!
     * @brief Add a filter to match the given parent
     * @param device Parent
     * @return This
     */
    UDevEnumerate& MatchParent(const UDevDevice& device);

    /*!
     * @brief Reset all filters
     * @return This
     */
    UDevEnumerate& ResetFilters();

    /*!
     * @brief Get device list according to filters
     * @return Device list
     */
    DeviceList List();

  private:
    //! Context reference
    UDev& mUDev;
    //! Enumeration handle
    udev_enumerate* mEnumeration;
};

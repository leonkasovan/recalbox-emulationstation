//
// Created by Bkg2k on 17/09/2023.
//
#pragma once

#include "utils/String.h"
#include "utils/datetime/TimeSpan.h"
#include <libudev.h>

// Forward declaration
class UDev;
class UDevDevice;

class UDevMonitor
{
  public:
    /*!
     * @brief Constructor
     * @param context udev context
     */
    explicit UDevMonitor(UDev& context);

    /*!
     * @brief Add a filter to match the given sybsystem & type (optionnal)
     * @param subsystem Subsystem to match
     * @param type Type to match (empty string if not required)
     * @return This
     */
    UDevMonitor& MatchDevice(const String& subsystem, const String& type) { udev_monitor_filter_add_match_subsystem_devtype(mMonitor, subsystem.data(), type.size() ? type.data() : nullptr); return *this; }

    /*!
     * @brief Add a filter to match the given tag
     * @param tag tag to match
     * @return This
     */
    UDevMonitor& MatchTag(const String& tag) { udev_monitor_filter_add_match_tag(mMonitor, tag.data()); return *this; }

    //! Check if the current monitor is active or not
    bool IsActive() const { return mFd != -1; }

    /*!
     * @brief Wait for  device indefinitely
     * @return Device
     */
    UDevDevice WaitForDevice();

    /*!
     * @brief Wait for a device for the given period of time
     * @param span Time span
     * @return Device
     */
    UDevDevice WaitForDevice(const TimeSpan& span);

    /*!
     * @brief Wait for a device for the given period of time in milliseconds
     * @param ms milliseconds
     * @return Device
     */
    UDevDevice WaitForDevice(int ms);

    /*!
     * @brief Peek a device immediately
     * @return Device
     */
    UDevDevice PeekDevice();

  private:
    UDev& mUDev;
    udev_monitor* mMonitor;
    int mFd;

    UDevDevice TryFetchDeviceFor(const TimeSpan& maxDuration);
};

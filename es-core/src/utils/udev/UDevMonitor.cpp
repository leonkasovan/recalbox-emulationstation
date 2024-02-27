//
// Created by Bkg2k on 17/09/2023.
//

#include "UDevMonitor.h"
#include <poll.h>
#include <libudev.h>
#include "UDev.h"
#include "UDevDevice.h"

UDevMonitor::UDevMonitor(UDev& context)
  : mUDev(context)
  , mMonitor(udev_monitor_new_from_netlink(mUDev.Context(), "udev::monitor"))
{
}

UDevDevice UDevMonitor::WaitForDevice()
{
  return TryFetchDeviceFor(TimeSpan(INT64_MAX));
}

UDevDevice UDevMonitor::PeekDevice()
{
  return TryFetchDeviceFor(TimeSpan(0));
}

UDevDevice UDevMonitor::WaitForDevice(const TimeSpan& span)
{
  return TryFetchDeviceFor(span);
}

UDevDevice UDevMonitor::WaitForDevice(int ms)
{
  return TryFetchDeviceFor(TimeSpan((long long)ms));
}

UDevDevice UDevMonitor::TryFetchDeviceFor(const TimeSpan& maxDuration)
{
  if (!IsActive())
  {
    if (udev_monitor_enable_receiving(mMonitor) < 0) return UDevDevice(mUDev, nullptr);
    if (mFd = udev_monitor_get_fd(mMonitor); mFd < 0)
      return UDevDevice(mUDev, nullptr);
  }

  pollfd fd{ mFd, POLLIN, 0 };

  auto count = ::poll(&fd, 1, maxDuration.Milliseconds());
  if (count == -1) return UDevDevice(mUDev, nullptr);

  return (fd.events & fd.revents) != 0 ? UDevDevice(mUDev, udev_monitor_receive_device(mMonitor)) : UDevDevice(mUDev, nullptr);
}

//
// Created by bkg2k on 27/03/2022.
//
// As part of the RECALBOX Project
// http://www.recalbox.com
//

#include "RecalboxStorageWatcher.h"
#include "utils/Sizes.h"
#include <guis/GuiInfoPopup.h>
#include <utils/os/fs/Path.h>
#include <hardware/devices/mount/MountMonitor.h>
#include <WindowManager.h>
#include <utils/locale/LocaleHelper.h>

void RecalboxStorageWatcher::CheckStorageFreeSpace(WindowManager& window, MountMonitor& mountMonitor, const Path& path)
{
  const DeviceMount* device = mountMonitor.SizeOf(path);
  if (device != nullptr)
    if (device->TotalSize() != 0)
    {
      long long limit = (device->MountPoint() == MountMonitor::sSharePath) ? sShareRequiredMinimumFreeSize : sStorageRequiredMinimumFreeSize;
      if (device->FreeSize() < limit)
      {
        String text = _("Free space on device %NAME% has bone under %LIMIT%!\nYou should try to free some space quickly!");
        text.Replace("%NAME%", device->DisplayableDeviceName())
            .Replace("%LIMIT%", Sizes(limit).ToHumanSize());
        window.InfoPopupAdd(new GuiInfoPopup(window, text, 30, PopupType::Warning));
      }
    }
}

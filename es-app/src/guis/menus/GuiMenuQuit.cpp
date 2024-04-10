//
// Created by bkg2k on 12/02/2020.
//

#include <guis/menus/GuiMenuQuit.h>
#include <guis/GuiMsgBox.h>
#include <utils/locale/LocaleHelper.h>
#include <MainRunner.h>
#include <hardware/Case.h>
#include "utils/Files.h"

GuiMenuQuit::GuiMenuQuit(WindowManager& window)
  : GuiMenuBase(window, _("QUIT"), this)
{
  if (Case::CurrentCase().CanShutdownFromMenu())
  {
    // Shutdown
    AddSubMenu(_("SHUTDOWN SYSTEM"), (int)Components::Shutdown);

    // Fast Shutdown
    AddSubMenu(_("FAST SHUTDOWN SYSTEM"), (int) Components::FastShutdown);
  }

  // Reboot
  AddSubMenu(_("RESTART SYSTEM"), (int)Components::Reboot);

  // Quit ES
  AddSubMenu(_("QUIT GUI FRONTEND"), (int)Components::QuitGui);

  int usb_no = 0;
  for(const String& line : Files::LoadFile(Path("/proc/mounts")).Split('\n')) // For every entry
    if (!line.empty()) {
      String::List items = line.Split(' ');
      const String& mountPoint = items[1];

      if (mountPoint.StartsWith("/recalbox/share/externals")){
        const char *usb;

        usb = strrchr(mountPoint.c_str(), '/');
        if (usb){
          usb++;
          usb_no++;
          mMountPoints.push_back(mountPoint);
          AddSubMenu("EJECT " + String(usb).UpperCase(), (int)Components::QuitGui + usb_no);
        }
      }
    }
}

void GuiMenuQuit::PushQuitGui(WindowManager& window)
{
  window.pushGui(new GuiMenuQuit(window));
}

void GuiMenuQuit::SubMenuSelected(int id)
{
  switch((Components)id)
  {
    case Components::Shutdown:
    {
      mWindow.pushGui(new GuiMsgBox(mWindow, _("REALLY SHUTDOWN?"), _("YES"),
                                        [] { MainRunner::RequestQuit(MainRunner::ExitState::Shutdown); }, _("NO"), nullptr));
      break;
    }
    case Components::FastShutdown:
    {
      mWindow.pushGui(new GuiMsgBox(mWindow, _("REALLY SHUTDOWN WITHOUT SAVING METADATAS?"), _("YES"),
                                        [] { MainRunner::RequestQuit(MainRunner::ExitState::FastShutdown); }, _("NO"), nullptr));
      break;
    }
    case Components::Reboot:
    {
      mWindow.pushGui(new GuiMsgBox(mWindow, _("REALLY RESTART?"), _("YES"),
                                        [] { MainRunner::RequestQuit(MainRunner::ExitState::NormalReboot); }, _("NO"), nullptr));
      break;
    }
    case Components::QuitGui:
    {
      mWindow.pushGui(new GuiMsgBox(mWindow, _("REALLY QUIT GUI?"), _("YES"),
                                        [] { MainRunner::RequestQuit(MainRunner::ExitState::Quit); }, _("NO"), nullptr));
      break;
    }
  }

  if (id > (int)Components::QuitGui){
    String umount_cmd = "umount ";

    umount_cmd.Append(mMountPoints[id - (int)Components::QuitGui - 1]);
    if (system(umount_cmd.c_str()) != 0){
      mWindow.InfoPopupAdd(new GuiInfoPopup(mWindow, "[ERROR] USB can't be ejected", 2, PopupType::Warning));
      { LOG(LogError) << "[GuiMenuQuit] Can not umount " << mMountPoints[id - (int)Components::QuitGui - 1]; }
    }else{
      mWindow.InfoPopupAdd(new GuiInfoPopup(mWindow, "USB successfully ejected", 2, PopupType::Recalbox));
      { LOG(LogInfo) << "[GuiMenuQuit] Success umount " << mMountPoints[id - (int)Components::QuitGui - 1]; }
    }
    Close();
  }
}

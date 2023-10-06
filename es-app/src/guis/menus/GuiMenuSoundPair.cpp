//
// Created by davidb2111 on 31/08/2022.
//
// As part of the RECALBOX Project
// http://www.recalbox.com
//

#include <guis/menus/GuiMenuSoundPair.h>
#include <recalbox/RecalboxSystem.h>
#include <guis/GuiMsgBox.h>
#include <utils/locale/LocaleHelper.h>

GuiMenuSoundPair::GuiMenuSoundPair(WindowManager& window, const String::List& deviceList)
  : GuiMenuBase(window, _("PAIR A BLUETOOTH AUDIO DEVICE"), this)
  , mDevices(deviceList)
{
  int index = -1;
  for (const auto & controllerString : mDevices)
    AddSubMenu(controllerString, ++index);
}

bool GuiMenuSoundPair::Execute(GuiWaitLongExecution<String, bool>& from, const String& parameter)
{
  (void)from;
  return RecalboxSystem::pairBluetooth(parameter);
}

void GuiMenuSoundPair::Completed(const String& parameter, const bool& result)
{
  (void)parameter;
  mWindow.pushGui(new GuiMsgBox(mWindow, result ? _("AUDIO DEVICE PAIRED") : _("UNABLE TO PAIR AUDIO DEVICE"), _("OK")));
}

void GuiMenuSoundPair::SubMenuSelected(int id)
{
  String device = mDevices[id];
  String text = _("PAIRING %s ...").Replace("%s", device);
  mWindow.pushGui((new GuiWaitLongExecution<String, bool>(mWindow, *this))->Execute(device, text));
}


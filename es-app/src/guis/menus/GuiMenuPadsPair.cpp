//
// Created by bkg2k on 13/01/2021.
//
// As part of the RECALBOX Project
// http://www.recalbox.com
//

#include <guis/menus/GuiMenuPadsPair.h>
#include <recalbox/RecalboxSystem.h>
#include <guis/GuiMsgBox.h>
#include <utils/locale/LocaleHelper.h>

GuiMenuPadsPair::GuiMenuPadsPair(WindowManager& window, const String::List& deviceList)
  : GuiMenuBase(window, _("PAIR BLUETOOTH CONTROLLERS"), this)
  , mDevices(deviceList)
{
  int index = -1;
  for (const auto & controllerString : mDevices)
    AddSubMenu(controllerString, ++index);
}

bool GuiMenuPadsPair::Execute(GuiWaitLongExecution<String, bool>& from, const String& parameter)
{
  (void)from;
  return RecalboxSystem::pairBluetooth(parameter);
}

void GuiMenuPadsPair::Completed(const String& parameter, const bool& result)
{
  (void)parameter;
  mWindow.pushGui(new GuiMsgBox(mWindow, result ? _("CONTROLLER PAIRED") : _("UNABLE TO PAIR CONTROLLER"), _("OK")));
}

void GuiMenuPadsPair::SubMenuSelected(int id)
{
  String device = mDevices[id];
  String text = _("PAIRING %s ...").Replace("%s", device);
  mWindow.pushGui((new GuiWaitLongExecution<String, bool>(mWindow, *this))->Execute(device, text));
}


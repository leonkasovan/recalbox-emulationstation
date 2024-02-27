//
// Created by bkg2k on 12/01/2021.
//
// As part of the RECALBOX Project
// http://www.recalbox.com
//

#include "GuiMenuPads.h"
#include "GuiMenuPadsPair.h"
#include "guis/GuiBluetoothDevices.h"
#include <guis/MenuMessages.h>
#include <guis/GuiMsgBox.h>
#include <recalbox/RecalboxSystem.h>
#include <guis/GuiDetectDevice.h>

GuiMenuPads::GuiMenuPads(WindowManager& window)
  : GuiMenuBase(window, _("CONTROLLERS SETTINGS"), this)
  , mMapper(InputManager::Instance().Mapper())
  , mRefreshing(false)
{
  // Configure a pad
  AddSubMenu(_("CONFIGURE A CONTROLLER"), (int)Components::Configure, _(MENUMESSAGE_CONTROLLER_CONF_HELP_MSG));

  // Pair a pad
  AddSubMenu(_("PAIR BLUETOOTH CONTROLLERS"), (int)Components::Pair, _(MENUMESSAGE_CONTROLLER_BT_HELP_MSG));

  // Unpair all pads
  AddSubMenu(_("FORGET BLUETOOTH CONTROLLERS"), (int)Components::Unpair, _(MENUMESSAGE_CONTROLLER_FORGET_HELP_MSG));

  // Automatic pairing on boot
  AddSwitch(_("AUTO PAIR ON BOOT"), RecalboxConf::Instance().GetAutoPairOnBoot(), (int)Components::AutoPairOnBoot, this, _(MENUMESSAGE_CONTROLLER_AUTOPAIRONBOOT_HELP_MSG));

  // Driver
  AddList<String>(_("DRIVER"), (int)Components::Driver, this, GetModes(), _(MENUMESSAGE_CONTROLLER_DRIVER_HELP_MSG));

  // Pad OSD always on
  AddSwitch(_("ALWAYS SHOW PAD OSD"), RecalboxConf::Instance().GetPadOSD(), (int)Components::PadOSD, this, _(MENUMESSAGE_CONTROLLER_PADOSD_HELP_MSG));

  // Pad OSD type
  AddList<RecalboxConf::PadOSDType>(_("PAD OSD TYPE"), (int)Components::PadOSDType, this, GetPadOSDType(), _(MENUMESSAGE_CONTROLLER_PADOSDTYPE_HELP_MSG));

  // Pad list
  for(int i = 0; i < Input::sMaxInputDevices; ++i)
  {
    static const char* balls[10] ={ "\u2776", "\u2777", "\u2778", "\u2779", "\u277a", "\u277b", "\u277c", "\u277d", "\u277e", "\u277f" };
    String name(balls[i]);
    name.Append(' ').Append(_("INPUT P%i")).Replace("%i", String(i + 1));
    mDevices[i] = AddList<int>(name, (int)Components::Pads + i, this);
  }

  // Process & refresh device selector components
  RefreshDevices();

  // Subscribe refresh event
  InputManager::Instance().AddNotificationInterface(this);

  // Force OSD when thius menu is on
  mWindow.OSD().GetPadOSD().ForcedPadOSDActivation(true);
}

GuiMenuPads::~GuiMenuPads()
{
  InputManager::Instance().RemoveNotificationInterface(this);
  mWindow.OSD().GetPadOSD().ForcedPadOSDActivation(false);
}

std::vector<GuiMenuBase::ListEntry<String>> GuiMenuPads::GetModes()
{
  std::vector<ListEntry<String>> list;

  String mode = RecalboxConf::Instance().GetGlobalInputDriver();
  if ((mode != "udev") && (mode != "sdl2")) mode = "auto";

  list.push_back({ _("AUTOMATIC"), "auto", mode == "auto" });
  list.push_back({ _("SYSTEM DRIVER"), "udev", mode == "udev" });
  list.push_back({ _("GAME LIBRARY DRIVER"), "sdl2", mode == "sdl2" });

  return list;
}

void GuiMenuPads::PadsAddedOrRemoved(bool removed)
{
  (void)removed;
  RefreshDevices();
}

void GuiMenuPads::StartConfiguring()
{
  Gui* msgBox = new GuiMsgBox(mWindow,
                              _("YOU ARE GOING TO CONFIGURE A CONTROLLER. IF YOU HAVE ONLY ONE JOYSTICK, "
                                "CONFIGURE THE DIRECTIONS KEYS AND SKIP JOYSTICK CONFIG BY HOLDING A BUTTON. "
                                "IF YOU DO NOT HAVE A SPECIAL KEY FOR HOTKEY, CHOOSE THE SELECT BUTTON. SKIP "
                                "ALL BUTTONS YOU DO NOT HAVE BY HOLDING A KEY. BUTTONS NAMES ARE BASED ON THE "
                                "SNES CONTROLLER."), _("OK"), std::bind(GuiMenuPads::RunDeviceDetection, this));
  mWindow.pushGui(msgBox);
}

void GuiMenuPads::RunDeviceDetection(GuiMenuPads* thiz)
{
  thiz->mWindow.pushGui(new GuiDetectDevice(thiz->mWindow, false, [thiz] { thiz->RefreshDevices(); }));
}

String::List GuiMenuPads::Execute(GuiWaitLongExecution<bool, String::List>& from, const bool& parameter)
{
  (void)from;
  (void)parameter;
  return RecalboxSystem::scanBluetooth();
}

void GuiMenuPads::Completed(const bool& parameter, const String::List& result)
{
  (void)parameter;
  mWindow.pushGui(result.empty()
                  ? (Gui*)new GuiMsgBox(mWindow, _("NO CONTROLLERS FOUND"), _("OK"))
                  : (Gui*)new GuiMenuPadsPair(mWindow, result));
}

void GuiMenuPads::StartScanningDevices()
{
  BTAutopairManager::Instance().StartDiscovery();
  mWindow.pushGui(new GuiBluetoothDevices(mWindow));
}

void GuiMenuPads::UnpairAll()
{
  RecalboxSystem::forgetBluetoothControllers();
  mWindow.pushGui(new GuiMsgBox(mWindow, _("CONTROLLERS LINKS HAVE BEEN DELETED."), _("OK")));
}

void GuiMenuPads::RefreshDevices()
{
  mRefreshing = true;
  // Finaly fill in all components
  InputMapper::PadList list = mMapper.GetPads();
  for(int i = 0; i < (int)list.size(); ++i)
  {
    mDevices[i]->clear();
    for(int j = 0; j < (int)list.size(); ++j)
      if (const InputMapper::Pad& displayablePad = list[j]; displayablePad.IsConnected())
        mDevices[i]->add(mMapper.GetDecoratedName(displayablePad.mPosition), j, i == j);
  }
  for(int i = Input::sMaxInputDevices; --i >= (int)list.size();)
  {
    mDevices[i]->clear();
    mDevices[i]->add(_("NONE"), -1, true);
  }
  mRefreshing = false;
}

void GuiMenuPads::SubMenuSelected(int id)
{
  switch((Components)id)
  {
    case Components::Configure: StartConfiguring(); break;
    case Components::Pair:
    {
      Gui* msgBox = new GuiMsgBox(mWindow,
                                  _("The bluetooth pairing will start and run for several minutes.\n"
                                  "During this time, you just have to apply the pairing procedure on any bluetooth controller you want to pair.\n"
                                  "The next window will display all detected bluetooth devices and their status for information purposes only.\n"
                                  "You can close it at any time, while continuing to pair your bluetooth devices for as long as the bluetooth icon is blinking on the top left."),
                                  _("OK"), [this] { StartScanningDevices(); });
      mWindow.pushGui(msgBox);
      break;
    }
    case Components::Unpair: UnpairAll(); break;
    case Components::Pads:
    case Components::Driver:
    case Components::PadOSD:
    case Components::PadOSDType:
    case Components::AutoPairOnBoot:
    default: break;
  }
}

void GuiMenuPads::OptionListComponentChanged(int id, int index, const int& value)
{
  (void)index;
  (void)value;
  if (mRefreshing) return;

  int newIndex = mDevices[id]->getSelected();
  if (newIndex < 0) return; // Ignore user playing with NONE :)

  // Get positions
  InputMapper::PadList list = mMapper.GetPads();
  int position1 = list[id].mPosition;
  int position2 = list[newIndex].mPosition;

  // Swap both pads
  mMapper.Swap(position1, position2);
  RefreshDevices();
}

void GuiMenuPads::OptionListComponentChanged(int id, int index, const String& value)
{
  (void)index;
  if ((Components)id == Components::Driver)
    RecalboxConf::Instance().SetGlobalInputDriver(value).Save();
}

void GuiMenuPads::SwitchComponentChanged(int id, bool& status)
{
  if ((Components)id == Components::PadOSD)
    RecalboxConf::Instance().SetPadOSD(status).Save();
  if ((Components)id == Components::AutoPairOnBoot)
    RecalboxConf::Instance().SetAutoPairOnBoot(status).Save();
}

void GuiMenuPads::OptionListComponentChanged(int id, int index, const RecalboxConf::PadOSDType& value)
{
  (void)index;
  if ((Components)id == Components::PadOSDType)
  {
    RecalboxConf::Instance().SetPadOSDType(value).Save();
    mWindow.OSD().GetPadOSD().UpdatePadIcon();
  }
}

std::vector<GuiMenuBase::ListEntry<RecalboxConf::PadOSDType>> GuiMenuPads::GetPadOSDType()
{
  RecalboxConf::PadOSDType selected = RecalboxConf::Instance().GetPadOSDType();
  return std::vector<GuiMenuBase::ListEntry<RecalboxConf::PadOSDType>>
  {
    { "Nintendo SNES", RecalboxConf::PadOSDType::Snes, selected == RecalboxConf::PadOSDType::Snes },
    { "Nintendo N64", RecalboxConf::PadOSDType::N64, selected == RecalboxConf::PadOSDType::N64 },
    { "Sega Megadrive", RecalboxConf::PadOSDType::MD, selected == RecalboxConf::PadOSDType::MD },
    { "Sega Dreamcast", RecalboxConf::PadOSDType::DC, selected == RecalboxConf::PadOSDType::DC },
    { "Sony Playstation", RecalboxConf::PadOSDType::PSX, selected == RecalboxConf::PadOSDType::PSX },
    { "Microsoft XBox", RecalboxConf::PadOSDType::XBox, selected == RecalboxConf::PadOSDType::XBox },
  };
}

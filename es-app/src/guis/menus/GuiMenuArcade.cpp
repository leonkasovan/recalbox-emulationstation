//
// Created by bkg2k on 15/06/23.
//

#include "GuiMenuArcade.h"
#include "GuiMenuArcadeAllInOneSystem.h"
#include <views/ViewController.h>
#include <guis/MenuMessages.h>
#include <systems/arcade/ArcadeVirtualSystems.h>

GuiMenuArcade::GuiMenuArcade(WindowManager& window, IArcadeGamelistInterface* arcadeInterface)
  :	GuiMenuBase(window, _("ARCADE VIEW OPTIONS"), this)
  , mArcade(arcadeInterface)
{
  AddSwitch(_("ENABLE ENHANCED VIEW"), RecalboxConf::Instance().GetArcadeViewEnhanced(), (int)Components::EnhancedView, this, _(MENUMESSAGE_UI_ARCADE_ENHANCED_MSG));

  AddSwitch(_("FOLD CLONES BY DEFAULT"), RecalboxConf::Instance().GetArcadeViewFoldClones(), (int)Components::FoldClones, this, _(MENUMESSAGE_UI_ARCADE_HIDE_CLONES_MSG));

  AddSwitch(_("HIDE BIOS"), RecalboxConf::Instance().GetArcadeViewHideBios(), (int)Components::HideBios, this, _(MENUMESSAGE_UI_ARCADE_HIDE_BIOS_MSG));

  AddSwitch(_("HIDE NON-WORKING GAMES"), RecalboxConf::Instance().GetArcadeViewHideNonWorking(), (int)Components::HideNonWorking, this, _(MENUMESSAGE_UI_ARCADE_HIDE_NONWORKING_MSG));

  AddSwitch(_("ALWAYS USE OFFICIAL NAMES"), RecalboxConf::Instance().GetArcadeUseDatabaseNames(), (int)Components::UseDatabasesNames, this, _(MENUMESSAGE_UI_ARCADE_OFFICIAL_NAMES_MSG));

  // Global menu if no arcade interface has been provided
  if (mArcade == nullptr)
  {
    // Per manufacturer systems
    AddMultiList<String>(_("MANUFACTURER VIRTUAL SYSTEMS"), (int)Components::ManufacturersVirtual, this, GetManufacturersVirtualEntries());

    // All-in-one arcade system options
    AddSubMenu(_("ARCADE ALL-IN-ONE SYSTEM"), (int)Components::GlobalArcadeSystem);
  }
  else // Gamelist menu
  {
    if (mArcade->HasValidDatabase())
      AddMultiList<int>((_F(_("HIDE {0} MANUFACTURERS")) / mArcade->GetCurrentCoreName())(), (int) Components::ManufacturersFilter, this, GetManufacturerFilterEntries(), _(MENUMESSAGE_GAMELISTOPTION_MANUFACTURERS_MSG));
  }
}

GuiMenuArcade::~GuiMenuArcade()
{
  // Force refreshing all gamelists
  ViewController::Instance().InvalidateAllGamelistsExcept(nullptr);
}

void GuiMenuArcade::SwitchComponentChanged(int id, bool status)
{
  switch((Components)id)
  {
    case Components::EnhancedView: RecalboxConf::Instance().SetArcadeViewEnhanced(status).Save(); break;
    case Components::FoldClones: RecalboxConf::Instance().SetArcadeViewFoldClones(status).Save(); break;
    case Components::HideBios: RecalboxConf::Instance().SetArcadeViewHideBios(status).Save(); break;
    case Components::HideNonWorking: RecalboxConf::Instance().SetArcadeViewHideNonWorking(status).Save(); break;
    case Components::UseDatabasesNames: RecalboxConf::Instance().SetArcadeUseDatabaseNames(status).Save(); break;
    case Components::ManufacturersVirtual:
    case Components::ManufacturersFilter:
    case Components::GlobalArcadeSystem:
    default: break;
  }
}

std::vector<GuiMenuBase::ListEntry<String>> GuiMenuArcade::GetManufacturersVirtualEntries()
{
  std::vector<GuiMenuBase::ListEntry<String>> result;
  const RecalboxConf& conf = RecalboxConf::Instance();

  for(const String& rawIdentifier : ArcadeVirtualSystems::GetVirtualArcadeSystemList())
  {
    String identifier(rawIdentifier);
    identifier.Replace('/', '-');
    result.push_back({ ArcadeVirtualSystems::GetRealName(rawIdentifier), identifier, conf.IsInCollectionArcadeManufacturers(identifier) });
  }
  return result;
}

void GuiMenuArcade::OptionListMultiComponentChanged(int id, const String::List& value)
{
  if ((Components)id == Components::ManufacturersVirtual)
    RecalboxConf::Instance().SetCollectionArcadeManufacturers(value).Save();
}

void GuiMenuArcade::OptionListMultiComponentChanged(int id, const std::vector<int>& value)
{
  if ((Components)id == Components::ManufacturersFilter)
  {
    std::vector<ArcadeDatabase::Driver> driverList = mArcade->GetDriverList();
    String::List driverNameList;
    for(int driverIndex : value)
      driverNameList.push_back(driverList[driverIndex].Name.empty() ? ArcadeVirtualSystems::sAllOtherDriver : driverList[driverIndex].Name);
    RecalboxConf::Instance().SetArcadeSystemHiddenDrivers(mArcade->GetAttachedSystem(), driverNameList);
  }
}

std::vector<GuiMenuBase::ListEntry<int>> GuiMenuArcade::GetManufacturerFilterEntries()
{
  std::vector<ArcadeDatabase::Driver> driverList = mArcade->GetDriverList();
  std::vector<GuiMenuBase::ListEntry<int>> result;
  for(const ArcadeDatabase::Driver& driver : driverList)
    result.push_back({ FormatManufacturer(driver), driver.Index, RecalboxConf::Instance().IsInArcadeSystemHiddenDrivers(mArcade->GetAttachedSystem(), driver.Name) });
  return result;
}

String GuiMenuArcade::FormatManufacturer(const ArcadeDatabase::Driver& driver)
{
  String newName = ArcadeVirtualSystems::GetRealName(driver.Name);
  if (driver.Name.empty()) newName = _("ALL OTHERS");
  if (newName.Contains('/')) newName.Replace('/', " - ");
  int count = mArcade->GetGameCountForDriver(driver.Index);
  newName.Append(" (").Append(count != 0 ? (_F(_N("{0} GAME", "{0} GAMES", count)) / count)() : "").Append(')');
  return newName;
}

void GuiMenuArcade::SubMenuSelected(int id)
{
  if ((Components)id == Components::GlobalArcadeSystem)
    mWindow.pushGui(new GuiMenuArcadeAllInOneSystem(mWindow));
}

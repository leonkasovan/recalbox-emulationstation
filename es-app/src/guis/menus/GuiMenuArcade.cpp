//
// Created by bkg2k on 15/06/23.
//

#include "GuiMenuArcade.h"
#include "GuiMenuArcadeAllInOneSystem.h"
#include <views/ViewController.h>
#include <guis/MenuMessages.h>
#include <systems/arcade/ArcadeVirtualSystems.h>

GuiMenuArcade::GuiMenuArcade(WindowManager& window, SystemManager& systemManager, IArcadeGamelistInterface* arcadeInterface)
  :	GuiMenuBase(window, _("ARCADE SETTINGS"), this)
  , mSystemManager(systemManager)
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
    //if (mArcade->HasValidDatabase())
    //  AddMultiList<int>((_F(_("HIDE {0} MANUFACTURERS")) / mArcade->GetCurrentCoreName())(), (int) Components::ManufacturersFilter, this, GetManufacturerFilterEntries(), _(MENUMESSAGE_GAMELISTOPTION_MANUFACTURERS_MSG));
  }
}

void GuiMenuArcade::SwitchComponentChanged(int id, bool& status)
{
  switch((Components)id)
  {
    case Components::EnhancedView:
    {
      RecalboxConf::Instance().SetArcadeViewEnhanced(status).Save();
      ViewController::Instance().InvalidateAllGamelistsExcept(nullptr);
      break;
    }
    case Components::FoldClones:
    {
      RecalboxConf::Instance().SetArcadeViewFoldClones(status).Save();
      ViewController::Instance().InvalidateAllGamelistsExcept(nullptr);
      break;
    }
    case Components::HideBios:
    {
      RecalboxConf::Instance().SetArcadeViewHideBios(status);
      if (mSystemManager.UpdatedTopLevelFilter())
      {
        ViewController::Instance().InvalidateAllGamelistsExcept(nullptr);
        RecalboxConf::Instance().Save();
      }
      else
      {
        mWindow.displayMessage(_("There is no game to show after this filter is changed! No change recorded."));
        RecalboxConf::Instance().SetArcadeViewHideBios(!status);
        status = !status;
      }
      break;
    }
    case Components::HideNonWorking:
    {
      RecalboxConf::Instance().SetArcadeViewHideNonWorking(status);
      if (mSystemManager.UpdatedTopLevelFilter())
      {
        ViewController::Instance().InvalidateAllGamelistsExcept(nullptr);
        RecalboxConf::Instance().Save();
      }
      else
      {
        mWindow.displayMessage(_("There is no game to show after this filter is changed! No change recorded."));
        RecalboxConf::Instance().SetArcadeViewHideNonWorking(!status);
        status = !status;
      }
      break;
    }
    case Components::UseDatabasesNames:
    {
      RecalboxConf::Instance().SetArcadeUseDatabaseNames(status).Save();
      ViewController::Instance().InvalidateAllGamelistsExcept(nullptr);
      break;
    }
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

  for(const std::pair<String, String>& rawIdentifier : ArcadeVirtualSystems::GetVirtualArcadeSystemListExtended())
  {
    String identifier(SystemManager::sArcadeManufacturerPrefix);
    identifier.Append(rawIdentifier.first).Replace('\\', '-');
    mManufacturersIdentifiers.push_back(identifier);
    result.push_back({ /*String(rawIdentifier).Replace('\\', " - ")*/rawIdentifier.second, identifier, conf.IsInCollectionArcadeManufacturers(identifier) });
  }
  return result;
}

void GuiMenuArcade::OptionListMultiComponentChanged(int id, const String::List& value)
{
  if ((Components)id == Components::ManufacturersVirtual)
  {
    // Save
    RecalboxConf::Instance().SetCollectionArcadeManufacturers(value).Save();
    // Refresh all systems
    for(const String& manufacturer : mManufacturersIdentifiers)
      mSystemManager.UpdateVirtualArcadeManufacturerSystemsVisibility(manufacturer,
                                                                      RecalboxConf::Instance().IsInCollectionArcadeManufacturers(manufacturer) ?
                                                                      SystemManager::Visibility::ShowAndSelect :
                                                                      SystemManager::Visibility::Hide);
  }
}

void GuiMenuArcade::OptionListMultiComponentChanged(int id, const std::vector<int>& value)
{
  if ((Components)id == Components::ManufacturersFilter)
  {
    std::vector<ArcadeDatabase::Manufacturer> manufacturerList = mArcade->GetManufacturerList();
    String::List manufacturerNameList;
    for(int manufacturerIndex : value)
      manufacturerNameList.push_back(manufacturerList[manufacturerIndex].Name.empty() ? ArcadeVirtualSystems::sAllOtherManufacturers : manufacturerList[manufacturerIndex].Name);
    RecalboxConf::Instance().SetArcadeSystemHiddenManufacturers(mArcade->GetAttachedSystem(), manufacturerNameList);
  }
}

std::vector<GuiMenuBase::ListEntry<int>> GuiMenuArcade::GetManufacturerFilterEntries()
{
  std::vector<ArcadeDatabase::Manufacturer> manufacturerList = mArcade->GetManufacturerList();
  std::vector<GuiMenuBase::ListEntry<int>> result;
  for(const ArcadeDatabase::Manufacturer& manufacturer : manufacturerList)
    result.push_back({FormatManufacturer(manufacturer), manufacturer.Index, RecalboxConf::Instance().IsInArcadeSystemHiddenManufacturers(mArcade->GetAttachedSystem(), manufacturer.Name) });
  return result;
}

String GuiMenuArcade::FormatManufacturer(const ArcadeDatabase::Manufacturer& manufacturer)
{
  String newName = manufacturer.Name;
  if (newName.empty()) newName = _("ALL OTHERS");
  if (newName.Contains('\\')) newName.Replace('\\', " - ");
  int count = mArcade->GetGameCountForManufacturer(manufacturer.Index);
  newName.Append(" (").Append(count != 0 ? (_F(_N("{0} GAME", "{0} GAMES", count)) / count)() : "").Append(')');
  return newName;
}

void GuiMenuArcade::SubMenuSelected(int id)
{
  if ((Components)id == Components::GlobalArcadeSystem)
    mWindow.pushGui(new GuiMenuArcadeAllInOneSystem(mWindow, mSystemManager));
}

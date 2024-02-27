#include <RecalboxConf.h>
#include <utils/locale/LocaleHelper.h>
#include <systems/SystemManager.h>
#include <guis/GuiMsgBox.h>
#include "GuiMenuArcadeAllInOneSystem.h"
#include "components/SwitchComponent.h"

GuiMenuArcadeAllInOneSystem::GuiMenuArcadeAllInOneSystem(WindowManager& window, SystemManager& systemManager)
  : GuiMenuBase(window, _("ARCADE VIRTUAL SYSTEM"), nullptr)
  , mSystemManager(systemManager)
{
  // Enable arcade
  AddSwitch(_("ENABLE ARCADE VIRTUAL SYSTEM"), RecalboxConf::Instance().GetCollectionArcade(), (int)Components::ArcadeOnOff, this);

  // Include neogeo?
  AddSwitch(_("INCLUDE NEO-GEO"), RecalboxConf::Instance().GetCollectionArcadeNeogeo(), (int)Components::IncludeNeogeo, this);

  // Hide original systems?
  AddSwitch(_("HIDE ORIGINAL SYSTEMS"), RecalboxConf::Instance().GetCollectionArcadeHideOriginals(), (int)Components::HideOriginals, this);
}

void GuiMenuArcadeAllInOneSystem::SwitchComponentChanged(int id, bool& status)
{
  switch((Components)id)
  {
    case Components::ArcadeOnOff:
    {
      RecalboxConf::Instance().SetCollectionArcade(status).Save();
      mSystemManager.UpdateSystemsVisibility(mSystemManager.VirtualSystemByType(VirtualSystemType::Arcade),
                                             status ? SystemManager::Visibility::ShowAndSelect : SystemManager::Visibility::Hide);
      mSystemManager.ManageArcadeVirtualSystem();
      break;
    }
    case Components::IncludeNeogeo:
    {
      RecalboxConf::Instance().SetCollectionArcadeNeogeo(status).Save();
      mSystemManager.ManageArcadeVirtualSystem();
      break;
    }
    case Components::HideOriginals:
    {
      RecalboxConf::Instance().SetCollectionArcadeHideOriginals(status).Save();
      mSystemManager.ManageArcadeVirtualSystem();
      break;
    }
    default: break;
  }
}


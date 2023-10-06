#include <RecalboxConf.h>
#include <utils/locale/LocaleHelper.h>
#include <systems/SystemManager.h>
#include <guis/GuiMsgBox.h>
#include "GuiMenuArcadeAllInOneSystem.h"
#include "components/SwitchComponent.h"

GuiMenuArcadeAllInOneSystem::GuiMenuArcadeAllInOneSystem(WindowManager& window)
  : GuiMenuBase(window, _("ARCADE VIRTUAL SYSTEM"), nullptr)
  , mOriginalArcadeOnOff(RecalboxConf::Instance().GetCollectionArcade())
  , mOriginalIncludeNeogeo(RecalboxConf::Instance().GetCollectionArcadeNeogeo())
  , mOriginalHideOriginals(RecalboxConf::Instance().GetCollectionArcadeHideOriginals())
{
  // Enable arcade
  AddSwitch(_("ENABLE ARCADE VIRTUAL SYSTEM"), mOriginalArcadeOnOff, (int)Components::ArcadeOnOff, this);

  // Include neogeo?
  AddSwitch(_("INCLUDE NEO-GEO"), mOriginalIncludeNeogeo, (int)Components::IncludeNeogeo, this);

  // Hide original systems?
  AddSwitch(_("HIDE ORIGINAL SYSTEMS"), mOriginalHideOriginals, (int)Components::HideOriginals, this);
}

GuiMenuArcadeAllInOneSystem::~GuiMenuArcadeAllInOneSystem()
{
  const RecalboxConf& conf = RecalboxConf::Instance();
  if ((conf.GetCollectionArcade() != mOriginalArcadeOnOff) ||
      (conf.GetCollectionArcadeNeogeo() != mOriginalIncludeNeogeo) ||
      (conf.GetCollectionArcadeHideOriginals() != mOriginalHideOriginals))
    RequestRelaunch();
}

void GuiMenuArcadeAllInOneSystem::SwitchComponentChanged(int id, bool status)
{
  switch((Components)id)
  {
    case Components::ArcadeOnOff: RecalboxConf::Instance().SetCollectionArcade(status).Save(); break;
    case Components::IncludeNeogeo: RecalboxConf::Instance().SetCollectionArcadeNeogeo(status).Save(); break;
    case Components::HideOriginals: RecalboxConf::Instance().SetCollectionArcadeHideOriginals(status).Save(); break;
    default: break;
  }
}


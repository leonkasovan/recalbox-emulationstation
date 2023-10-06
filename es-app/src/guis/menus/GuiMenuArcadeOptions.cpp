//
// Created by bkg2k on 29/05/23.
//

#include "GuiMenuArcadeOptions.h"
#include <views/ViewController.h>
#include "guis/MenuMessages.h"

GuiMenuArcadeOptions::GuiMenuArcadeOptions(WindowManager& window)
  :	GuiMenuBase(window, _("ARCADE VIEW OPTIONS"), nullptr)
{
  AddSwitch(_("ENABLE ENHANCED VIEW"), RecalboxConf::Instance().GetArcadeViewEnhanced(), (int)Components::EnhancedView, this, _(MENUMESSAGE_UI_ARCADE_ENHANCED_MSG));

  AddSwitch(_("FOLD CLONES BY DEFAULT"), RecalboxConf::Instance().GetArcadeViewFoldClones(), (int)Components::FoldClones, this, _(MENUMESSAGE_UI_ARCADE_HIDE_CLONES_MSG));

  AddSwitch(_("HIDE BIOS"), RecalboxConf::Instance().GetArcadeViewHideBios(), (int)Components::HideBios, this, _(MENUMESSAGE_UI_ARCADE_HIDE_BIOS_MSG));

  AddSwitch(_("HIDE NON-WORKING GAMES"), RecalboxConf::Instance().GetArcadeViewHideNonWorking(), (int)Components::HideNonWorking, this, _(MENUMESSAGE_UI_ARCADE_HIDE_NONWORKING_MSG));

  AddSwitch(_("ALWAYS USE OFFICIAL NAMES"), RecalboxConf::Instance().GetArcadeUseDatabaseNames(), (int)Components::UseDatabasesNames, this, _(MENUMESSAGE_UI_ARCADE_OFFICIAL_NAMES_MSG));
}

GuiMenuArcadeOptions::~GuiMenuArcadeOptions()
{
  // Force refreshing all gamelists
  ViewController::Instance().InvalidateAllGamelistsExcept(nullptr);
}

void GuiMenuArcadeOptions::SwitchComponentChanged(int id, bool status)
{
  switch((Components)id)
  {
    case Components::EnhancedView: RecalboxConf::Instance().SetArcadeViewEnhanced(status).Save(); break;
    case Components::FoldClones: RecalboxConf::Instance().SetArcadeViewFoldClones(status).Save(); break;
    case Components::HideBios: RecalboxConf::Instance().SetArcadeViewHideBios(status).Save(); break;
    case Components::HideNonWorking: RecalboxConf::Instance().SetArcadeViewHideNonWorking(status).Save(); break;
    case Components::UseDatabasesNames: RecalboxConf::Instance().SetArcadeUseDatabaseNames(status).Save(); break;
    default: break;
  }
}

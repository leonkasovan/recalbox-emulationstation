//
// Created by bkg2k on 06/01/2021.
//
// As part of the RECALBOX Project
// http://www.recalbox.com
//

#include "GuiMenuGameSettings.h"
#include "GuiMenuNetplay.h"
#include "GuiMenuRetroAchievements.h"
#include "GuiMenuTools.h"
#include "guis/GuiMsgBox.h"
#include "views/MenuFilter.h"
#include <LibretroRatio.h>
#include <guis/MenuMessages.h>
#include <systems/SystemManager.h>

GuiMenuGameSettings::GuiMenuGameSettings(WindowManager& window, SystemManager& systemManager)
  : GuiMenuBase(window, _("GAMES SETTINGS"), this)
  , mSystemManager(systemManager)
{
  // Screen ratio choice
  bool isCrt = Board::Instance().CrtBoard().IsCrtAdapterAttached();
  if (! isCrt && RecalboxConf::Instance().GetMenuType() != RecalboxConf::Menu::Bartop)
    AddList<String>(_("GAME RATIO"), (int)Components::Ratio, this, GetRatioEntries(), _(MENUMESSAGE_GAME_RATIO_HELP_MSG));

  // RecalboxOverlays
  AddSwitch(_("RECALBOX OVERLAYS"), RecalboxConf::Instance().GetGlobalRecalboxOverlays(), (int)Components::RecalboxOverlays, this, _(MENUMESSAGE_GAME_OVERLAYS_HELP_MSG));

  // smoothing
  mSmooth = AddSwitch(_("SMOOTH GAMES"), RecalboxConf::Instance().GetGlobalSmooth(), (int)Components::Smooth, this, _(MENUMESSAGE_GAME_SMOOTH_HELP_MSG));

  // rewind
  AddSwitch(_("REWIND"), RecalboxConf::Instance().GetGlobalRewind(), (int)Components::Rewind, this,_(MENUMESSAGE_GAME_REWIND_HELP_MSG));

  // Softpatching
  AddList<RecalboxConf::SoftPatching>(_("SOFTPATCHING"), (int)Components::Softpatching, this, GetSoftpatchingEntries(), _(MENUMESSAGE_GAME_SOFTPATCHING));

  // show savestates
  mShowSaveStates = AddSwitch(_("SHOW SAVE STATES ON START"), RecalboxConf::Instance().GetGlobalShowSaveStateBeforeRun(), (int)Components::ShowSaveStates, this, _(MENUMESSAGE_GAME_SHOW_SAVESTATES_HELP_MSG));

  // autosave
  mAutoSave = AddSwitch(_("AUTO SAVE/LOAD"), RecalboxConf::Instance().GetGlobalAutoSave(), (int)Components::AutoSave, this, _(MENUMESSAGE_GAME_AUTOSAVELOAD_HELP_MSG));

  // Press twice to quit
  AddSwitch(_("PRESS TWICE TO QUIT GAME"), RecalboxConf::Instance().GetGlobalQuitTwice(), (int)Components::QuitTwice, this, _(MENUMESSAGE_GAME_PRESS_TWICE_QUIT_HELP_MSG));

  if(!isCrt)
  {
    // Integer Scale
    AddSwitch(_("INTEGER SCALE (PIXEL PERFECT)"), RecalboxConf::Instance().GetGlobalIntegerScale(), (int)Components::IntegerScale, this, _(MENUMESSAGE_GAME_INTEGER_SCALE_HELP_MSG));

    // Shaders preset
    AddList<String>(_("SHADERS SET"), (int)Components::ShaderSet, this, GetShaderPresetsEntries(), _(MENUMESSAGE_GAME_SHADERSET_HELP_MSG));

    // Shaders
    AddList<String>(_("ADVANCED SHADERS"), (int)Components::Shaders, this, GetShadersEntries(), _(MENUMESSAGE_GAME_SHADERS_HELP_MSG));

    // HD mode
    if(MenuFilter::ShouldDisplayMenuEntry(MenuFilter::HDMode))
      AddSwitch(_("HD MODE"), RecalboxConf::Instance().GetGlobalHDMode(), (int)Components::HDMode, this, _(MENUMESSAGE_GAME_HD_MODE_HELP_MSG));

    // Widescreen mode
    if(MenuFilter::ShouldDisplayMenuEntry(MenuFilter::Widescreen))
      AddSwitch(_("WIDESCREEN (16/9)"), RecalboxConf::Instance().GetGlobalWidescreenMode(), (int)Components::WideScreenMode, this, _(MENUMESSAGE_GAME_WIDESCREEN_MODE_HELP_MSG));

    if(Board::Instance().HasVulkanSupport())
      AddSwitch(_("ENABLE VULKAN DRIVER"), RecalboxConf::Instance().GetGlobalVulkanDriver(), (int)Components::VulkanDriver, this, _(MENUMESSAGE_GAME_VULKAN_DRIVER_HELP_MSG));
  }


  // Super GameBoy
  AddList<String>(_("GAME BOY MODE"), (int)Components::SuperGameBoy, this, GetSuperGameBoyEntries(), _(MENUMESSAGE_GAME_SUPERGAMEBOY_HELP_MSG));

  // Retroachievements
  if (RecalboxConf::Instance().GetMenuType() != RecalboxConf::Menu::Bartop)
  {
    AddSubMenu(_("RETROACHIEVEMENTS SETTINGS"), (int)Components::RetroAchivements, _(MENUMESSAGE_RA_HELP_MSG));
    AddSubMenu(_("NETPLAY SETTINGS"), (int)Components::Netplay, _(MENUMESSAGE_NP_HELP_MSG));
  }
}

std::vector<GuiMenuBase::ListEntry<String>> GuiMenuGameSettings::GetRatioEntries()
{
  std::vector<GuiMenuBase::ListEntry<String>> list;

  String currentRatio = RecalboxConf::Instance().GetGlobalRatio();
  for (const auto& ratio : LibretroRatio::GetRatio())
    list.push_back({ ratio.first, ratio.second, currentRatio == ratio.second });

  return list;
}

std::vector<GuiMenuBase::ListEntry<String>> GuiMenuGameSettings::GetShadersEntries()
{
  std::vector<GuiMenuBase::ListEntry<String>> list;

  GuiMenuTools::ShaderList shaderList = GuiMenuTools::ListShaders();
  String currentShader = RecalboxConf::Instance().GetGlobalShaders();
  list.push_back({ _("NONE"), "", currentShader.empty() });
  for (const GuiMenuTools::Shader& shader : shaderList)
    list.push_back({ shader.Displayable, shader.ShaderPath.ToString(), currentShader == shader.ShaderPath.ToString() });

  return list;
}

std::vector<GuiMenuBase::ListEntry<String>> GuiMenuGameSettings::GetSuperGameBoyEntries()
{
  std::vector<GuiMenuBase::ListEntry<String>> list;

  String currentOption = RecalboxConf::Instance().GetSuperGameBoy();
  list.push_back({ _("GAME BOY"), "gb", currentOption == "gb" });
  list.push_back({ _("SUPER GAME BOY"), "sgb", currentOption == "sgb" });
  list.push_back({ _("ASK AT LAUNCH"), "ask", currentOption == "ask" });
  return list;
}

std::vector<GuiMenuBase::ListEntry<RecalboxConf::SoftPatching>> GuiMenuGameSettings::GetSoftpatchingEntries()
{
  std::vector<GuiMenuBase::ListEntry<RecalboxConf::SoftPatching>> list;

  RecalboxConf::SoftPatching currentOption = RecalboxConf::Instance().GetGlobalSoftpatching();

  list.emplace_back( _("AUTO"), RecalboxConf::SoftPatching::Auto, currentOption == RecalboxConf::SoftPatching::Auto );
  list.emplace_back( _("LAUNCH LAST"), RecalboxConf::SoftPatching::LaunchLast, currentOption == RecalboxConf::SoftPatching::LaunchLast );
  list.emplace_back( _("SELECT"), RecalboxConf::SoftPatching::Select, currentOption == RecalboxConf::SoftPatching::Select );
  list.emplace_back( _("DISABLE"), RecalboxConf::SoftPatching::Disable, currentOption == RecalboxConf::SoftPatching::Disable);

  return list;
}

std::vector<GuiMenuBase::ListEntry<String>> GuiMenuGameSettings::GetShaderPresetsEntries()
{
  std::vector<GuiMenuBase::ListEntry<String>> list;

  String currentPreset = RecalboxConf::Instance().GetGlobalShaderSet();
  if (currentPreset != "scanlines" && currentPreset != "retro" && currentPreset != "crtcurved") currentPreset = "none";
  list.push_back({ _("NONE"), "none", currentPreset == "none" });
  list.push_back({ _("CRT CURVED"), "crtcurved", currentPreset == "crtcurved" });
  list.push_back({ _("SCANLINES"), "scanlines", currentPreset == "scanlines" });
  list.push_back({ _("RETRO"), "retro", currentPreset == "retro" });

  return list;
}

void GuiMenuGameSettings::SubMenuSelected(int id)
{
  if ((Components)id == Components::RetroAchivements) mWindow.pushGui(new GuiMenuRetroAchievements(mWindow));
  else if ((Components)id == Components::Netplay) mWindow.pushGui(new GuiMenuNetplay(mWindow, mSystemManager));
}


void GuiMenuGameSettings::OptionListComponentChanged(int id, int index, const RecalboxConf::SoftPatching& value)
{
  (void)index;
  if ((Components)id == Components::Softpatching)
    RecalboxConf::Instance().SetGlobalSoftpatching(value).Save();
}

void GuiMenuGameSettings::OptionListComponentChanged(int id, int index, const String& value)
{
  (void)index;
  switch((Components)id)
  {
    case Components::Ratio: RecalboxConf::Instance().SetGlobalRatio(value).Save(); break;
    case Components::Shaders:  RecalboxConf::Instance().SetGlobalShaders(value).Save(); break;
    case Components::ShaderSet:
    {
      if (value != "none" && (RecalboxConf::Instance().GetGlobalSmooth()))
        mWindow.pushGui(new GuiMsgBox(mWindow,
                                      _("YOU JUST ACTIVATED THE SHADERS FOR ALL SYSTEMS. FOR A BETTER RENDERING, IT IS ADVISED TO DISABLE GAME SMOOTHING. DO YOU WANT TO CHANGE THIS OPTION AUTOMATICALLY?"),
                                      _("LATER"), nullptr, _("YES"), [this]
                                      { mSmooth->setState(false); }));
      RecalboxConf::Instance().SetGlobalShaderSet(value).Save();
      break;
    }
    case Components::SuperGameBoy: RecalboxConf::Instance().SetSuperGameBoy(value).Save(); break;
    case Components::RecalboxOverlays:
    case Components::Smooth:
    case Components::Rewind:
    case Components::AutoSave:
    case Components::ShowSaveStates:
    case Components::QuitTwice:
    case Components::IntegerScale:
    case Components::Softpatching:
    case Components::RetroAchivements:
    case Components::Netplay:
    case Components::HDMode:
    case Components::WideScreenMode:
    case Components::VulkanDriver:
    default: break;
  }
}

void GuiMenuGameSettings::SwitchComponentChanged(int id, bool& status)
{
  switch((Components)id)
  {
    case Components::Smooth: RecalboxConf::Instance().SetGlobalSmooth(status).Save(); break;
    case Components::RecalboxOverlays: RecalboxConf::Instance().SetGlobalRecalboxOverlays(status).Save(); break;
    case Components::Rewind: RecalboxConf::Instance().SetGlobalRewind(status).Save(); break;
    case Components::ShowSaveStates:
      RecalboxConf::Instance().SetGlobalShowSaveStateBeforeRun(status).Save();
      if (status)
      {
        mAutoSave->setState(false);
        RecalboxConf::Instance().SetGlobalAutoSave(false).Save();
      }
      break;
    case Components::AutoSave:
      RecalboxConf::Instance().SetGlobalAutoSave(status).Save();
      if (status)
      {
        mShowSaveStates->setState(false);
        RecalboxConf::Instance().SetGlobalShowSaveStateBeforeRun(false).Save();
      }
      break;
    case Components::QuitTwice: RecalboxConf::Instance().SetGlobalQuitTwice(status).Save(); break;
    case Components::IntegerScale: RecalboxConf::Instance().SetGlobalIntegerScale(status).Save(); break;
    case Components::HDMode: RecalboxConf::Instance().SetGlobalHDMode(status).Save(); break;
    case Components::WideScreenMode: RecalboxConf::Instance().SetGlobalWidescreenMode(status).Save(); break;
    case Components::VulkanDriver: RecalboxConf::Instance().SetGlobalVulkanDriver(status).Save(); break;
    case Components::Ratio:
    case Components::Softpatching:
    case Components::Shaders:
    case Components::ShaderSet:
    case Components::RetroAchivements:
    case Components::Netplay:
    case Components::SuperGameBoy:
      break;
  }
}


//
// Created by bkg2k on 04/01/2021.
//
// As part of the RECALBOX Project
// http://www.recalbox.com
//

#include <guis/menus/GuiMenuSystemConfiguration.h>
#include <components/OptionListComponent.h>
#include <components/SwitchComponent.h>
#include <systems/SystemManager.h>
#include <guis/MenuMessages.h>
#include <LibretroRatio.h>
#include "GuiMenuTools.h"

GuiMenuSystemConfiguration::GuiMenuSystemConfiguration(WindowManager& window, SystemData& system, SystemManager& systemManager, AdvancedMenuOptions options)
  : GuiMenuBase(window, system.FullName(), nullptr)
  , mSystemManager(systemManager)
  , mSystem(system)
{
  // Default emulator/core
  if(options.emulator)
    AddList(_("Emulator"), (int)Components::Emulator, this, GetEmulatorEntries(), _(MENUMESSAGE_ADVANCED_EMU_EMU_HELP_MSG));

  // Screen ratio choice
  if(options.ratio)
    AddList(_("GAME RATIO"), (int)Components::Ratio, this, GetRatioEntries(), _(MENUMESSAGE_GAME_RATIO_HELP_MSG));

  // smoothing
  if(options.ratio)
    AddSwitch(_("SMOOTH GAMES"), RecalboxConf::Instance().GetSystemSmooth(system), (int)Components::Smooth, this, _(MENUMESSAGE_GAME_SMOOTH_HELP_MSG));

  // rewind
  if(options.rewind)
    AddSwitch(_("REWIND"), RecalboxConf::Instance().GetSystemRewind(system), (int)Components::Rewind, this, _(MENUMESSAGE_GAME_REWIND_HELP_MSG));

  // autosave
  if(options.autosave)
    AddSwitch(_("AUTO SAVE/LOAD"), RecalboxConf::Instance().GetSystemAutoSave(system), (int)Components::AutoSave, this, _(MENUMESSAGE_GAME_AUTOSAVELOAD_HELP_MSG));

  // Shaders
  if(options.shaders)
    AddList(_("SHADERS"), (int)Components::Shaders, this, GetShadersEntries(), _(MENUMESSAGE_GAME_SHADERS_HELP_MSG));

  // Shaders preset
  if(options.shaderSet)
    AddList(_("SHADERS SET"), (int)Components::ShaderSet, this, GetShaderSetEntries(), _(MENUMESSAGE_GAME_SHADERSET_HELP_MSG));
}

std::vector<GuiMenuBase::ListEntry<String>> GuiMenuSystemConfiguration::GetEmulatorEntries()
{
  std::vector<ListEntry<String>> list;

  String currentEmulator(RecalboxConf::Instance().GetSystemEmulator(mSystem));
  String currentCore    (RecalboxConf::Instance().GetSystemCore(mSystem));
  GuiMenuTools::EmulatorAndCoreList eList =
    GuiMenuTools::ListEmulatorAndCore(mSystem, mDefaultEmulator, mDefaultCore, currentEmulator, currentCore);
  if (!eList.empty())
    for (const GuiMenuTools::EmulatorAndCore& emulator : eList)
      list.push_back({ emulator.Displayable, emulator.Identifier, emulator.Selected });

  return list;
}

std::vector<GuiMenuBase::ListEntry<String>> GuiMenuSystemConfiguration::GetRatioEntries()
{
  std::vector<ListEntry<String>> list;

  String currentRatio = RecalboxConf::Instance().GetSystemRatio(mSystem);
  for (const auto& ratio : LibretroRatio::GetRatio())
    list.push_back({ ratio.first, ratio.second, currentRatio == ratio.second });

  return list;
}

std::vector<GuiMenuBase::ListEntry<String>> GuiMenuSystemConfiguration::GetShadersEntries()
{
  std::vector<ListEntry<String>> list;

  GuiMenuTools::ShaderList shaderList = GuiMenuTools::ListShaders();
  String currentShader = RecalboxConf::Instance().GetSystemShaders(mSystem);
  list.push_back({ _("NONE"), "", currentShader.empty() });
  for (const GuiMenuTools::Shader& shader : shaderList)
    list.push_back({ shader.Displayable, shader.ShaderPath.ToString(), currentShader == shader.ShaderPath.ToString() });

  return list;
}

std::vector<GuiMenuBase::ListEntry<String>> GuiMenuSystemConfiguration::GetShaderSetEntries()
{
  std::vector<ListEntry<String>> list;

  String currentShader = RecalboxConf::Instance().GetSystemShaderSet(mSystem);
  if (currentShader.empty() || !String("|none|scanlines|retro|crtcurved|").Contains(String('|').Append(currentShader).Append('|'))) currentShader = "none";
  list.push_back({ _("NONE"), "none", currentShader == "none" });
  list.push_back({ _("CRT CURVED"), "crtcurved", currentShader == "crtcurved" });
  list.push_back({ _("SCANLINES"), "scanlines", currentShader == "scanlines" });
  list.push_back({ _("RETRO"), "retro", currentShader == "retro" });

  return list;
}

void GuiMenuSystemConfiguration::OptionListComponentChanged(int id, int index, const String& value)
{
  (void)index;
  switch((Components)id)
  {
    case Components::Emulator:
    {
      // Split emulator & core
      String emulator, core;
      if (value.Extract(':', emulator, core, false))
      {
        if (emulator == mDefaultEmulator && core == mDefaultCore)
          RecalboxConf::Instance().SetSystemEmulator(mSystem, "")
                                  .SetSystemCore(mSystem, "").Save();
        else
          RecalboxConf::Instance().SetSystemEmulator(mSystem, emulator)
                                  .SetSystemCore(mSystem, core).Save();
      }
      else { LOG(LogError) << "[SystemConfigurationGui] Error splitting emulator and core!"; }
      break;
    }
    case Components::Ratio: RecalboxConf::Instance().SetSystemRatio(mSystem, value).Save(); break;
    case Components::Shaders: RecalboxConf::Instance().SetSystemShaders(mSystem, value).Save(); break;
    case Components::ShaderSet: RecalboxConf::Instance().SetSystemShaderSet(mSystem, value).Save(); break;
    case Components::Smooth:
    case Components::Rewind:
    case Components::AutoSave: break;
  }
}

void GuiMenuSystemConfiguration::SwitchComponentChanged(int id, bool& status)
{
  switch((Components)id)
  {
    case Components::Smooth: RecalboxConf::Instance().SetSystemSmooth(mSystem, status).Save(); break;
    case Components::Rewind: RecalboxConf::Instance().SetSystemRewind(mSystem, status).Save(); break;
    case Components::AutoSave: RecalboxConf::Instance().SetSystemAutoSave(mSystem, status).Save(); break;
    case Components::Emulator:
    case Components::Ratio:
    case Components::Shaders:
    case Components::ShaderSet: break;
  }
}

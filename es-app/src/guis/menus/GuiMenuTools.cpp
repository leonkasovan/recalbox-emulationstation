//
// Created by bkg2k on 06/01/2021.
//
// As part of the RECALBOX Project
// http://www.recalbox.com
//

#include <systems/SystemManager.h>
#include <guis/menus/GuiMenuTools.h>
#include <utils/locale/LocaleHelper.h>
#include <algorithm>

const Path GuiMenuTools::sShadersPath("/recalbox/share/shaders");

GuiMenuTools::EmulatorAndCoreList
GuiMenuTools::ListGameEmulatorAndCore(FileData& game, String& outDefaultEmulator,
                                      String& outDefaultCore, const String& currentEmulator,
                                      const String& currentCore)
{
  EmulatorAndCoreList result;
  String emulator = currentEmulator;
  String core = currentCore;

  bool override = false;
  if (EmulatorManager::GetGameEmulatorOverriden(game, outDefaultEmulator, outDefaultCore, override))
  {
    if (override)
    {
      String displayName(outDefaultEmulator);
      if (displayName != outDefaultCore) displayName.Append(' ').Append(outDefaultCore);
      result.push_back({ "", _("FORCED").Append(' ').Append(displayName), true });
      return result;
    }

    String defaultSystemEmulator;
    String defaultSystemCore;
    EmulatorManager::GetSystemEmulator(game.System(), defaultSystemEmulator, defaultSystemCore);
    String displayName(defaultSystemEmulator);
    if (displayName != defaultSystemCore) displayName.Append(' ').Append(defaultSystemCore);
    result.push_back({ "", _("SYSTEM DEFAULT").Append(" (").Append(displayName).Append(')'), currentEmulator.empty() && currentCore.empty() });

    if (emulator.empty()) emulator = outDefaultEmulator;
    if (core.empty()) core = outDefaultCore;
    for (const String& emulatorName : EmulatorManager::GetEmulators(game.System()))
      for (const String& coreName : EmulatorManager::GetCores(game.System(), emulatorName))
      {
        // Get display name, composed of "emulator core" or just "emulator" of both are the same (standalone)
        // Add "(default)" if this is the default emulator/core
        displayName.Assign(emulatorName);
        if (displayName != coreName) displayName.Append(' ').Append(coreName);

        // Build a key "emulator:core"
        String emulatorAndCore(emulatorName);
        emulatorAndCore.Append(':').Append(coreName);
        bool match = (emulatorName == emulator && coreName == core) &&
                     (!currentEmulator.empty() || !currentCore.empty());
        if (match) { LOG(LogDebug) << "[GUI] Selected emulator/core: " << emulatorAndCore; }
        // Add the entry
        result.push_back({ emulatorAndCore, displayName, match });
      }
  }
  else
  {
    result.push_back({ "", "SYSTEM DEFAULT", true });
    { LOG(LogError) << "[GUI] Can't get default emulator/core for " << game.RomPath(); }
  }

  return result;

}

GuiMenuTools::EmulatorAndCoreList
GuiMenuTools::ListEmulatorAndCore(SystemData& system, String& outDefaultEmulator,
                                  String& outDefaultCore, const String& currentEmulator,
                                  const String& currentCore)
{
  EmulatorAndCoreList result;
  String emulator = currentEmulator;
  String core = currentCore;

  if (EmulatorManager::GetDefaultEmulator(system, outDefaultEmulator, outDefaultCore))
  {
    bool selected = false;
    if (emulator.empty()) emulator = outDefaultEmulator;
    if (core.empty()) core = outDefaultCore;
    for (const String& emulatorName : EmulatorManager::GetEmulators(system))
      for (const String& coreName : EmulatorManager::GetCores(system, emulatorName))
      {
        // Get display name, composed of "emulator core" or just "emulator" of both are the same (standalone)
        // Add "(default)" if this is the default emulator/core
        String displayName(emulatorName);
        if (displayName != coreName) displayName.Append(' ').Append(coreName);
        if (outDefaultCore == coreName && outDefaultEmulator == emulatorName)
          displayName.Append(" (").Append(_("DEFAULT")).Append(')');

        // Build a key "emulator:core"
        String emulatorAndCore(emulatorName);
        emulatorAndCore.Append(':').Append(coreName);
        bool match = emulatorName == emulator && coreName == core;
        if (match) { LOG(LogDebug) << "[GUI] Selected emulator/core: " << emulatorAndCore; }
        selected |= match;
        // Add the entry
        result.push_back({ emulatorAndCore, displayName, match });
      }
    if (!selected)
      result.push_back({ "DEFAULT", "DEFAULT", true });
  }
  else { LOG(LogError) << "[GUI] Can't get default emulator/core for " << system.FullName(); }

  return result;
}


void GuiMenuTools::ReadShaderFolder(const Path& root, Path::PathList& glslp)
{
  Path::PathList files = root.GetDirectoryContent();
  for(const Path& path : files)
    if (path.Extension() == ".glslp")
      glslp.push_back(path);
    else if (path.IsDirectory())
      ReadShaderFolder(path, glslp);
}

Path::PathList GuiMenuTools::GetShaderList()
{
  Path::PathList glslp;
  ReadShaderFolder(sShadersPath, glslp);
  std::sort(glslp.begin(), glslp.end());
  return glslp;
}

GuiMenuTools::ShaderList GuiMenuTools::ListShaders()
{
  ShaderList result;

  for (const Path& path : GetShaderList())
  {
    bool ok = false;
    String shaderName = path.MakeRelative(sShadersPath, ok).ToString();
    shaderName.Replace('/', " - ", 3).Replace('_', " ", 1);
    result.push_back({ path, shaderName });
  }

  return result;
}

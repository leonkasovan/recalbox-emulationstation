//
// Created by gugue_u on 01/04/2022.
//

#pragma once


#include "RecalboxConf.h"
#include "systems/SystemData.h"
#include "systems/SystemManager.h"

class SuperGameBoyData
{
  public:
    SuperGameBoyData()
      : mEnableSuperGameBoy(RecalboxConf::Instance().GetSuperGameBoy() == "sgb")
    {};

    [[nodiscard]] bool ShouldEnable(const SystemData& system) const { return system.IsGameBoy() && mEnableSuperGameBoy; }

    [[nodiscard]] String Core(const FileData& game, const String& defaultCore) const
    {
      // Change to mgba if user did not overload the core for gameboy
      if (!EmulatorManager::ConfigOverloaded(game))
        return "mgba";
      else
        return defaultCore;
    }

    void Enable(bool enabled)
    {
      mEnableSuperGameBoy = enabled;
    }

    [[nodiscard]] bool ShouldAskForSuperGameBoy(const SystemData& system) const
    {
      return system.IsGameBoy() &&
             (!mEnableSuperGameBoy && RecalboxConf::Instance().GetSuperGameBoy() == "ask");
    }

  private:
    bool mEnableSuperGameBoy;
};

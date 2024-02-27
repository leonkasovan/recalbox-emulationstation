//
// Created by gugue_u on 01/04/2022.
//

#pragma once


#include "hardware/crt/CrtAdapterType.h"
#include "hardware/Board.h"
#include "CrtConf.h"
#include "emulators/EmulatorData.h"

class JammaData
{
  private:
    bool systemIsDreamcastArcade(const String& systemName) const
    {
      return systemName == "atomiswave" || systemName == "naomigd" || systemName == "naomi";
    }

    bool systemIsArcade(const FileData& game, const EmulatorData& emulator) const
    {
      return systemIsDreamcastArcade(game.System().Name())
              || (emulator.Emulator() == "libretro" && (emulator.Core() == "fbneo" || emulator.Core() == "mame2015" || emulator.Core() == "mame"));
    }

  public:
    JammaData() {};
    bool ShouldConfigureJammaConfiggen() const {
      return  (Board::Instance().CrtBoard().GetCrtAdapter() == CrtAdapterType::RGBJamma);
    }

    bool ShouldSwitchTo6ButtonLayout(const FileData& game, const EmulatorData& emulator) const {
      return  ShouldConfigureJammaConfiggen()
              && CrtConf::Instance().GetSystemCRTJammaPanelButtons() == "6"
              && ! systemIsArcade(game, emulator);
    }

    bool ShouldConfigureNeoGeoLayout(const FileData& game, const EmulatorData& emulator) const {
      String buttonOnPanel = CrtConf::Instance().GetSystemCRTJammaPanelButtons();
      return  ShouldConfigureJammaConfiggen()
              && (buttonOnPanel == "6" || buttonOnPanel == "5")
              && systemIsArcade(game, emulator);
    }

    String JammaControlType(const FileData& game, const EmulatorData& emulator) const {
      if(ShouldSwitchTo6ButtonLayout(game, emulator))
        return "6btns";
      else if(ShouldConfigureNeoGeoLayout(game, emulator))
          return CrtConf::Instance().GetSystemCRTJammaNeogeoLayout();
      else return "standard";
    }
};

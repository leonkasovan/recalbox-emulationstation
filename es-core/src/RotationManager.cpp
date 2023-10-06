//
// Created by digitalLumberjack on 10/03/23.
//

#include "RotationManager.h"
#include "hardware/Board.h"
#include "recalbox/BootConf.h"
#include "Renderer.h"
#include "systems/SystemData.h"

bool RotationManager::ShouldRotateTateEnter(RotationType& rotationType)
{
  (void)rotationType;
  // We will rotate the tate system only if:
  // - the board can rotate
  // - there is no Left or Right rotation yet
  // - the system Tate is configured to be rotated
 /* if(Board::Instance().GetRotationCapabilities().canRotate
    && !Renderer::Instance().IsRotatedSide()
    && RecalboxConf::Instance().GetCollectionTateRotateSystemView()){
      rotationType = static_cast<RotationType>(RecalboxConf::Instance().GetCollectionTateRotation());
      return true;
  }*/
  return false;
}

bool RotationManager::ShouldRotateTateExit(RotationType& rotationType)
{
  (void)rotationType;
  // We will rotate back the tate system only if:
  // - the board can rotate
  // - there is Left or Right rotation
  // - the system Tate is configured to be rotated
  // - the system wide rotation is not left or right
  /*if(Board::Instance().GetRotationCapabilities().canRotate
     && Renderer::Instance().IsRotatedSide()
     && RecalboxConf::Instance().GetCollectionTateRotateSystemView()
     && (GetSystemRotation() == RotationType::None || GetSystemRotation() == RotationType::Upsidedown)){
    rotationType = RotationType::None;
    return true;
  }*/
  return false;
}

RotationType RotationManager::ShouldRotateGame(const FileData& game)
{
  // We will rotate the game if:
  // - the system rotation is set
  // OR
  // - the game is a vertical game
  // - the board can rotate
  // - the rotate game option is set
  RotationType gameRotation = RotationType::None;
  if(GetSystemRotation() != RotationType::None)
  {
    gameRotation = GetSystemRotation();
    {LOG(LogDebug) << "[RotationManager] System rotation is set, setting game rotation to " << RotationUtils::StringValue(gameRotation);}
  }
  else if(game.Metadata().Rotation() != RotationType::None || game.System().Name() == "vectrex")
  {
    const RotationCapability cap = Board::Instance().GetRotationCapabilities();
    // The board should be able to rotate, either the config has been set by user, either we use the automatic capability
    if(cap.rotationAvailable)
    {
      if(RecalboxConf::Instance().IsDefined(RecalboxConf::sTateGameRotation)) {
        // Forced rotation
        gameRotation = RotationUtils::FromUint(RecalboxConf::Instance().GetTateGameRotation());
        {LOG(LogDebug) << "[RotationManager] Setting game rotation to " << RotationUtils::StringValue(gameRotation) << " from recalbox.conf";}
      } else if(cap.autoRotateGames){
        gameRotation = cap.defaultRotationWhenTate;
        {LOG(LogDebug) << "[RotationManager] Setting game rotation to " << RotationUtils::StringValue(gameRotation) << " from Board capabilities";}
      }
    }
  }
  return gameRotation;
}

bool RotationManager::ShouldRotateFrontendControls()
{
  // We will rotate the controls if:
  // - the board can rotate
  // - the board is rotated for tate or system-wide
  const RotationCapability cap = Board::Instance().GetRotationCapabilities();
  bool rotate = cap.rotationAvailable && cap.rotateControls && Renderer::Instance().IsRotatedSide();
  if (rotate) { LOG(LogDebug) << "[RotationManager] Rotate controls ON"; }
  return rotate;
}

bool RotationManager::ShouldRotateGameControls(const FileData& game)
{
  // We will rotate the controls if:
  // - the game is a vertical game
  // - the board can rotate
  // - the board is rotated for tate or system-wide
  const RotationCapability cap = Board::Instance().GetRotationCapabilities();
  const RotationType gameRotation = ShouldRotateGame(game);
  bool rotate = (gameRotation == RotationType::Left || gameRotation == RotationType::Right) && cap.rotateControls;
  if (rotate) { LOG(LogDebug) << "[RotationManager] Rotate game controls ON"; }
  return rotate;
}

RotationType RotationManager::GetSystemRotation()
{
  return BootConf::Instance().GetRotation();
}

bool RotationManager::IsVerticalGame(const FileData& game){
  return game.Metadata().Rotation() == RotationType::Left || game.Metadata().Rotation() == RotationType::Right;
}

//
// Created by bkg2k on 31/05/23.
//
#pragma once

// Forward declarations
class ArcadeGame;
class FileData;

//! Linked arcade/game structure
struct ArcadeTupple
{
  const ArcadeGame* mArcade; //!< Arcade game - May be null
  FileData* mGame;           //!< Game - Never null

  //! Constructor
  ArcadeTupple(const ArcadeGame* arcade, FileData* game) : mArcade(arcade), mGame(game) {}
};

typedef std::vector<ArcadeTupple*> ArcadeTupplePointerList;
typedef std::vector<ArcadeTupple> ArcadeTuppleList;


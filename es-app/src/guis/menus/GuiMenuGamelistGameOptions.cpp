//
// Created by bkg2k on 28/01/2021.
//
// As part of the RECALBOX Project
// http://www.recalbox.com
//

#include <guis/MenuMessages.h>
#include <guis/GuiScraperSingleGameRun.h>
#include "GuiMenuGamelistGameOptions.h"
#include "GuiMenuTools.h"
#include <components/EditableComponent.h>
#include <components/RatingComponent.h>
#include <components/SwitchComponent.h>
#include <views/ViewController.h>
#include <LibretroRatio.h>
#include "games/GameFilesUtils.h"

GuiMenuGamelistGameOptions::GuiMenuGamelistGameOptions(WindowManager& window, ISimpleGameListView& view, SystemManager& systemManager, SystemData& system, FileData& game)
  : GuiMenuBase(window, _("GAME OPTIONS"), this)
  , mView(view)
  , mSystemManager(systemManager)
  , mSystem(system)
  , mGame(game)
{
  if (mGame.IsGame())
  {
    String gameName(game.Name());
    gameName.Append(" (").Append(game.RomPath().Filename()).Append(')');
    SetFooter(_("GAME %s").Replace("%s", gameName.UpperCaseUTF8()));
  }
  else if (mGame.IsFolder())
  {
    SetFooter(_("FOLDER %s").Replace("%s", mGame.Name().ToUpperCaseUTF8()));
  }

  // Run width
  if (mGame.IsGame())
    mEmulator = AddList<String>(_("RUN WITH"), (int)Components::Emulator, this, GetEmulatorEntries(), _(MENUMESSAGE_ADVANCED_EMU_EMU_HELP_MSG));

  // Patch width
  if (mGame.IsGame() && !GameFilesUtils::GetSoftPatches(&mGame).empty())
  {
    mPath = AddList<Path>(_("SOFTPATCHING"), (int)Components::Patch, this, GetPatchEntries());

  }
  // Ratio
  if (mGame.IsGame())
    mRatio = AddList<String>(_("Ratio"), (int)Components::Ratio, this, GetRatioEntries(), _(MENUMESSAGE_GAME_RATIO_HELP_MSG));

  // Game name
  mName = AddEditable(_("Name"), mGame.Metadata().Name(), (int)Components::Name, this, false);

  // Rating
  if (mGame.IsGame())
    mRating = AddRating(_("Rating"), mGame.Metadata().Rating(), (int)Components::Rating, this);

  // Normalized genre
  if (mGame.IsGame())
    mGenre = AddList<GameGenres>(_("Genre"), (int)Components::Genre, this, GetGenreEntries());

  // Description
  mDescription = AddEditable(_("Description"), mGame.Metadata().Description(), (int)Components::Description, this, false);

  // Favorite
  if (mGame.IsGame())
    mFavorite = AddSwitch(_("Favorite"), mGame.Metadata().Favorite(), (int)Components::Favorite, this);

  // Hidden
  mHidden = AddSwitch(_("Hidden"), mGame.Metadata().Hidden(), (int)Components::Hidden, this);

  // Adult
  if (mGame.IsGame())
    mAdult = AddSwitch(_("Adult"), mGame.Metadata().Adult(), (int)Components::Adult, this);
  // Adult
  if (mGame.IsGame())
    mRotation = AddSwitch(_("Rotation"), mGame.Metadata().Rotation() != RotationType::None, (int)Components::Rotation, this);

  // Scrape
  if (mGame.IsGame())
    AddSubMenu(_("SCRAPE"), (int)Components::Scrape);


//  _N("%i GAME HIDDEN", "%i GAMES HIDDEN", data.Hidden)) .Replace("%i", String(data.Hidden)
  if (mGame.IsGame() )
  {
    int timePlayed = mGame.Metadata().TimePlayed();
    if (timePlayed >= 3600)
    {
      int hours = timePlayed / 3600;
      AddText(_("TIME PLAYED"), _N("%i HOUR", "%i HOURS", hours).Replace("%i", String(hours)));
    }
    else if (timePlayed >= 60)
    {
      int minutes = timePlayed / 60;
      AddText(_("TIME PLAYED"), _N("%i MINUTE", "%i MINUTES", minutes).Replace("%i", String(minutes)));
    }
  }
}

GuiMenuGamelistGameOptions::~GuiMenuGamelistGameOptions()
{
  if(mGame.Metadata().IsDirty())
    mSystem.UpdateGamelistXml();
}

std::vector<GuiMenuBase::ListEntry<String>> GuiMenuGamelistGameOptions::GetRatioEntries()
{
  std::vector<ListEntry<String>> list;

  String currentRatio = mGame.Metadata().Ratio();
  for (const auto& ratio : LibretroRatio::GetRatio())
    list.push_back({ ratio.first, ratio.second, currentRatio == ratio.second });

  return list;
}

std::vector<GuiMenuBase::ListEntry<GameGenres>> GuiMenuGamelistGameOptions::GetGenreEntries()
{
  std::vector<ListEntry<GameGenres>> list;

  for(GameGenres genre : Genres::GetOrderedList())
    list.push_back({ Genres::GetFullName(genre), genre, genre == mGame.Metadata().GenreId() });

  return list;
}

std::vector<GuiMenuBase::ListEntry<String>> GuiMenuGamelistGameOptions::GetEmulatorEntries()
{
  std::vector<ListEntry<String>> list;

  String currentEmulator(mGame.Metadata().Emulator());
  String currentCore    (mGame.Metadata().Core());
  GuiMenuTools::EmulatorAndCoreList eList =
    GuiMenuTools::ListGameEmulatorAndCore(mGame, mDefaultEmulator, mDefaultCore, currentEmulator, currentCore);
  if (!eList.empty())
    for (const GuiMenuTools::EmulatorAndCore& emulator : eList)
      list.push_back({ emulator.Displayable, emulator.Identifier, emulator.Selected });

  return list;
}


std::vector<GuiMenuBase::ListEntry<Path>> GuiMenuGamelistGameOptions::GetPatchEntries()
{
  std::vector<ListEntry<Path>> list;
  std::vector<Path> patches = GameFilesUtils::GetSoftPatches(&mGame);


  unsigned long patchListSize = patches.size();
  list.push_back({ _("original"), Path("original")  , true});

  for(auto& path : patches)
  {
    bool isDefault = patchListSize == 1 || (path == mGame.Metadata().LastPatch() && mGame.Metadata().LastPatch().Exists());
    String patchName = path.Directory() == mGame.RomPath().Directory() ? path.Filename() + " (auto)" : path.Filename();

    list.push_back({ patchName, path , isDefault });
  }
  return list;
}

void GuiMenuGamelistGameOptions::OptionListComponentChanged(int id, int index, const String& value)
{
  (void)index;
  if ((Components)id == Components::Emulator)
  {
    mGame.Metadata().SetEmulator(String::Empty);
    mGame.Metadata().SetCore(String::Empty);
    // Split emulator & core
    String emulator;
    String core;
    if (value.Extract(':', emulator, core, false))
      if (emulator != mDefaultEmulator || core != mDefaultCore)
      {
        mGame.Metadata().SetEmulator(emulator);
        mGame.Metadata().SetCore(core);
      }
  }
  else if ((Components)id == Components::Ratio)
    mGame.Metadata().SetRatio(value);
}

void GuiMenuGamelistGameOptions::OptionListComponentChanged(int id, int index, const Path& value)
{
  (void)index;
  if ((Components)id == Components::Patch)
    mGame.Metadata().SetLastPatch(value);
}

void GuiMenuGamelistGameOptions::OptionListComponentChanged(int id, int index, const GameGenres& value)
{
  (void)index;
  if ((Components)id == Components::Genre)
    mGame.Metadata().SetGenreId(value);
}

void GuiMenuGamelistGameOptions::EditableComponentTextChanged(int id, const String& text)
{
  if ((Components)id == Components::Name)
    mGame.Metadata().SetName(text);
  else if ((Components)id == Components::Description)
    mGame.Metadata().SetDescription(text);
}

void GuiMenuGamelistGameOptions::SwitchComponentChanged(int id, bool& status)
{
  MetadataType updatedMetadata = MetadataType::None;
  switch((Components)id)
  {
    case Components::Favorite:
    {
      ViewController::Instance().ToggleFavorite(&mGame, true, status);
      updatedMetadata = MetadataType::Favorite;
      break;
    }
    case Components::Rotation:
    {
      mGame.Metadata().SetRotation(status ? RotationType::Left : RotationType::None);
      updatedMetadata = MetadataType::Rotation;
      break;
    }
    case Components::Hidden:
    {
      mGame.Metadata().SetHidden(status);
      updatedMetadata = MetadataType::Hidden;
      break;
    }
    case Components::Adult:
    {
      mGame.Metadata().SetAdult(status);
      updatedMetadata = MetadataType::Adult;
      break;
    }
    case Components::Name:
    case Components::Description:
    case Components::Rating:
    case Components::Genre:
    case Components::Scrape:
    case Components::Ratio:
    case Components::Emulator:
    case Components::Patch:
      break;
  }
  if (updatedMetadata != MetadataType::None)
    mSystemManager.UpdateSystemsOnGameChange(&mGame, updatedMetadata, false);
}

void GuiMenuGamelistGameOptions::RatingChanged(int id, float value)
{
  if ((Components)id == Components::Rating)
    mGame.Metadata().SetRating(value);
}

void GuiMenuGamelistGameOptions::SubMenuSelected(int id)
{
  if ((Components)id == Components::Scrape)
    mWindow.pushGui(new GuiScraperSingleGameRun(mWindow, mSystemManager, mGame, this));
}

void GuiMenuGamelistGameOptions::ScrapingComplete(FileData& game, MetadataType changedMetadata)
{
  (void)changedMetadata;

  // Refresh menu
  if ((changedMetadata & MetadataType::Name) != 0) mName->setText(game.Metadata().Name());
  if ((changedMetadata & MetadataType::Rating) != 0) mRating->setValue(game.Metadata().Rating());
  if ((changedMetadata & MetadataType::GenreId) != 0) mGenre->select(game.Metadata().GenreId());
  if ((changedMetadata & MetadataType::Synopsis) != 0) mDescription->setText(game.Metadata().Description());
  if ((changedMetadata & MetadataType::Adult) != 0) mAdult->setState(game.Metadata().Adult());
  mMenu.onSizeChanged();
}

#include <games/GameFilesUtils.h>
#include "GuiMenuGamelistGameDeleteSelectionOptions.h"
#include "GuiMenuTools.h"
#include "games/FileData.h"
#include <views/ViewController.h>
#include <guis/GuiMsgBox.h>
#include "components/ButtonComponent.h"
#include "utils/locale/LocaleHelper.h"

GuiMenuGamelistGameDeleteSelectionOptions::GuiMenuGamelistGameDeleteSelectionOptions(WindowManager& window, ISimpleGameListView& view, FileData& game)
  : GuiMenuBase(window, _("SELECT FILES TO DELETE"), nullptr)
  , mView(view)
  , mGame(game)
{
  if (mGame.IsGame())
  {
    // Footer
    String gameName(game.Name());
    gameName.Append(" (").Append(game.RomPath().Filename()).Append(')');
    SetFooter(_("GAME %s").Replace("%s", gameName.UpperCaseUTF8()));

    // Delete
    mGameFiles = AddMultiList<Path>(_("GAME FILES (ROM | DISK IMAGE)"), (int) Components::Delete, nullptr, GetGameFileEntries());
    mMedias = AddMultiList<Path>(_("MEDIA FILES"), (int) Components::Delete, nullptr, GetMediaEntries());
    mExtras = AddMultiList<Path>(_("CONFIGURATION AND PATCH FILES"), (int) Components::Delete, nullptr, GetExtraEntries());
    mSaves = AddMultiList<Path>(_("SAVE FILES"), (int) Components::Delete, nullptr, GetSaveEntries());

    mMenu.addButton(_("OK"), "", [this]
    {
      if(mGameFiles->getSelectedObjects().empty() && mSaves->getSelectedObjects().empty() && mExtras->getSelectedObjects().empty() &&
      mMedias->getSelectedObjects().empty())
      {
        mWindow.pushGui(
          (new GuiMsgBox(mWindow, _("No file selected,\nyou must at least choose one."), _("BACK"), TextAlignment::Left)));
        mMenu.SetDefaultButton(1);

      }
      else
          mWindow.pushGui((new GuiMsgBoxScroll(mWindow, _("DELETE SELECTED FILES, CONFIRM?"), ComputeMessage(), _("YES"), [this]
      {
        DeleteSelectedFiles();
      }, _("BACK"), {}, "", nullptr, TextAlignment::Left))->SetDefaultButton(1));
    });

    mMenu.addButton(_("BACK"), "", [this] { Close(); });
    mMenu.setCursorToButtons();
    mMenu.SetDefaultButton(1);
  }
}

std::vector<GuiMenuBase::ListEntry<Path>> GuiMenuGamelistGameDeleteSelectionOptions::GetGameFileEntries()
{
  std::vector<ListEntry<Path>> list;
  list.push_back({ mGame.RomPath().Filename(),  mGame.RomPath(), false });

  for (const auto& file : GameFilesUtils::GetGameSubFiles(mGame))
  {
    Path path = Path(file);
    list.push_back({ path.Filename(), path, false });
  }
  return list;
}

std::vector<GuiMenuBase::ListEntry<Path>> GuiMenuGamelistGameDeleteSelectionOptions::GetMediaEntries()
{
  std::vector<ListEntry<Path>> list;
   if (mGame.Metadata().Image().Exists())
    {
        list.push_back({ _("Image"), mGame.Metadata().Image(), false });
    }
    if (mGame.Metadata().Thumbnail().Exists())
    {
        list.push_back({ _("Thumbnail"), mGame.Metadata().Thumbnail(), false });
    }
    if (mGame.Metadata().Video().Exists())
    {
        list.push_back({ _("Video"), mGame.Metadata().Video(), false });
    }

  return list;
}

std::vector<GuiMenuBase::ListEntry<Path>> GuiMenuGamelistGameDeleteSelectionOptions::GetExtraEntries()
{
  std::vector<ListEntry<Path>> list;
  for (const auto& patch : GameFilesUtils::GetGameExtraFiles(mGame))
  {
    Path path = Path(patch);
      list.push_back({ path.Filename(), path, false });
  }
  return list;
}

std::vector<GuiMenuBase::ListEntry<Path>> GuiMenuGamelistGameDeleteSelectionOptions::GetSaveEntries()
{
  std::vector<ListEntry<Path>> list;
  //std::vector<ListEntry<Path>> mediaList;
  for (const auto& patch : GameFilesUtils::GetGameSaveFiles(mGame))
  {
    Path path = Path(patch);
    list.push_back({ path.Filename(), path, false });
  }
  return list;
}

void GuiMenuGamelistGameDeleteSelectionOptions::DeleteSelectedFiles()
{
  HashSet<String> list;
  HashSet<String> mediaList;

  bool mainGameDeleted = false;

  for(const Path& path : mGameFiles->getSelectedObjects())
  {
    if(path == mGame.RomPath())
    {
      mainGameDeleted = true;
    }
    list.insert(path.ToString());
  }

  for(const Path& path : mExtras->getSelectedObjects())
  {
    list.insert(path.ToString());
  }
  for(const Path& path : mSaves->getSelectedObjects())
  {
    list.insert(path.ToString());
  }

  for(const Path& path : mMedias->getSelectedObjects())
  {
    mediaList.insert(path.ToString());
  }

  GameFilesUtils::DeleteSelectedFiles(mGame, list, mediaList);
  if(mainGameDeleted)
  {
    mView.removeEntry(&mGame);
  }
  mWindow.CloseAll();
}

String GuiMenuGamelistGameDeleteSelectionOptions::ComputeMessage()
{
  String message = _("Game").Append(": ").Append(mGame.Name()).Append("\n\n");

  if(!mGameFiles->getSelectedObjects().empty())
  {
    message.Append(_("GAME FILES (ROM | DISK IMAGE)").Append('\n'));
    for (const Path& path: mGameFiles->getSelectedObjects())
    {
      message.Append(path.Filename()).Append('\n');
    }
  }
  if(!mExtras->getSelectedObjects().empty())
  {
    message.Append('\n').Append(_("CONFIGURATION AND PATCH FILES")).Append('\n');
    for (const Path& path: mExtras->getSelectedObjects())
    {
      message.Append(path.Filename()).Append('\n');
    }
  }

  if(!mSaves->getSelectedObjects().empty())
  {
    message.Append('\n').Append(_("SAVE FILES")).Append('\n');
    for (const Path& path: mSaves->getSelectedObjects())
    {
      message.Append(path.Filename()).Append('\n');
    }
  }

  if(!mMedias->getSelectedObjects().empty())
  {
    message.Append('\n').Append(_("MEDIA FILES")).Append('\n');
    for (const Path& path: mMedias->getSelectedObjects())
    {
      message.Append(path.Filename()).Append('\n');
    }
  }

  return message;
}


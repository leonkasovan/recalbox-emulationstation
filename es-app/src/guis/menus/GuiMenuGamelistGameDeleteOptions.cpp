#include <games/GameFilesUtils.h>
#include "GuiMenuGamelistGameDeleteOptions.h"
#include "GuiMenuTools.h"
#include <views/ViewController.h>
#include "components/ButtonComponent.h"
#include "GuiMenuGamelistGameDeleteSelectionOptions.h"

GuiMenuGamelistGameDeleteOptions::GuiMenuGamelistGameDeleteOptions(WindowManager& window, ISimpleGameListView& view, FileData& game)
  : GuiMenuBase(window, _("DELETE ALL FILES"), this)
  , mView(view)
  , mGame(game)
{
  if (mGame.IsGame())
  {
    // Footer
    String gameName(game.Name());
    gameName.Append(" (").Append(game.RomPath().Filename()).Append(')');
    SetFooter(_("GAME %s").Replace("%s", gameName.ToUpperCaseUTF8()));

    // Delete
    mGameFiles = GameFilesUtils::GetGameSubFiles(mGame);
    String fileCount = _N("%i file", "%i files", (int) mGameFiles.size() + 1).Replace("%i", String((int) mGameFiles.size() + 1));
    AddText(_("GAME FILES (ROM | DISK IMAGE)"), fileCount);

    mMediaFiles = GameFilesUtils::GetMediaFiles(mGame);
    if(!mMediaFiles.empty())
    {
      fileCount = _N("%i file", "%i files", (int) mMediaFiles.size()).Replace("%i",String((int) mMediaFiles.size()));
      AddText(_("MEDIA FILES"), fileCount);
    }

    mExtraFiles = GameFilesUtils::GetGameExtraFiles(mGame);
    if(!mExtraFiles.empty())
    {
      fileCount = _N("%i file", "%i files", (int) GameFilesUtils::GetGameExtraFiles(mGame).size()).Replace("%i", String((int) GameFilesUtils::GetGameExtraFiles(mGame).size()));
      AddText(_("CONFIGURATION AND PATCH FILES"), fileCount);
    }

    mSaveFiles = GameFilesUtils::GetGameSaveFiles(mGame);
    if(!mSaveFiles.empty())
    {
      fileCount = _N("%i file", "%i files", (int) mSaveFiles.size()).Replace("%i", String((int) mSaveFiles.size()));
      AddText(_("SAVE FILES"), fileCount);
    }

    AddSubMenu(_("ADVANCED DELETE"),  (int)Components::Advanced, "");

    mMenu.addButton(_("OK"), "", [this]
    {
      mWindow.pushGui((new GuiMsgBoxScroll(mWindow, _("DELETE ALL FILES, CONFIRM?"), ComputeMessage(), _("YES"), [this]
      {
        DeleteAllFiles();
      }, _("NO"), {}, "", nullptr, TextAlignment::Left))->SetDefaultButton(1));
    });

    mMenu.addButton(_("CANCEL"), "", [this] { Close(); });
    mMenu.setCursorToButtons();
    mMenu.SetDefaultButton(0);
  }
}

void GuiMenuGamelistGameDeleteOptions::DeleteAllFiles()
{
  GameFilesUtils::DeleteAllFiles(mGame);
  mView.removeEntry(&mGame);
  mWindow.CloseAll();
}

bool GuiMenuGamelistGameDeleteOptions::ProcessInput(const InputCompactEvent& event)
{

    return GuiMenuBase::ProcessInput(event);
}

void GuiMenuGamelistGameDeleteOptions::SubMenuSelected(int id)
{
  switch((Components)id)
  {
    case Components::Advanced:
      mWindow.pushGui(new GuiMenuGamelistGameDeleteSelectionOptions(mWindow, mView, mGame));
    case Components::Delete:
      break;
  }
}

String GuiMenuGamelistGameDeleteOptions::ComputeMessage()
{
  String message = _("Game").Append(": ").Append(mGame.Name()).Append('\n');
  message.Append(_("You are about to delete this files, confirm ?"));
  message.Append("\n\n");
  message.Append(mGame.RomPath().Filename()).Append('\n');

  for(const auto& path : mGameFiles)
    message.Append(Path(path).Filename()).Append('\n');

  if(!mExtraFiles.empty())
    for (const auto& path: mExtraFiles)
      message.Append(Path(path).Filename()).Append('\n');

  if(!mSaveFiles.empty())
    for (const auto& path: mSaveFiles)
      message.Append(Path(path).Filename()).Append('\n');

  if(!mMediaFiles.empty())
    for (const auto& path: mMediaFiles)
    {
      if (path.Contains("/media/images/"))
        message.Append(_("Image")).Append('\n');
      if (path.Contains("/media/thumbnails/"))
        message.Append(_("Thumbnail")).Append('\n');
      if (path.Contains("/media/videos/"))
        message.Append(_("Video")).Append('\n');
    }

  return message;
}
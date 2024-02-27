#include <games/GameFilesUtils.h>
#include "GuiMenuSoftpatchingLauncher.h"
#include "GuiMenuTools.h"
#include "games/FileData.h"
#include <views/ViewController.h>
#include "components/ButtonComponent.h"
#include "utils/locale/LocaleHelper.h"

GuiMenuSoftpatchingLauncher::GuiMenuSoftpatchingLauncher(WindowManager& window,
                                                         FileData& game,
                                                         std::vector<Path>&& patches,
                                                         int lastChoice,
                                                         ISoftPatchingNotifier* notifier)
  : GuiMenuBase(window, _("SOFTPATCHING"), nullptr)
  , mGame(game)
  , mPatches(patches)
{
    // Footer
    String gameName(game.Name());
    gameName.Append(" (").Append(game.RomPath().Filename()).Append(')');
    SetFooter(_("GAME %s").Replace("%s", gameName.UpperCaseUTF8()));

    // select
    mPaths = AddList<Path>(_("select a patch"),(int) Components::Patch, this,GetPatchesEntries(), "");

    mMenu.addButton(_("original"), "", [this, notifier] { Close(); if (notifier != nullptr) mGame.Metadata().SetLastPatch(Path("original")); notifier->SoftPathingDisabled(); });
    mMenu.addButton(_("patched"),  "", [this, notifier] { Close(); if (notifier != nullptr) notifier->SoftPatchingSelected(mPaths->getSelected()); });

    mMenu.setCursorToButtons();
    mMenu.SetDefaultButton(mGame.Metadata().LastPatch() == Path("original") ? 0 :  lastChoice);
    GuiMenuBase::SetFooter(game.Name());
}

std::vector<GuiMenuBase::ListEntry<Path>> GuiMenuSoftpatchingLauncher::GetPatchesEntries()
{
  std::vector<ListEntry<Path>> list;
  int nb = 1;
  unsigned long patchListSize = mPatches.size();
  for(auto& path : mPatches)
  {
    bool isDefault = patchListSize == 1 || (path == mGame.Metadata().LastPatch() && mGame.Metadata().LastPatch().Exists()) || nb == 1;
   String patchName = path.Directory() == mGame.RomPath().Directory() ? path.Filename() + " (auto)" : path.Filename();

    list.push_back({ patchName, path , isDefault });
    nb++;
  }
  return list;
}

void GuiMenuSoftpatchingLauncher::OptionListComponentChanged(int id, int index, const Path& value)
{
  (void)index;
  if ((Components)id == Components::Patch)
    mGame.Metadata().SetLastPatch(value);
}


















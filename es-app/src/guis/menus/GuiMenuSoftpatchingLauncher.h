
#pragma once

#include <guis/menus/GuiMenuBase.h>
#include <views/gamelist/ISimpleGameListView.h>
#include "emulators/run/GameLinkedData.h"
#include "views/ISoftPatchingNotifier.h"

class SystemManager;
class SystemData;
template<class T> class OptionListComponent;

class GuiMenuSoftpatchingLauncher : public GuiMenuBase
, private IOptionListComponent<Path>

{
  public:
    explicit GuiMenuSoftpatchingLauncher(WindowManager& window,
                                         FileData& game,
                                         std::vector<Path>&& patches,
                                         int lastChoice,
                                         ISoftPatchingNotifier* notifier);

  private:
    enum class Components
    {
      Patch,
    };

    //! Game reference
    FileData& mGame;
    std::vector<Path> mPatches;
    std::shared_ptr<OptionListComponent<Path>> mPaths;

    std::vector<ListEntry<Path>> GetPatchesEntries();

    void OptionListComponentChanged(int id, int index, const Path& value) override;
};




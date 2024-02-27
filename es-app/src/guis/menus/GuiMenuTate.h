#pragma once

#include <guis/menus/GuiMenuBase.h>
#include <systems/SystemData.h>
#include <components/MenuComponent.h>

template<typename T>
class OptionListComponent;
class SwitchComponent;

class GuiMenuTate : public GuiMenuBase
                               , private IOptionListComponent<RotationType>
                               , private ISwitchComponent
{
  public:
    //! Constructor
    GuiMenuTate(WindowManager& window, SystemManager& systemManager);

    //! Destructor
    ~GuiMenuTate() override;

  private:
    enum class Components
    {
      TateEnabled,
      TateGamesRotation,
      TateCompleteSystemRotation,
      TateOnly,
    };

    SystemManager& mSystemManager;

    //! Tate system enabled
    std::shared_ptr<SwitchComponent>                    mTateEnabled;
    //! Rotate System view ON/OFF
    std::shared_ptr<SwitchComponent>                    mRotateSystemView;
    //! System Rotation
    std::shared_ptr<OptionListComponent<RotationType>>           mSystemRotation;

    //! Original enabled value
    bool mOriginalTateEnabled;
    //! Original Games Rotation value
    RotationType mOriginalGamesRotation;
    //! Original Games Rotation value
    RotationType mOriginalSystemRotation;
    //! Original tate only valeu
    bool mOriginalTateOnly;


    //! Get rotations entries
    static std::vector<GuiMenuBase::ListEntry<RotationType>> GetRotationEntries(RotationType currentRotation);

    /*
     * IOptionListComponent<String> implementation
     */
    void OptionListComponentChanged(int id, int index, const RotationType& value) override;

    /*
     * ISwitchComponent implementation
     */
    void SwitchComponentChanged(int id, bool& status) override;
};

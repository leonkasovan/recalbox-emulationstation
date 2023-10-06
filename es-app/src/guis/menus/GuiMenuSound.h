//
// Created by bkg2k on 30/10/2020.
//
#pragma once

#include <guis/menus/GuiMenuBase.h>
#include <components/MenuComponent.h>
#include <components/SwitchComponent.h>
#include <components/OptionListComponent.h>
#include <audio/AudioMode.h>
#include <guis/GuiWaitLongExecution.h>
#include <audio/IAudioNotification.h>

class GuiMenuSound : public GuiMenuBase
                   , public ILongExecution<bool, String::List>
                   , public IGuiMenuBase
                   , public IAudioNotification
                   , private ISliderComponent
                   , private IOptionListComponent<String>
                   , private IOptionListComponent<AudioMode>
{
  public:
    /*!
     * @brief Default constructor
     * @param window Global window
     */
    explicit GuiMenuSound(WindowManager& window);

    ~GuiMenuSound();

    /*!
     * @brief Called once per frame. Override to implement your own drawings.
     * Call your base::Update() to ensure animation and childrens are updated properly
     * @param deltaTime Elapsed time from the previous frame, in millisecond
     */
    void Update(int deltaTime) override;

    /*!
     * @brief Refresh audio output menu list
     */
    void Refresh();
  private:
    enum class Components
    {
      Volume,
      AudioMode,
      Output,
      Pair,
    };

    //! Volume slider
    std::shared_ptr<SliderComponent> mVolume;
    //! Audio
    std::shared_ptr<OptionListComponent<AudioMode>> mAudioMode;
    //! Outputs
    std::shared_ptr<OptionListComponent<String>> mOutputList;

    //! Get Output List
    static std::vector<ListEntry<String>> GetOutputEntries();

    static std::vector<ListEntry<AudioMode>> GetAudioModeEntries();

    void OptionListComponentChanged(int id, int index, const String& value) override;
    void OptionListComponentChanged(int id, int index, const AudioMode& value) override;

    /*
     * ISliderComponent implementation
     */

    void SliderMoved(int id, float value) override;

    /*
     * IGuiMenuBase implementation
     */

    void SubMenuSelected(int id) override;

    void StartScanningDevices();

    String::List Execute(GuiWaitLongExecution<bool, String::List>&, const bool&) override;

    /*!
     * @brief Called when the device scan is complete
     * @param parameter Useless "in" parameter
     * @param result List of available devices
     */
    void Completed(const bool& parameter, const String::List& result) override;

    /*!
     * @brief Called from PulseAudioController whenever a sink is added or removed
     */
    void NotifyAudioChange() final;
};

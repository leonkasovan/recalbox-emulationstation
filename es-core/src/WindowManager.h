#pragma once

#include <utils/storage/Stack.h>
#include <guis/Gui.h>
#include <components/HelpComponent.h>
#include <components/ImageComponent.h>
#include <resources/Font.h>
#include <input/InputManager.h>
#include "guis/PopupType.h"
#include "osd/OSDManager.h"

// Forward declaration
class GuiInfoPopupBase;

class WindowManager
{
  public:
    /*!
     * @brief Default constructor
     */
    WindowManager();

    /*!
     * @brief Default destructor
     */
    virtual ~WindowManager();

    void pushGui(Gui* gui);

    void RemoveGui(Gui* gui);

    void displayMessage(const String& message, bool urgent = true);

    void displayScrollMessage(const String& title, const String& message, bool urgent = true);

    Gui* peekGui();

    void deleteAllGui();

    void textInput(const char* text);

    virtual bool ProcessInput(const InputCompactEvent& event);

    virtual void Update(int deltaTime);

    virtual void Render(Transform4x4f& transform);

    void RenderAll(bool halfLuminosity = false);

    bool Initialize(unsigned int width, unsigned int height, bool initRenderer = true);

    bool ReInitialize();

    static void Finalize();

    void normalizeNextUpdate() { mNormalizeNextUpdate = true; }

    [[nodiscard]] bool isSleeping() const { return mSleeping; }

    void renderHelpPromptsEarly(); // used to render HelpPrompts before a fade
    void UpdateHelp(bool force = false) { mHelp.UpdateHelps(force); }

    void DoSleep();

    void DoWake();

    /*!
     * @brief Add new regular info popup
     * @param message Message in popup
     * @param duration Duration in seconds
     * @param icon Icon type
     * @param first True to make this popup the first visible
     */
    void InfoPopupAddRegular(const String& message, int duration, PopupType icon, bool first);

    /*!
     * @brief Add new popup to dysplay list
     * @param infoPopup Popup to add to display list
     * @param first Force popup to take the first place
     *
     */
    void InfoPopupAdd(GuiInfoPopupBase* infoPopup, bool first = false);

    /*!
     * @brief Remove the info popup at the given index and restack
     * the whole list so that there no hole
     * @param infoPopup popup to remove
     */
    void InfoPopupRemove(GuiInfoPopupBase* infoPopup);

    /*!
     * @brief Check if the given popup is on screen
     * @param infoPopup popup to remove
     * @result True if the popup is on screen
     */
    bool InfoPopupIsShown(GuiInfoPopupBase* infoPopup);

    /*!
     * @brief Close all gui
     */
    void CloseAll();

    /*!
     * @brief Check if the window has
     * @return
     */
    [[nodiscard]] bool HasGui() const { return !mGuiStack.Empty(); }

    /*!
     * @brief Check if the given UI is on top of the screen
     * @return True if the given UI is the first visible
     */
    virtual bool AmIOnTopOfScreen(const Gui* ui) const
    {
      if (!mGuiStack.Empty())
        if (mGuiStack.Peek() == ui) return true;
      return false;
    }

    /*!
     * @brief Update the help system.
     * @return True if the update system has been set, false otherwise
     */
    virtual bool UpdateHelpSystem();

    /*!
     * @brief Check if at least a GUI on the stack is claiming not to be disturbed!
     * @return
     */
    [[nodiscard]] virtual bool DoNotDisturb() const = 0;

    [[nodiscard]] bool HasWindowInDoNotDisturb() const
    {
      for(int i = mGuiStack.Count(); --i >= 0;)
        if (mGuiStack[i]->DoNotDisturb()) return true;
      return false;
    }

    /*!
     * @brief Rotate
     * @return
     */
    virtual void Rotate(RotationType rotation) = 0;

    //! Get OSD manager
    OSDManager& OSD() { return mOSD; }

    /*!
     * @brief Set display visibility. Usually set to false while loading a single system in autorun mode
     * @param visible Visibility
     */
    void SetDisplayEnabled(bool enabled) { mDisplayEnabled = enabled; }

  private:
    //! Maximum popup info
    static constexpr int sMaxInfoPopups = 10;
    //! Maximum displayable popup info
    static constexpr int sMaxDisplayableInfoPopups = 4;

    // Returns true if at least one component on the stack is processing
    bool isProcessing();

    void renderScreenSaver();

    static void exitScreenSaver();

    static bool KonamiCode(const InputCompactEvent& input);

    //! OSD Manager
    OSDManager mOSD;

    HelpComponent mHelp;
    ImageComponent mBackgroundOverlay;
    Array<GuiInfoPopupBase*> mInfoPopups;

    Stack<Gui*> mGuiStack;
    String::List mMessages;
    String::List mScrollMessages;
    String::List mScrollTitle;

    std::vector<std::shared_ptr<Font> > mDefaultFonts;

    int mAverageDeltaTime;
    unsigned int mTimeSinceLastInput;

    bool mNormalizeNextUpdate;
    bool mSleeping;
    bool mRenderedHelpPrompts;

    //! Screen refresh enabled?
    bool mDisplayEnabled;

    /*!
     * @brief Delete GUI pending for deletion
     */
    void deleteClosePendingGui();

    /*!
     * @brief Ensure there is at least one room to add a new infopopup
     */
    void InfoPopupsShrink();

    /*!
     * @brief Remove the info popup at the given index and restack
     * the whole list so that there no hole
     * @param index
     */
    void InfoPopupsRemove(int index);

    /*!
     * @brief Update info popup & timers
     * Delete on timeout
     * @param delta elapsed milliseconds
     */
    void InfoPopupsUpdate(int delta);

    /*!
     * @brief Display info popups
     */
    void InfoPopupsDisplay(Transform4x4f& transform);

    /*!
     * @brief Set all popup target position
     */
    void InfoPopupRetarget();

    /*!
     * @brief Check and display battery icon
     */
    static void DisplayBatteryState();
};

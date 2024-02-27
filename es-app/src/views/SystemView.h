#pragma once

#include "components/base/Component.h"
#include "components/TextComponent.h"
#include "components/IList.h"
#include "themes/ThemeExtras.h"
#include "IProgressInterface.h"
#include "utils/os/system/Thread.h"

class SystemManager;
class SystemData;
class AnimatedImageComponent;

enum class CarouselType : unsigned int
{
	Horizontal    = 0,
	Vertical      = 1,
	VerticalWheel = 2
};

struct SystemViewData
{
	std::shared_ptr<Component> logo;
	std::shared_ptr<Component> logotext;
	std::shared_ptr<ThemeExtras> backgroundExtras;
};

struct SystemViewCarousel
{
	CarouselType type;
	Vector2f pos;
	Vector2f size;
	Vector2f origin;
	float logoScale;
	float logoRotation;
	Vector2f logoRotationOrigin;
  TextAlignment logoAlignment;
	unsigned int color;
	int maxLogoCount; // number of logos shown on the carousel
	Vector2f logoSize;
	float zIndex;
};

struct SystemGameCount
{
  int VisibleGames;
  int Favorites;
  int Hidden;
};

class SystemView : public IList<SystemViewData, SystemData*>
                 , public Thread
                 , public ISyncMessageReceiver<SystemGameCount>
{
  public:
    SystemView(WindowManager& window, SystemManager& systemManager);

    //! Destructor
    ~SystemView() override { Thread::Stop(); }

    void onShow() override {	mShowing = true; }
    void onHide() override {	mShowing = false; }

    void goToSystem(SystemData* system, bool animate);

    bool ProcessInput(const InputCompactEvent& event) override;
    void Update(int deltaTime) override;
    void Render(const Transform4x4f& parentTrans) override;

    void onThemeChanged(const ThemeData& theme);

    bool getHelpPrompts(Help& help) override;
    void ApplyHelpStyle() override;
    void populate();

    /*!
     * @brief Set progress interface used ot notify first system loading
     * @param interface Progress interface
     */
    void SetProgressInterface(IProgressInterface* interface) { mProgressInterface = interface; }

    /*!
     * @brief update view with only visible games systems
     */
    void manageSystemsList();

    /*!
    * @brief Add a system to system view
    * @param System data
    */
    void addSystem(SystemData* system);

    /*!
     * @brief remove a system from system view
     * @param System data
     */
    void removeSystem(SystemData* system);

    /*!
     * @brief Lookup a system by name in the system list
     * @param name System name to look for
     * @return True if the system exists in the systemlist, false otherwise
     */
    SystemData* LookupSystemByName(const String& name);

    SystemData* Prev();
    void RemoveCurrentSystem();
    void Sort();
    void onCursorChanged(const CursorState& state) override;

  private:
    //! SystemManager instance
    SystemManager& mSystemManager;

    SystemViewCarousel mCarousel;
    TextComponent mSystemInfo;

    //! IProgressInterface to notify loading screen we're loading systems (and it takes time...)
    IProgressInterface* mProgressInterface;

    //! System information synchronizer
    SyncMessageSender<SystemGameCount> mSender;
    //! System from witch to extract data (game count, favorites, hidden)
    const SystemData* mSystemFromWitchToExtractData;
    //! System locker
    Mutex mSystemLocker;
    //! Fetch info thread signal
    Signal mSystemSignal;

    // unit is list index
    SystemData* mCurrentSystem;
    float mCamOffset;
    float mExtrasCamOffset;
    float mExtrasFadeOpacity;
    bool mViewNeedsReload;
    bool mShowing;
    bool mLaunchKodi;

    /*!
     * @brief Set next system from witch to extract game information
     * @param system
     */
    void SetNextSystem(const SystemData* system);

    /*
     * Thread implementation
     */

    //! Break the system info fetching thread
    void Break() override { mSystemSignal.Fire(); }

    //! Implementation of system fetching info
    void Run() override;

    /*
     * ISyncMessageReceiver<SystemGameCount> implementation
     */

    //! Receive enf-of-fetch info for the very last system the user is on
    void ReceiveSyncMessage(const SystemGameCount& data) override;

    /*
     * Legacy
     */

    void getViewElements(const ThemeData& theme);
    void getDefaultElements();
    void getCarouselFromTheme(const ThemeElement* elem);

    void renderCarousel(const Transform4x4f& parentTrans);
    void renderExtras(const Transform4x4f& parentTrans, float lower, float upper);
    void renderInfoBar(const Transform4x4f& trans);
    void renderFade(const Transform4x4f& trans);
};

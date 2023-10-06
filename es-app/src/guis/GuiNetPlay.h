//
// Created by xizor on 20/05/18.
//
#pragma once

#include <components/ComponentList.h>
#include "guis/Gui.h"
#include "components/NinePatchComponent.h"
#include "components/ComponentGrid.h"
#include <recalbox/RecalboxSystem.h>
#include <themes/MenuThemeData.h>
#include <components/BusyComponent.h>
#include <utils/os/system/Thread.h>
#include <utils/sync/SyncMessageSender.h>

class TextComponent;
class ButtonComponent;
class SystemManager;

#define TITLE_VERT_PADDING (Renderer::Instance().DisplayHeightAsFloat()*0.0637f)

class LobbyGame
{
  public:
    FileData*   mGame;
    String mFormattedName;
    String mGameName;
    String mGameCRC;
    String mCoreLongName;
    String mCoreShortName;
    String mCoreVersion;
    String mUserName;
    String mFrontEnd;
    String mRetroarchVersion;
    String mCountry;
    String mIp;
    String mMitmIp;
    int         mPort;
    int         mMitmPort;
    int         mHostMethod;
    int         mPingTimeInMs;
    bool        mNeedPlayerPassword;
    bool        mNeedViewerPassword;
    bool        mIsRecalbox;

    LobbyGame()
      : mGame(nullptr),
        mPort(0),
        mMitmPort(0),
        mHostMethod(0),
        mPingTimeInMs(0),
        mNeedPlayerPassword(false),
        mNeedViewerPassword(false),
        mIsRecalbox(false)
    {
    }
};

enum class GuiNetPlayMessageType
{
  LobbyLoaded,
  Ping,
};

class GuiNetPlay: public Gui
                , private Thread
                , private ISyncMessageReceiver<GuiNetPlayMessageType>
{
  public:
    explicit GuiNetPlay(WindowManager&window, SystemManager& systemManager);

    ~GuiNetPlay() override;

    inline void addRow(const ComponentListRow& row, bool setCursorHere = false, bool updateGeometry = true)
    {
      mList->addRow(row, setCursorHere, updateGeometry);
      if (updateGeometry) updateSize();
    }

    void populateGrid();

    void populateGridMeta(int i);

    void launch();

    [[nodiscard]] float getButtonGridHeight() const;

    void updateSize();

    bool ProcessInput(const InputCompactEvent& event) override;

    bool getHelpPrompts(Help& help) override;

    void onSizeChanged() override;

    void Update(int deltaTime) override;

    void Render(const Transform4x4f& parentTrans) override;

  private:
    //! Immutable core information
    struct CoreInfo
    {
      private:
        //! Long name (i.e. "MAME 2003-Plus")
        String mLongName;
        //! Short name (i.e. "mame2003+")
        String mShortName;
        //! Version
        String mVersion;

      public:
        CoreInfo(const String& longName, const String& shortName, const String& version)
          : mLongName(longName)
          , mShortName(shortName)
          , mVersion(version)
        {
        }

        CoreInfo() = default;

        //! Long name
        [[nodiscard]] const String& LongName() const { return mLongName; }
        //! Short name
        [[nodiscard]] const String& ShortName() const { return mShortName; }
        //! Version
        [[nodiscard]] const String& Version() const { return mVersion; }
        //! Empty?
        [[nodiscard]] bool Empty() const { return mLongName.empty(); }
    };

    /*
     * Thread Implementation
     */

    /*!
     * @brief Main thread routine
     */
    void Run() override;

    /*
     * Synchronous event
     */

    /*!
     * @brief Receive SDL event from the main thread
     * @param event SDL event
     */
    void ReceiveSyncMessage(const GuiNetPlayMessageType& event) override;

    /*!
     * @brief Look for a game in all gamelist available
     * @param game game or hash
     * @return FileData of the game is found, otherwise nullptr
     */
    FileData* FindGame(const String& game);

    /*!
     * @brief Get the playable games from the lobby and fill the list
     */
    void ParseLobby();

    /*!
     * @brief Load core map from recalbox so that we can translate
     * long core names to short core names
     */
    void LoadCoreMap();

    /*!
     * @brief Get formatted name w/ icons
     * @param game Gamer to get data from
     * @return Formatted game name
     */
    String GetFormattedName(const LobbyGame& game);

    /*!
     * @brief Get core information
     * @param name Long name to lookup
     * @return Core information
     */
    const CoreInfo& GetCoreInfo(const String& name);

    //! SystemManager instance
    SystemManager& mSystemManager;

    //! Core information list
    std::vector<CoreInfo> mCoreList;

    /*!
     * @brief Netplayable Game list
     */
    std::vector<LobbyGame> mLobbyList;

    bool mLobbyLoaded;

    SyncMessageSender<GuiNetPlayMessageType> mSender;

    NinePatchComponent mBackground;
    BusyComponent mBusyAnim;
    ComponentGrid mGrid;
    std::shared_ptr<MenuTheme> mMenuTheme;

    std::shared_ptr<ComponentGrid> mGridMeta;
    std::shared_ptr<ComponentGrid> mGridMetaRight;
    std::shared_ptr<ComponentGrid> mButtonGrid;
    std::vector<std::shared_ptr<ButtonComponent> > mButtons;
    std::shared_ptr<TextComponent> mTitle;
    std::shared_ptr<ComponentList> mList;
    std::shared_ptr<TextComponent> mMetaTextLblUsername;
    std::shared_ptr<TextComponent> mMetaTextUsername;
    std::shared_ptr<TextComponent> mMetaTextLblCountry;
    std::shared_ptr<TextComponent> mMetaTextCountry;
    std::shared_ptr<TextComponent> mMetaTextLblRomHash;
    std::shared_ptr<TextComponent> mMetaTextRomHash;
    std::shared_ptr<TextComponent> mMetaTextLblRomFile;
    std::shared_ptr<TextComponent> mMetaTextRomFile;
    std::shared_ptr<TextComponent> mMetaTextLblCore;
    std::shared_ptr<TextComponent> mMetaTextCore;
    std::shared_ptr<TextComponent> mMetaTextLblCoreVer;
    std::shared_ptr<TextComponent> mMetaTextCoreVer;
    std::shared_ptr<TextComponent> mMetaTextLblLatency;
    std::shared_ptr<TextComponent> mMetaTextLatency;
    std::shared_ptr<TextComponent> mMetaTextLblRAVer;
    std::shared_ptr<TextComponent> mMetaTextRAVer;
    std::shared_ptr<TextComponent> mMetaTextLblHostArch;
    std::shared_ptr<TextComponent> mMetaTextHostArch;
    std::shared_ptr<TextComponent> mMetaTextLblCanJoin;
    std::shared_ptr<TextComponent> mMetaTextCanJoin;
};

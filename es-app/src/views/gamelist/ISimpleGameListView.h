#pragma once

#include <guis/Gui.h>
#include <components/ImageComponent.h>
#include <components/TextComponent.h>
#include "themes/ThemeExtras.h"
#include "IArcadeGamelistInterface.h"
#include "SlowDataInformation.h"
#include <systems/SystemData.h>

class SystemManager;

enum class FileChangeType
{
  Added,
  Run,
  MetadataChanged,
  Removed,
  Sorted,
  DisplayUpdated,
};

class ISimpleGameListView : public Gui
{
  public:
    enum class Change
    {
      Resort, //!< New sorting required
      Update, //!< Update lists
    };

    ISimpleGameListView(WindowManager& window, SystemManager& systemManager, SystemData& system);

    ~ISimpleGameListView() override = default;

    /*!
     * @brief Must be called right after the constructor
     */
    virtual void Initialize() = 0;

    /*!
     * @brief Get Arcade interface
     * @return Arcade interface or nullptr
     */
    virtual IArcadeGamelistInterface* getArcadeInterface()
    { return nullptr; }

    /*!
     * @brief Called when a major change occurs on the system
     * @param change Change type
     */
    virtual void onChanged(Change change);

    /*!
     * @brief Refresh name & properties of the given item
     */
    virtual void RefreshItem(FileData* game) = 0;

    // Called whenever the theme changes.
    virtual void onThemeChanged(const ThemeData& theme);

    bool ProcessInput(const InputCompactEvent& event) override;

    bool getHelpPrompts(Help& help) override;

    virtual std::vector<unsigned int> getAvailableLetters();

    virtual void jumpToLetter(unsigned int unicode);

    virtual void jumpToNextLetter(bool forward);

    [[nodiscard]] const SystemData& System() const { return mSystem; }

    void ApplyHelpStyle() override;

    void updateInfoPanel();

    void setTheme(const ThemeData& theme);

    [[nodiscard]] virtual int Count() const = 0;

    [[nodiscard]] virtual bool IsEmpty() const = 0;

    virtual FileData* getCursor() = 0;

    virtual FileData* getDataAt(int i) = 0;

    virtual const String& getCursorText() = 0;

    virtual const String& getCursorTextAt(int i) = 0;

    virtual void setCursorStack(FileData*) = 0;

    virtual void setCursor(FileData*) = 0;

    virtual int getCursorIndex() = 0;

    virtual int getCursorIndexMax() = 0;

    virtual void setCursorIndex(int) = 0;

    virtual void OnGameSelected() = 0;

    virtual void removeEntry(FileData* fileData) = 0;

    [[nodiscard]] virtual const char* getName() const = 0;

    virtual void DoUpdateGameInformation(bool update) = 0;

    virtual void populateList(const FolderData& folder) = 0;

    virtual void refreshList() = 0;

    virtual FileData::List getFileDataList() = 0;

    /*!
     * @brief Get available regions from the current game list
     * @return Region list (may be empty)
     */
    virtual Regions::List AvailableRegionsInGames() = 0;

    /*!
     * @brief Check if the current game (under cursor) has p2k file available
     * @return True if at least one p2k file has been found
     */
    [[nodiscard]] virtual bool HasCurrentGameP2K() const = 0;

    /*!
     * @brief Gamelist may update thos information if required
     * @param info
     */
    virtual void UpdateSlowData(const SlowDataInformation& info) = 0;

  protected:
    virtual void launch(FileData* game) = 0;

    virtual void clean() = 0;

    virtual FileData* getEmptyListItem() = 0;

    SystemData& mSystem;
    const ThemeData* mTheme;

    //! SystemManager instance
    SystemManager& mSystemManager;

    TextComponent mHeaderText;
    ImageComponent mHeaderImage;
    ImageComponent mBackground;

    ThemeExtras mThemeExtras;

    std::stack<FolderData*> mCursorStack;

  private:

    bool mVerticalMove;

    bool IsFavoriteSystem()
    { return mSystem.IsFavorite(); }
};

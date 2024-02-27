//
// Created by bkg2k on 31/05/23.
//
#pragma once

#include <views/gamelist/DetailedGameListView.h>
#include <systems/arcade/ArcadeTupple.h>
#include "IArcadeGamelistInterface.h"

class ArcadeGameListView : public DetailedGameListView
                         , public IArcadeGamelistInterface
{
  public:
    /*!
     * @brief Constructor
     * @param window Windows manager reference
     * @param systemManager System manager reference
     * @param system Target system
     */
    ArcadeGameListView(WindowManager& window, SystemManager& systemManager, SystemData& system);

  private:
    //! Linked arcade/game structure + clone list
    struct ParentTupple : public ArcadeTupple
    {
      ArcadeTuppleList* mCloneList;
      bool mFolded;   //!< True if the parent is folded
      //! Constructor
      ParentTupple(const ArcadeGame* arcade, FileData* game, bool folded) : ArcadeTupple(arcade, game), mCloneList(nullptr), mFolded(folded) {}
      //! Copy constructor
      ParentTupple(const ParentTupple& source)
        : ArcadeTupple(source)
        , mCloneList(source.mCloneList != nullptr ? new ArcadeTuppleList(*source.mCloneList) : nullptr)
        , mFolded(source.mFolded)
        {}
      //! Move constructor
      ParentTupple(ParentTupple&& source) noexcept
        : ArcadeTupple(source)
        , mCloneList(source.mCloneList)
        , mFolded(source.mFolded)
      {
        source.mCloneList = nullptr;
      }
      //! Destructor
      ~ParentTupple() { delete mCloneList; }
      //! Inject clones
      void AddClones(ArcadeTuppleList* list) { mCloneList = list; }
    };

    typedef std::vector<ParentTupple> ParentTuppleList;

    //! List
    ParentTuppleList mGameList;

    //! Last database to use
    const ArcadeDatabase* mDatabase;
    //! Default emulator for the current folder
    String mDefaultEmulator;
    //! Default core for the current folder
    String mDefaultCore;

    /*!
     * @brief Get Arcade interface
     * @return Arcade interface
     */
    IArcadeGamelistInterface* getArcadeInterface() override { return this; }

    /*!
     * @brief Refresh name & properties of the given item
     */
    void RefreshItem(FileData* game);

    /*!
     * @brief Rebuild the gamelist - regenerate internal structure
     * @param folder
     */
    void populateList(const FolderData& folder) final;

    /*!
     * @brief Rebuild the gamelist using internal structures
     */
    void BuildList();

    /*!
     * @brief Get arcade specific icons
     * @param item Item to get icon for
     * @return Icon
     */
    String getArcadeItemIcon(const ArcadeTupple& game);

    /*!
     * @brief Sort items to render special arcade list
     * @param items Unsorted games
     * @param database Arcade database
     * @param ascending True for an ascending sort, false for a descending sort
     * @return Sorted items
     */
    void BuildAndSortArcadeGames(FileData::List& items, FileSorts::ComparerArcade comparer, bool ascending);

    /*!
     * @brief Add a complete sorted lists of arcade tupple to the gamelist
     * @param categoryLists One or more arcade tupple list to sort and add
     * @param comparer Sort comparer
     * @param ascending True = ascending sort
     */
    void AddSortedCategories(const std::vector<ParentTuppleList*>& categoryLists, FileSorts::ComparerArcade comparer, bool ascending);

    /*!
     * @brief Get display name of the given game
     * @param database Arcade Database
     * @param game Game
     * @return Final display name
     */
    static String GetDisplayName(const ArcadeTupple& game);

    /*!
     * @brief Get description of the given game
     * @param game Game
     * @return Description
     */
    String GetDescription(FileData& game) override;

    /*!
     * @brief Get display name of the given game
     * @param database Arcade Database
     * @param game Game
     * @return Final display name
     */
    String GetDisplayName(FileData& game) override;

    /*!
     * @brief Get display name of the given game w/icons
     * @param database Arcade Database
     * @param game Game
     * @return Final display name
     */
    String GetIconifiedDisplayName(const ArcadeTupple& game);

    /*!
     * @brief Get available regions from the given listt
     * @return Region list (may be empty)
     */
    static Regions::List AvailableRegionsInGames(ParentTuppleList& list);

    /*!
     * @brief Jump to the first game starting with the given unicode char, from the cursor
     * @param unicode Unicode char to lookup
     */
    void jumpToLetter(unsigned int unicode) override;

    /*!
     * @brief Jump to next/previous letter forward or backward
     * @param forward True to jump forward, false to jump backward
     */
    void jumpToNextLetter(bool forward) override;

    /*!
     * @brief Lookup the arcade tupple attached to the given
     * @param item game
     * @return ArcadeTupple (or empty ArcadeTupple if the lookup fails!)
     */
    const ArcadeTupple& Lookup(const FileData& item);

    /*!
     * @brief Lookup display name of the given item
     * @param item game
     * @return Display name
     */
    String LookupDisplayName(const FileData& item);

    //! Fold all parents
    void FoldAll();

    //! Unfold all parents
    void UnfoldAll();

    //! Fold the current parent or the current clone's parent
    void Fold();

    //! Unfold the current parent
    void Unfold();

    /*
     * Component overrides
     */

    /*!
     * @brief Process extended actions
     * @param event Event to process
     * @return true if the event has been processed, false otherwise
     */
    bool ProcessInput(const InputCompactEvent& event) override;

    /*
     * IArcadeGamelistInterface implementation
     */

    [[nodiscard]] const SystemData& GetAttachedSystem() const override { return mSystem; };

    /*!
     * @brief Check if the Arcade game list has a valid database for the current emulator/core
     * @return
     */
    [[nodiscard]] bool HasValidDatabase() const override { return mDatabase != nullptr; }

    /*!
     * @brief Get driver list (arcade view only)
     * @return Driver list
     */
    [[nodiscard]] std::vector<ArcadeDatabase::Manufacturer> GetManufacturerList() const override;

    /*!
     * @brief Get the current emulator name for the current folder
     * @return Emulator name
     */
    [[nodiscard]] String GetCurrentEmulatorName() const override { return mDefaultEmulator; }

    /*!
     * @brief Get the current core name for the current folder
     * @return Core name
     */
    [[nodiscard]] String GetCurrentCoreName() const override { return mDefaultCore; }

    /*!
     * @brief Get the number of games attached to the given driver, in the current gamalist
     * @param driverIndex Driver index
     * @return Game count
     */
    [[nodiscard]] int GetGameCountForManufacturer(int driverIndex) const override;

    /*!
     * @brief Check if the current database can be filtered
     * @return
     */
    [[nodiscard]] bool CanBeFiltered() const override { return mDatabase != nullptr && mDatabase->CanBeFiltered(); }

    /*!
     * @brief Check if at least one manufacturer in the Limited manufacturers holder match one of the manufacturer in the given set
     * @param manufacturerSet manufacturer set
     * @param manufacturers manufacturer holders
     * @return True if at least one match has been found, false otherwise
     */
    static bool HasMatchingManufacturer(const HashSet<int>& manufacturerSet, const ArcadeGame::LimitedManufacturerHolder& manufacturers);
};

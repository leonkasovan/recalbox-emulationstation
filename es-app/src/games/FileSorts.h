#pragma once

#include <vector>
#include "games/FileData.h"
#include <systems/arcade/ArcadeTupple.h>

class FileSorts
{
  public:
    //! Arcade comparer for Wuick sorts
    typedef int (*ComparerArcade)(const ArcadeTupple& a, const ArcadeTupple& b);

    //! Specialized sort sets
    enum class SortSets
    {
      SingleSystem, //!< One console, one system
      MultiSystem,  //!< Virtual multi systems
      Arcade,       //!< Arcade
    };

    //! Available sorts
    enum class Sorts
    {
        FileNameAscending,    //!< By filename, A-Z
        FileNameDescending,   //!< By filename, Z-A
        SystemAscending,      //!< By system then by filename, A-Z
        SystemDescending,     //!< By system then by filename, Z-A
        RatingAscending,      //!< By rating then by filename, 0-9
        RatingDescending,     //!< By rating then by filename, 9-0
        TimesPlayedAscending, //!< By times played then by filename, A-Z
        TimesPlayedDescending,//!< By times played then by filename, Z-A
        LastPlayedAscending,  //!< By last-played datetime then by filename, 2000-0
        LastPlayedDescending, //!< By last-played datetime then by filename, 0-2000
        PlayersAscending,     //!< By number of players then by filename, 1-2
        PlayersDescending,    //!< By number of players then by filename, 2-1
        DeveloperAscending,   //!< By developer then by filename, A-Z
        DeveloperDescending,  //!< By developer then by filename, Z-A
        PublisherAscending,   //!< By publisher then by filename, A-Z
        PublisherDescending,  //!< By publisher then by filename, Z-A
        GenreAscending,       //!< By normalized genre (genreid) then by filename, EnumFirst-EnumLast
        GenreDescending,      //!< By normalized genre (genreid) then by filename, EnumLast-EnumFirst
        ReleaseDateAscending, //!< By release date then by filename, 0-9
        ReleaseDateDescending,//!< By release date then by filename, 9-0
    };

  private:
    //! Initialize Sort tables
    static bool Initialize();
    //! Initialization holder for static auto-initialization
    static bool sInitialized;

    //! Saved flag: use database name for arcade games
    static bool sUseDatabaseNames;
    //! Saved flags: ise filename for all games
    static bool sUseFileName;

    //! Sort record
    struct SortType
    {
      String mDescription;
      FileData::Comparer mComparer;
      ComparerArcade mComparerArcade;
      Sorts mSort;
      bool mAscending;

      SortType(Sorts sort, FileData::Comparer sortFunction, ComparerArcade sortFunctionArcade, bool sortAscending, const String & sortDescription)
        : mDescription(sortDescription),
          mComparer(sortFunction),
          mComparerArcade(sortFunctionArcade),
          mSort(sort),
          mAscending(sortAscending)
      {
      }
    };

    //! All normal sorts
    static std::vector<SortType> sAllSorts;

    /*!
     * @brief Case-insensitive compare of both UTF-8 string
     * @param a Left string
     * @param b Right string
     * @return Quicksort compare value: negative, 0 or positive
     */
    static int unicodeCompareUppercase(const String& a, const String& b);

    /*!
     * @brief Compare top level type Folder/game
     * @param fd1 Left File/Folder
     * @param fd2 Right File/Folder
     * @return Quicksort compare value: negative, 0 or positive
     */
    static int compareFoldersAndGames(const FileData& fd1, const FileData& fd2);

    /*!
     * Highly optimized Quicksort, inspired from original Delphi 7 code
     * @param low Lowest element
     * @param high Highest element
     * @param comparer Compare method
     */
    static void QuickSortAscendingArcade(ArcadeTupplePointerList& items, int low, int high, ComparerArcade comparer);

    /*!
     * Highly optimized Quicksort, inspired from original Delphi 7 code
     * @param low Lowest element
     * @param high Highest element
     * @param comparer Compare method
     */
    static void QuickSortDescendingArcade(ArcadeTupplePointerList& items, int low, int high, ComparerArcade comparer);

  public:
    #define DeclareSortMethodPrototype(x) static int x(const FileData& file1, const FileData& file2);
    #define DeclareSortMethodPrototypeArcade(x) static int x(const ArcadeTupple& file1, const ArcadeTupple& file2);
    #define ImplementSortMethod(x) int FileSorts::x(const FileData& file1, const FileData& file2)
    #define ImplementSortMethodArcade(name, nc) int FileSorts::name(const ArcadeTupple& at1, const ArcadeTupple& at2) { return nc(*at1.mGame, *at2.mGame); }

    DeclareSortMethodPrototype(compareSystemName)
    DeclareSortMethodPrototype(compareFileName)
    DeclareSortMethodPrototype(compareRating)
    DeclareSortMethodPrototype(compareTimesPlayed)
    DeclareSortMethodPrototype(compareLastPlayed)
    DeclareSortMethodPrototype(compareNumberPlayers)
    DeclareSortMethodPrototype(compareDevelopper)
    DeclareSortMethodPrototype(comparePublisher)
    DeclareSortMethodPrototype(compareGenre)
    DeclareSortMethodPrototype(compareReleaseDate)

    DeclareSortMethodPrototypeArcade(compareSystemNameArcade)
    DeclareSortMethodPrototypeArcade(compareFileNameArcade)
    DeclareSortMethodPrototypeArcade(compareRatingArcade)
    DeclareSortMethodPrototypeArcade(compareTimesPlayedArcade)
    DeclareSortMethodPrototypeArcade(compareLastPlayedArcade)
    DeclareSortMethodPrototypeArcade(compareNumberPlayersArcade)
    DeclareSortMethodPrototypeArcade(compareDevelopperArcade)
    DeclareSortMethodPrototypeArcade(comparePublisherArcade)
    DeclareSortMethodPrototypeArcade(compareGenreArcade)
    DeclareSortMethodPrototypeArcade(compareReleaseDateArcade)

    /*!
     * @brief Get available sorts for a single system
     * @param multisystem or single system?
     * @return Sort list
     */
    static const std::vector<Sorts>& AvailableSorts(SortSets set);

    /*!
     * @brief Get sort description
     * @param sort Sort to get name from
     * @return Sort name
     */
    static const String& Name(Sorts sort);

    /*!
     * @brief Return whether the given sort is an ascending sort or not
     * @param sort Sort to check
     * @return True if the sort is ascending
     */
    static bool IsAscending(Sorts sort);

    /*!
     * @brief Get sort compare method
     * @param sort Sort to get method from
     * @return Sort name
     */
    static FileData::Comparer Comparer(Sorts sort);

    /*!
     * @brief Get sort compare method for arcade systems
     * @param sort Sort to get method from
     * @return Sort name
     */
    static ComparerArcade ComparerArcadeFromSort(Sorts sort);

    /*!
     * @brief Clamp the given sort in the available sorts from the given set
     * @param sort sort to clamp
     * @param set Sort set to check if the sort is available in
     * @return Either the given sort or the first sort available in the given set
     */
    static FileSorts::Sorts Clamp(Sorts sort, SortSets set);

    /*!
     * Quick sort items in the given list usign the given arcade database.
     * @param items Items to sort
     * @param database Arcade database
     * @param comparer Comparison function
     * @param ascending True for ascending sort, false for descending.
     */
    static void SortArcade(ArcadeTupplePointerList& items, ComparerArcade comparer, bool ascending);

};

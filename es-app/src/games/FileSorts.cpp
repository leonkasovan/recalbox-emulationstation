#include "utils/locale/LocaleHelper.h"
#include "FileSorts.h"
#include "systems/SystemData.h"

std::vector<FileSorts::SortType> FileSorts::sAllSorts;
bool FileSorts::sInitialized = false;
bool FileSorts::sUseDatabaseNames = false;
bool FileSorts::sUseFileName = false;

bool FileSorts::Initialize()
{
  if (!sInitialized)
  {
    sAllSorts.push_back(SortType(Sorts::FileNameAscending    , &compareFileName     , &compareFileNameArcade     , true , "\uF15d " + _("FILENAME")));
    sAllSorts.push_back(SortType(Sorts::FileNameDescending   , &compareFileName     , &compareFileNameArcade     , false, "\uF15e " + _("FILENAME")));
    sAllSorts.push_back(SortType(Sorts::RatingAscending      , &compareRating       , &compareRatingArcade       , true , "\uF165 " + _("RATING")));
    sAllSorts.push_back(SortType(Sorts::RatingDescending     , &compareRating       , &compareRatingArcade       , false, "\uF164 " + _("RATING")));
    sAllSorts.push_back(SortType(Sorts::TimesPlayedAscending , &compareTimesPlayed  , &compareTimesPlayedArcade  , true , "\uF160 " + _("TIMES PLAYED")));
    sAllSorts.push_back(SortType(Sorts::TimesPlayedDescending, &compareTimesPlayed  , &compareTimesPlayedArcade  , false, "\uF161 " + _("TIMES PLAYED")));
    sAllSorts.push_back(SortType(Sorts::LastPlayedAscending  , &compareLastPlayed   , &compareLastPlayedArcade   , true , "\uF160 " + _("LAST PLAYED")));
    sAllSorts.push_back(SortType(Sorts::LastPlayedDescending , &compareLastPlayed   , &compareLastPlayedArcade   , false, "\uF161 " + _("LAST PLAYED")));
    sAllSorts.push_back(SortType(Sorts::PlayersAscending     , &compareNumberPlayers, &compareNumberPlayersArcade, true , "\uF162 " + _("NUMBER OF PLAYERS")));
    sAllSorts.push_back(SortType(Sorts::PlayersDescending    , &compareNumberPlayers, &compareNumberPlayersArcade, false, "\uF163 " + _("NUMBER OF PLAYERS")));
    sAllSorts.push_back(SortType(Sorts::DeveloperAscending   , &compareDevelopper   , &compareDevelopperArcade   , true , "\uF15d " + _("DEVELOPER")));
    sAllSorts.push_back(SortType(Sorts::DeveloperDescending  , &compareDevelopper   , &compareDevelopperArcade   , false, "\uF15e " + _("DEVELOPER")));
    sAllSorts.push_back(SortType(Sorts::PublisherAscending   , &comparePublisher    , &comparePublisherArcade    , true , "\uF15d " + _("PUBLISHER")));
    sAllSorts.push_back(SortType(Sorts::PublisherDescending  , &comparePublisher    , &comparePublisherArcade    , false, "\uF15e " + _("PUBLISHER")));
    sAllSorts.push_back(SortType(Sorts::GenreAscending       , &compareGenre        , &compareGenreArcade        , true , "\uF15d " + _("GENRE")));
    sAllSorts.push_back(SortType(Sorts::GenreDescending      , &compareGenre        , &compareGenreArcade        , false, "\uF15e " + _("GENRE")));
    sAllSorts.push_back(SortType(Sorts::SystemAscending      , &compareSystemName   , &compareFileNameArcade     , true , "\uF166 " + _("SYSTEM NAME")));
    sAllSorts.push_back(SortType(Sorts::SystemDescending     , &compareSystemName   , &compareFileNameArcade     , false, "\uF167 " + _("SYSTEM NAME")));
    sAllSorts.push_back(SortType(Sorts::ReleaseDateAscending , &compareReleaseDate  , &compareReleaseDateArcade  , true , "\uF160 " + _("RELEASE DATE")));
    sAllSorts.push_back(SortType(Sorts::ReleaseDateDescending, &compareReleaseDate  , &compareReleaseDateArcade  , false, "\uF161 " + _("RELEASE DATE")));
    sInitialized = true;
  }
  return sInitialized;
}

int FileSorts::unicodeCompareUppercase(const String& a, const String& b)
{
  for (int apos = 0, bpos = 0;; )
  {
    int c1 = (int)String::UpperUnicode(a.ReadUTF8(apos));
    int c2 = (int)String::UpperUnicode(b.ReadUTF8(bpos));
    if ((c1 | c2) == 0) { return 0; }
    int c = c1 - c2;
    if (c != 0) { return c; }
  }
}

int FileSorts::compareFoldersAndGames(const FileData& fd1, const FileData& fd2)
{
  ItemType f1 = fd1.Type();
  ItemType f2 = fd2.Type();
  if (f1 == f2) return 0;                // Both are games or folders
  if (f1 == ItemType::Folder) return -1; // f1 is a folder, f2 is a game
  return 1;                              // f2 is a folder
}

#define CheckFoldersAndGames(f1, f2) { int folderComparison = compareFoldersAndGames(f1, f2); if (folderComparison != 0) return folderComparison; }

ImplementSortMethod(compareSystemName)
{
  const SystemData& system1 = file1.System();
  const SystemData& system2 = file2.System();
  const int result = unicodeCompareUppercase(system1.Name(), system2.Name());
  if (result != 0) { return result; }
  return unicodeCompareUppercase(file1.Name(), file2.Name());
}

ImplementSortMethod(compareFileName)
{
  CheckFoldersAndGames(file1, file2)
  return sUseFileName
         ? unicodeCompareUppercase(file1.Metadata().RomFileOnly().ToString(), file2.Metadata().RomFileOnly().ToString())
         : unicodeCompareUppercase(file1.Name(), file2.Name());
}

ImplementSortMethod(compareRating)
{
  CheckFoldersAndGames(file1, file2)
  float c = file1.Metadata().Rating() - file2.Metadata().Rating();
  if (c < 0) { return -1; }
  if (c > 0) { return 1; }
  return unicodeCompareUppercase(file1.Name(), file2.Name());
}

ImplementSortMethod(compareTimesPlayed)
{
  CheckFoldersAndGames(file1, file2)
  int playCount = (file1).Metadata().PlayCount() - (file2).Metadata().PlayCount();
  if (playCount != 0) return playCount;
  return unicodeCompareUppercase(file1.Name(), file2.Name());
}

ImplementSortMethod(compareLastPlayed)
{
  CheckFoldersAndGames(file1, file2)
  unsigned int ep1 = (file1).Metadata().LastPlayedEpoc();
  unsigned int ep2 = (file2).Metadata().LastPlayedEpoc();
  if (ep1 == 0) ep1 = 0xFFFFFFFF;
  if (ep2 == 0) ep2 = 0xFFFFFFFF;
  if (ep1 == ep2)
    return unicodeCompareUppercase(file1.Name(), file2.Name());
  return ep1 < ep2 ? -1 : 1;
}

ImplementSortMethod(compareNumberPlayers)
{
  CheckFoldersAndGames(file1, file2)
  int players = (file1).Metadata().PlayerRange() - (file2).Metadata().PlayerRange();
  if (players != 0) return players;
  return unicodeCompareUppercase(file1.Name(), file2.Name());
}

ImplementSortMethod(compareDevelopper)
{
  CheckFoldersAndGames(file1, file2)
  return unicodeCompareUppercase(file1.Metadata().Developer(), file2.Metadata().Developer());
}

ImplementSortMethod(comparePublisher)
{
  CheckFoldersAndGames(file1, file2)
  return unicodeCompareUppercase(file1.Metadata().Publisher(), file2.Metadata().Publisher());
}

ImplementSortMethod(compareGenre)
{
  CheckFoldersAndGames(file1, file2)
  int genre = (int)(file1).Metadata().GenreId() - (int)(file2).Metadata().GenreId();
  if (genre != 0) return genre;
  return unicodeCompareUppercase(file1.Name(), file2.Name());
}

ImplementSortMethod(compareReleaseDate)
{
  CheckFoldersAndGames(file1, file2)
  int releasedate = (int)(file1).Metadata().ReleaseDateEpoc() - (int)(file2).Metadata().ReleaseDateEpoc();
  if (releasedate != 0) return releasedate;
  return unicodeCompareUppercase(file1.Name(), file2.Name());
}

int FileSorts::compareFileNameArcade(const ArcadeTupple& at1, const ArcadeTupple& at2)
{
  if (sUseDatabaseNames && at1.mArcade != nullptr && at2.mArcade != nullptr)
  {
    CheckFoldersAndGames(*at1.mGame, *at2.mGame)
    return unicodeCompareUppercase(at1.mArcade->ArcadeName(), at2.mArcade->ArcadeName());
  }
  // Both non arcade games
  return compareFileName(*at1.mGame, *at2.mGame);
}

ImplementSortMethodArcade(compareRatingArcade, compareRating)

ImplementSortMethodArcade(compareTimesPlayedArcade, compareTimesPlayed)

ImplementSortMethodArcade(compareLastPlayedArcade, compareLastPlayed)

ImplementSortMethodArcade(compareNumberPlayersArcade, compareNumberPlayers)

ImplementSortMethodArcade(compareDevelopperArcade, compareDevelopper)

ImplementSortMethodArcade(comparePublisherArcade, comparePublisher)

ImplementSortMethodArcade(compareGenreArcade, compareGenre)

ImplementSortMethodArcade(compareReleaseDateArcade, compareReleaseDate)

const std::vector<FileSorts::Sorts>& FileSorts::AvailableSorts(SortSets set)
{
  switch(set)
  {
    case SortSets::MultiSystem:
    {
      //! Ordered multi-system sorts
      static std::vector<FileSorts::Sorts> sMulti =
      {
        Sorts::FileNameAscending,
        Sorts::FileNameDescending,
        Sorts::SystemAscending,
        Sorts::SystemDescending,
        Sorts::GenreAscending,
        Sorts::GenreDescending,
        Sorts::RatingAscending,
        Sorts::RatingDescending,
        Sorts::TimesPlayedAscending,
        Sorts::TimesPlayedDescending,
        Sorts::LastPlayedAscending,
        Sorts::LastPlayedDescending,
        Sorts::PlayersAscending,
        Sorts::PlayersDescending,
        Sorts::DeveloperAscending,
        Sorts::DeveloperDescending,
        Sorts::PublisherAscending,
        Sorts::PublisherDescending,
        Sorts::ReleaseDateAscending,
        Sorts::ReleaseDateDescending,
      };
      return sMulti;
    }
    case SortSets::Arcade:
    {
      //! Arcade sorts
      static std::vector<FileSorts::Sorts> sArcade =
      {
        Sorts::FileNameAscending,
        Sorts::FileNameDescending,
        Sorts::GenreAscending,
        Sorts::GenreDescending,
        Sorts::RatingAscending,
        Sorts::RatingDescending,
        Sorts::TimesPlayedAscending,
        Sorts::TimesPlayedDescending,
        Sorts::LastPlayedAscending,
        Sorts::LastPlayedDescending,
        Sorts::PlayersAscending,
        Sorts::PlayersDescending,
        Sorts::DeveloperAscending,
        Sorts::DeveloperDescending,
        Sorts::PublisherAscending,
        Sorts::PublisherDescending,
        Sorts::ReleaseDateAscending,
        Sorts::ReleaseDateDescending,
      };
      return sArcade;
    }
    case SortSets::SingleSystem:
    default: break;
  }

  //! Ordered mono-system sorts
  static std::vector<FileSorts::Sorts> sSingle =
  {
    Sorts::FileNameAscending,
    Sorts::FileNameDescending,
    Sorts::GenreAscending,
    Sorts::GenreDescending,
    Sorts::RatingAscending,
    Sorts::RatingDescending,
    Sorts::TimesPlayedAscending,
    Sorts::TimesPlayedDescending,
    Sorts::LastPlayedAscending,
    Sorts::LastPlayedDescending,
    Sorts::PlayersAscending,
    Sorts::PlayersDescending,
    Sorts::DeveloperAscending,
    Sorts::DeveloperDescending,
    Sorts::PublisherAscending,
    Sorts::PublisherDescending,
    Sorts::ReleaseDateAscending,
    Sorts::ReleaseDateDescending,
  };
  return sSingle;
}

const String& FileSorts::Name(FileSorts::Sorts sort)
{
  // Lazy initialization
  Initialize();

  for(const FileSorts::SortType& sortType : sAllSorts)
    if (sortType.mSort == sort)
      return sortType.mDescription;

  static String unknown("Unknown sort");
  return unknown;
}

bool FileSorts::IsAscending(FileSorts::Sorts sort)
{
  // Lazy initialization
  Initialize();

  for(const FileSorts::SortType& sortType : sAllSorts)
    if (sortType.mSort == sort)
      return sortType.mAscending;

  return false;
}


FileData::Comparer FileSorts::Comparer(FileSorts::Sorts sort)
{
  // Lazy initialization
  Initialize();
  sUseFileName = RecalboxConf::Instance().GetDisplayByFileName();

  for(const FileSorts::SortType& sortType : sAllSorts)
    if (sortType.mSort == sort)
      return sortType.mComparer;

  return nullptr;
}

FileSorts::ComparerArcade FileSorts::ComparerArcadeFromSort(FileSorts::Sorts sort)
{
  // Lazy initialization
  Initialize();
  sUseFileName = RecalboxConf::Instance().GetDisplayByFileName();
  sUseDatabaseNames = RecalboxConf::Instance().GetArcadeUseDatabaseNames();

  for(const FileSorts::SortType& sortType : sAllSorts)
    if (sortType.mSort == sort)
      return sortType.mComparerArcade;

  return nullptr;
}

FileSorts::Sorts FileSorts::Clamp(FileSorts::Sorts sort, FileSorts::SortSets set)
{
  const std::vector<FileSorts::Sorts>& sorts = AvailableSorts(set);

  // Sort available in the set?
  for(FileSorts::Sorts availableSort : sorts)
    if (sort == availableSort)
      return sort;

  // Nope, return the first available
  return sorts[0];
}

void FileSorts::SortArcade(ArcadeTupplePointerList& items, ComparerArcade comparer, bool ascending)
{
  if (items.size() > 1)
  {
    if (ascending)
      QuickSortAscendingArcade(items, 0, (int)items.size() - 1, comparer);
    else
      QuickSortDescendingArcade(items, 0, (int)items.size() - 1, comparer);
  }
}

void FileSorts::QuickSortAscendingArcade(ArcadeTupplePointerList& items, int low, int high, ComparerArcade comparer)
{
  int Low = low, High = high;
  const ArcadeTupple& pivot = *items[(Low + High) >> 1];
  do
  {
    while((*comparer)(*items[Low] , pivot) < 0) Low++;
    while((*comparer)(*items[High], pivot) > 0) High--;
    if (Low <= High)
    {
      ArcadeTupple* Tmp = items[Low]; items[Low] = items[High]; items[High] = Tmp;
      Low++; High--;
    }
  }while(Low <= High);
  if (High > low) QuickSortAscendingArcade(items, low, High, comparer);
  if (Low < high) QuickSortAscendingArcade(items, Low, high, comparer);
}

void FileSorts::QuickSortDescendingArcade(ArcadeTupplePointerList& items, int low, int high, ComparerArcade comparer)
{
  int Low = low, High = high;
  const ArcadeTupple& pivot = *items[(Low + High) >> 1];
  do
  {
    while((*comparer)(*items[Low] , pivot) > 0) Low++;
    while((*comparer)(*items[High], pivot) < 0) High--;
    if (Low <= High)
    {
      ArcadeTupple* Tmp = items[Low]; items[Low] = items[High]; items[High] = Tmp;
      Low++; High--;
    }
  }while(Low <= High);
  if (High > low) QuickSortDescendingArcade(items, low, High, comparer);
  if (Low < high) QuickSortDescendingArcade(items, Low, high, comparer);
}


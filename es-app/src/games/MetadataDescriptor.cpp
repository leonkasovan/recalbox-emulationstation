#include "MetadataDescriptor.h"
#include "MetadataFieldDescriptor.h"
#include "utils/locale/LocaleHelper.h"

// TODO: Use const char* instead
const String MetadataDescriptor::GameNodeIdentifier("game");
const String MetadataDescriptor::FolderNodeIdentifier("folder");

MetadataStringHolder MetadataDescriptor::sNameHolder(1 << 20, 128 << 10);
MetadataStringHolder MetadataDescriptor::sDescriptionHolder(1 << 20, 128 << 10);
MetadataStringHolder MetadataDescriptor::sDeveloperHolder(64 << 10, 32 << 10);
MetadataStringHolder MetadataDescriptor::sPublisherHolder(64 << 10, 32 << 10);
MetadataStringHolder MetadataDescriptor::sGenreHolder(64 << 10, 32 << 10);
MetadataStringHolder MetadataDescriptor::sEmulatorHolder(2 << 10, 1 << 10);
MetadataStringHolder MetadataDescriptor::sCoreHolder(2 << 10, 1 << 10);
MetadataStringHolder MetadataDescriptor::sRatioHolder(1 << 10, 1 << 10);
MetadataStringHolder MetadataDescriptor::sPathHolder(64 << 10, 32 << 10);
MetadataStringHolder MetadataDescriptor::sFileHolder(128 << 10, 32 << 10);

#ifdef _METADATA_STATS_
int MetadataDescriptor::LivingClasses = 0;
int MetadataDescriptor::LivingFolders = 0;
int MetadataDescriptor::LivingGames = 0;
#endif

const MetadataFieldDescriptor* MetadataDescriptor::GetMetadataFieldDescriptors(ItemType type, int& count)
{
  switch(type)
  {
    case ItemType::Game:
    {
      static const MetadataFieldDescriptor sGameMetadataDescriptors[] =
      {
        MetadataFieldDescriptor("path"       , ""     , _("Path")        , _("enter game path")             , MetadataType::Path        ,MetadataFieldDescriptor::DataType::Path   , MetadataFieldDescriptor::EditableType::None   , &MetadataDescriptor::IsDefaultRom            , &MetadataDescriptor::RomAsString         , &MetadataDescriptor::SetRomPathAsString      , false, true),
        MetadataFieldDescriptor("name"       , ""     , _("Name")        , _("enter game name")             , MetadataType::Name        ,MetadataFieldDescriptor::DataType::String , MetadataFieldDescriptor::EditableType::Text   , &MetadataDescriptor::IsDefaultName           , &MetadataDescriptor::NameAsString        , &MetadataDescriptor::SetName                 , false, true),
        MetadataFieldDescriptor("rating"     , "0.0"  , _("Rating")      , _("enter rating")                , MetadataType::Rating      ,MetadataFieldDescriptor::DataType::Rating , MetadataFieldDescriptor::EditableType::Rating , &MetadataDescriptor::IsDefaultRating         , &MetadataDescriptor::RatingAsString      , &MetadataDescriptor::SetRatingAsString       , false, true),
        MetadataFieldDescriptor("favorite"   , "false", _("Favorite")    , _("enter favorite")              , MetadataType::Favorite    ,MetadataFieldDescriptor::DataType::Bool   , MetadataFieldDescriptor::EditableType::Switch , &MetadataDescriptor::IsDefaultFavorite       , &MetadataDescriptor::FavoriteAsString    , &MetadataDescriptor::SetFavoriteAsString     , false, true),
        MetadataFieldDescriptor("hidden"     , "false", _("Hidden")      , _("set hidden")                  , MetadataType::Hidden      ,MetadataFieldDescriptor::DataType::Bool   , MetadataFieldDescriptor::EditableType::Switch , &MetadataDescriptor::IsDefaultHidden         , &MetadataDescriptor::HiddenAsString      , &MetadataDescriptor::SetHiddenAsString       , false, true),
        MetadataFieldDescriptor("emulator"   , ""     , _("Emulator")    , _("enter emulator")              , MetadataType::Emulator    ,MetadataFieldDescriptor::DataType::List   , MetadataFieldDescriptor::EditableType::List   , &MetadataDescriptor::IsDefaultEmulator       , &MetadataDescriptor::EmulatorAsString    , &MetadataDescriptor::SetEmulator             , false, true),
        MetadataFieldDescriptor("core"       , ""     , _("Core")        , _("enter core")                  , MetadataType::Core        ,MetadataFieldDescriptor::DataType::List   , MetadataFieldDescriptor::EditableType::List   , &MetadataDescriptor::IsDefaultCore           , &MetadataDescriptor::CoreAsString        , &MetadataDescriptor::SetCore                 , false, true),
        MetadataFieldDescriptor("ratio"      , "auto" , _("Ratio")       , _("enter ratio")                 , MetadataType::Ratio       ,MetadataFieldDescriptor::DataType::List   , MetadataFieldDescriptor::EditableType::List   , &MetadataDescriptor::IsDefaultRatio          , &MetadataDescriptor::RatioAsString       , &MetadataDescriptor::SetRatio                , false, true),
        MetadataFieldDescriptor("desc"       , ""     , _("Description") , _("enter description")           , MetadataType::Synopsis    ,MetadataFieldDescriptor::DataType::Text   , MetadataFieldDescriptor::EditableType::Text   , &MetadataDescriptor::IsDefaultDescription    , &MetadataDescriptor::DescriptionAsString , &MetadataDescriptor::SetDescription          , false, false),
        MetadataFieldDescriptor("image"      , ""     , _("Image")       , _("enter path to image")         , MetadataType::Image       ,MetadataFieldDescriptor::DataType::Path   , MetadataFieldDescriptor::EditableType::Text   , &MetadataDescriptor::IsDefaultImage          , &MetadataDescriptor::ImageAsString       , &MetadataDescriptor::SetImagePathAsString    , false, false),
        MetadataFieldDescriptor("thumbnail"  , ""     , _("Thumbnail")   , _("enter path to thumbnail")     , MetadataType::Thumbnail   ,MetadataFieldDescriptor::DataType::Path   , MetadataFieldDescriptor::EditableType::Text   , &MetadataDescriptor::IsDefaultThumbnail      , &MetadataDescriptor::ThumbnailAsString   , &MetadataDescriptor::SetThumbnailPathAsString, false, false),
        MetadataFieldDescriptor("video"      , ""     , _("Video")       , _("enter path to video")         , MetadataType::Video       ,MetadataFieldDescriptor::DataType::Path   , MetadataFieldDescriptor::EditableType::Text   , &MetadataDescriptor::IsDefaultVideo          , &MetadataDescriptor::VideoAsString       , &MetadataDescriptor::SetVideoPathAsString    , false, false),
        MetadataFieldDescriptor("releasedate", ""     , _("Release date"), _("enter release date")          , MetadataType::ReleaseDate ,MetadataFieldDescriptor::DataType::Date   , MetadataFieldDescriptor::EditableType::Date   , &MetadataDescriptor::IsDefaultReleaseDateEpoc, &MetadataDescriptor::ReleaseDateAsString , &MetadataDescriptor::SetReleaseDateAsString  , false, false),
        MetadataFieldDescriptor("developer"  , ""     , _("Developer")   , _("enter game developer")        , MetadataType::Developer   ,MetadataFieldDescriptor::DataType::String , MetadataFieldDescriptor::EditableType::Text   , &MetadataDescriptor::IsDefaultDeveloper      , &MetadataDescriptor::DeveloperAsString   , &MetadataDescriptor::SetDeveloper            , false, false),
        MetadataFieldDescriptor("publisher"  , ""     , _("Publisher")   , _("enter game publisher")        , MetadataType::Publisher   ,MetadataFieldDescriptor::DataType::String , MetadataFieldDescriptor::EditableType::Text   , &MetadataDescriptor::IsDefaultPublisher      , &MetadataDescriptor::PublisherAsString   , &MetadataDescriptor::SetPublisher            , false, false),
        MetadataFieldDescriptor("genre"      , ""     , _("Genre")       , _("enter game genre")            , MetadataType::Genre       ,MetadataFieldDescriptor::DataType::Int    , MetadataFieldDescriptor::EditableType::Text   , &MetadataDescriptor::IsDefaultGenre          , &MetadataDescriptor::GenreAsString       , &MetadataDescriptor::SetGenre                , false, false),
        MetadataFieldDescriptor("genreid"    , ""     , _("Genre ID")    , _("enter game genre id")         , MetadataType::GenreId     ,MetadataFieldDescriptor::DataType::Int    , MetadataFieldDescriptor::EditableType::List   , &MetadataDescriptor::IsDefaultGenreId        , &MetadataDescriptor::GenreIdAsString     , &MetadataDescriptor::SetGenreIdAsString      , false, false),
        MetadataFieldDescriptor("adult"      , ""     , _("Adult")       , _("enter adult state")           , MetadataType::Adult       ,MetadataFieldDescriptor::DataType::Bool   , MetadataFieldDescriptor::EditableType::Switch , &MetadataDescriptor::IsDefaultAdult          , &MetadataDescriptor::AdultAsString       , &MetadataDescriptor::SetAdultAsString        , false, false),
        MetadataFieldDescriptor("players"    , "1"    , _("Players")     , _("enter number of players")     , MetadataType::Players     ,MetadataFieldDescriptor::DataType::Range  , MetadataFieldDescriptor::EditableType::Text   , &MetadataDescriptor::IsDefaultPlayerRange    , &MetadataDescriptor::PlayersAsString     , &MetadataDescriptor::SetPlayersAsString      , false, false),
        MetadataFieldDescriptor("region"     , ""     , _("Region")      , _("enter region")                , MetadataType::Region      ,MetadataFieldDescriptor::DataType::String , MetadataFieldDescriptor::EditableType::Text   , &MetadataDescriptor::IsDefaultRegion         , &MetadataDescriptor::RegionAsString      , &MetadataDescriptor::SetRegionAsString       , false, false),
        MetadataFieldDescriptor("playcount"  , "0"    , _("Play count")  , _("enter number of times played"), MetadataType::PlayCount   ,MetadataFieldDescriptor::DataType::Int    , MetadataFieldDescriptor::EditableType::None   , &MetadataDescriptor::IsDefaultPlayCount      , &MetadataDescriptor::PlayCountAsString   , &MetadataDescriptor::SetPlayCountAsString    , true , false),
        MetadataFieldDescriptor("lastplayed" , "0"    , _("Last played") , _("enter last played date")      , MetadataType::LastPlayed  ,MetadataFieldDescriptor::DataType::Date   , MetadataFieldDescriptor::EditableType::None   , &MetadataDescriptor::IsDefaultLastPlayedEpoc , &MetadataDescriptor::LastPlayedAsString  , &MetadataDescriptor::SetLastPlayedAsString   , true , false),
        MetadataFieldDescriptor("hash"       , "0"    , _("Rom Crc32")   , _("enter rom crc32")             , MetadataType::Crc32       ,MetadataFieldDescriptor::DataType::Crc32  , MetadataFieldDescriptor::EditableType::None   , &MetadataDescriptor::IsDefaultRomCrc32       , &MetadataDescriptor::RomCrc32AsString    , &MetadataDescriptor::SetRomCrc32AsString     , true , false),
        MetadataFieldDescriptor("lastPatch"  , ""     , _("Last Patch")  , _("enter patch")                 , MetadataType::LastPatch   ,MetadataFieldDescriptor::DataType::Path   , MetadataFieldDescriptor::EditableType::None   , &MetadataDescriptor::IsDefaultLastPath       , &MetadataDescriptor::LastPatchAsString   , &MetadataDescriptor::SetLastPatchAsString    , true , false),
        MetadataFieldDescriptor("rotation"   , "None" , _("Rotation")    , _("enter rotation")              , MetadataType::Rotation    ,MetadataFieldDescriptor::DataType::Int    , MetadataFieldDescriptor::EditableType::Switch , &MetadataDescriptor::IsDefaultRotation       , &MetadataDescriptor::RotationAsString    , &MetadataDescriptor::SetRotationAsString     , false, false),
        MetadataFieldDescriptor("timeplayed" , "0"    , _("TimePlayed")  , _("enter TimePlayed")            , MetadataType::TimePlayed  , MetadataFieldDescriptor::DataType::Int   , MetadataFieldDescriptor::EditableType::None   , &MetadataDescriptor::IsDefaultTimePlayed     , &MetadataDescriptor::TimePlayedAsString  , &MetadataDescriptor::SetTimePlayedAsString   , false, false),
      };

      count = sizeof(sGameMetadataDescriptors) / sizeof(MetadataFieldDescriptor);
      return &sGameMetadataDescriptors[0];
    }
    case ItemType::Folder:
    {
      static const MetadataFieldDescriptor sFolderMetadataDescriptors[] =
      {
        MetadataFieldDescriptor("path"       , ""        , _("Path")        , _("enter game path")             , MetadataType::Path        ,MetadataFieldDescriptor::DataType::Path  , MetadataFieldDescriptor::EditableType::None  , &MetadataDescriptor::IsDefaultRom            , &MetadataDescriptor::RomAsString         , &MetadataDescriptor::SetRomPathAsString      , false, true),
        MetadataFieldDescriptor("name"       , ""        , _("Name")        , _("enter game name")             , MetadataType::Name        ,MetadataFieldDescriptor::DataType::String, MetadataFieldDescriptor::EditableType::Text  , &MetadataDescriptor::IsDefaultName           , &MetadataDescriptor::NameAsString        , &MetadataDescriptor::SetName                 , false, true),
        MetadataFieldDescriptor("hidden"     , "false"   , _("Hidden")      , _("set hidden")                  , MetadataType::Hidden      ,MetadataFieldDescriptor::DataType::Bool  , MetadataFieldDescriptor::EditableType::Switch, &MetadataDescriptor::IsDefaultHidden         , &MetadataDescriptor::HiddenAsString      , &MetadataDescriptor::SetHiddenAsString       , false, true),
        MetadataFieldDescriptor("desc"       , ""        , _("Description") , _("enter description")           , MetadataType::Synopsis    ,MetadataFieldDescriptor::DataType::Text  , MetadataFieldDescriptor::EditableType::Text  , &MetadataDescriptor::IsDefaultDescription    , &MetadataDescriptor::DescriptionAsString , &MetadataDescriptor::SetDescription          , false, false),
        MetadataFieldDescriptor("image"      , ""        , _("Image")       , _("enter path to image")         , MetadataType::Image       ,MetadataFieldDescriptor::DataType::Path  , MetadataFieldDescriptor::EditableType::Text  , &MetadataDescriptor::IsDefaultImage          , &MetadataDescriptor::ImageAsString       , &MetadataDescriptor::SetImagePathAsString    , false, false),
        MetadataFieldDescriptor("thumbnail"  , ""        , _("Thumbnail")   , _("enter path to thumbnail")     , MetadataType::Thumbnail   ,MetadataFieldDescriptor::DataType::Path  , MetadataFieldDescriptor::EditableType::Text  , &MetadataDescriptor::IsDefaultThumbnail      , &MetadataDescriptor::ThumbnailAsString   , &MetadataDescriptor::SetThumbnailPathAsString, false, false),
      };

      count = sizeof(sFolderMetadataDescriptors) / sizeof(MetadataFieldDescriptor);
      return &sFolderMetadataDescriptors[0];
    }
    case ItemType::Root:
    case ItemType::Empty:
    default: break;
  }
  count = 0;
  return nullptr;
}

const MetadataDescriptor& MetadataDescriptor::Default()
{
  static bool initialized = false;
  static MetadataDescriptor defaultData(Path::Empty, "default", ItemType::Game);

  if (!initialized)
  {
    int count = 0;
    const MetadataFieldDescriptor* fields = GetMetadataFieldDescriptors(ItemType::Game, count);

    for (; --count >= 0;)
    {
      // Get field descriptor
      const MetadataFieldDescriptor& field = fields[count];

      // Set default value
      String value = field.DefaultValue();
      (defaultData.*field.SetValueMethod())(value);
    }
    initialized = true;
  }

  return defaultData;
}

String MetadataDescriptor::IntToRange(int range)
{
  int max = range >> 16;
  int min = range & 0xFFFF;

  // min = max, only one number
  if (min == max) return String(max);

  String value = String(max);

  // min or more range
  if (min == 0xFFFF)
  {
    value += '+';
  }
  else
  {
    // Full range
    value = String(min).Append('-').Append(value);
  }
  return value;
}

bool MetadataDescriptor::RangeToInt(const String& range, int& to)
{
  // max+ (min+)
  int p = range.Find('+');
  if (p >= 0)
  {
    if (!StringToInt(range, p, 0, '+')) return false;
    to = (p << 16) + 0xFFFF;
    return true;
  }

  // max-max
  p = range.Find('-');
  if (p < 0)
  {
    if (!StringToInt(range, p)) return false;
    to = (p << 16) + p;
    return true;
  }

  // min-max
  int min = 0; if (!StringToInt(range, min, 0, '-')) return false;
  int max = 0; if (!StringToInt(range, max, p + 1, 0  )) return false;
  if (min > max) { min = min ^ max; max = max ^ min; min = min ^ max; }
  to = (max << 16) + min;
  return true;
}

bool MetadataDescriptor::IntToHex(int from, String& to)
{
  static const char* hexa = "0123456789ABCDEF";
  char result[9];
  result[sizeof(result) - 1] = 0;

  for (int i = sizeof(result) - 1, p = 0; -- i >= 0;)
    result[p++] = hexa[(from >> (i << 2)) & 0xF];

  to = result;
  return true;
}

bool MetadataDescriptor::HexToInt(const String& from, int& to)
{
  if (from.empty()) return false;
  const char* src = from.c_str();

  int result = 0;
  for (;; src++)
  {
    int v = (unsigned char)src[0];
    if ((unsigned int)(v - 0x30) <= 9) { result <<= 4; result += v - 0x30; }
    else
    {
      v &= 0xDF;
      if ((unsigned int)(v - 0x41) <= 5) { result <<= 4; result += v - 0x37; }
      else break;
    }
  }
  if (src[0] != 0) return false;

  to = result;
  return true;
}

bool MetadataDescriptor::StringToInt(const String& from, int& to, int offset, char stop)
{
  const char* src = from.c_str() + offset;

  bool sign = (src[0] == '-');
  if (sign) src++;

  int result = 0;
  while ((unsigned int)(src[0] - 0x30) <= 9) { result *= 10; result += src[0] - 0x30; src++; }
  if (src[0] != stop) return false;

  to = sign ? -result : result;
  return true;
}

bool MetadataDescriptor::StringToInt(const String& from, int& to)
{
  const char* src = from.c_str();

  bool sign = (src[0] == '-');
  if (sign) src++;

  int result = 0;
  while ((unsigned int)(src[0] - 0x30) <= 9) { result *= 10; result += src[0] - 0x30; src++; }
  if (src[0] != 0) return false;

  to = sign ? -result : result;
  return true;
}

bool MetadataDescriptor::StringToFloat(const String& from, float& to)
{
  const char* src = from.c_str();

  bool sign = (src[0] == '-');
  if (sign) src++;

  int intPart = 0;
  int fractPart = 0;
  int pow10 = 1;

  // Integer part
  while ((unsigned int)(src[0] - 0x30) <= 9) { intPart *= 10; intPart += src[0] - 0x30; src++; }
  if (src[0] == '.')
  {
    src++;
    while ((unsigned int)(src[0] - 0x30) <= 9) { fractPart *= 10; fractPart += src[0] - 0x30; src++; pow10 *= 10; }
  }
  if (src[0] != 0) return false;

  float result = (float)intPart + ((float)fractPart / (float)pow10);

  to = sign ? -result : result;
  return true;
}

bool MetadataDescriptor::Deserialize(const XmlNode from, const Path& relativeTo)
{
  #ifdef _METADATA_STATS_
    if (_Type == ItemType::Game) LivingGames--;
    if (_Type == ItemType::Folder) LivingFolders--;
  #endif

  String name = from.name();
  if (name == GameNodeIdentifier) mType = ItemType::Game;
  else if (name == FolderNodeIdentifier) mType = ItemType::Folder;
  else return false; // Unidentified node

  mTimeStamp = (unsigned int)Xml::AttributeAsInt(from, "timestamp", 0);

  #ifdef _METADATA_STATS_
    if (_Type == ItemType::Game) LivingGames++;
    if (_Type == ItemType::Folder) LivingFolders++;
  #endif

  int count = 0;
  const MetadataFieldDescriptor* fields = GetMetadataFieldDescriptors(mType, count);
  if (fields == nullptr) return false;

  for (; --count >= 0; )
  {
    // Get field descriptor
    const MetadataFieldDescriptor& field = fields[count];

    // Get field data as string
    const String& defaultStringValue = field.DefaultValue();
    String value = Xml::AsString(from, field.Key(), defaultStringValue);
    // Ignore default values
    if (value == defaultStringValue) continue;

    // Absolute path
    if (field.Type() == MetadataFieldDescriptor::DataType::Path)
      value = Path(value).ToAbsolute(relativeTo).ToString();

    // Set value!
    (this->*field.SetValueMethod())(value);
  }

  // Control name
  if (mName < 0)
  {
    // Extract default name
    String defaultName = sFileHolder.GetString(mRomFile);
    mName = sNameHolder.AddString32(defaultName);
    mDirty = true;
  }
  else mDirty = false;

  return true;
}

void MetadataDescriptor::Serialize(XmlNode parentNode, const Path& filePath, const Path& relativeTo) const
{
  (void)filePath;
  int count = 0;
  bool dummy = false;

  const MetadataFieldDescriptor* fields = GetMetadataFieldDescriptors(mType, count);
  if (fields == nullptr) return;

  // Add empty node game/folder
  XmlNode node = parentNode.append_child(mType == ItemType::Game ? GameNodeIdentifier.c_str() : FolderNodeIdentifier.c_str());
  Xml::AddAttribute(node, "source", "Recalbox");
  Xml::AddAttribute(node, "timestamp", mTimeStamp);

  // Metadata
  String value;
  for (; --count >= 0; )
  {
    // Get field descriptor
    const MetadataFieldDescriptor& field = fields[count];

    // Default value?
    if ((this->*field.IsDefaultValueMethod())()) continue;

    // Get value
    value = (this->*field.GetValueMethod())();

    // Relative path
    if (field.Type() == MetadataFieldDescriptor::DataType::Path)
      value = Path(value).MakeRelative(relativeTo, dummy).ToString();

    // Store
    Xml::AddAsString(node, field.Key(), value);
  }
}

void MetadataDescriptor::Merge(const MetadataDescriptor& sourceMetadata)
{
  int count = 0;
  const MetadataFieldDescriptor* fields = GetMetadataFieldDescriptors(mType, count);
  if (fields == nullptr) return;

  for (; --count >= 0; )
  {
    // Get field descriptor
    const MetadataFieldDescriptor& field = fields[count];

    // Get/Set if current field is the default value
    if ((this->*field.IsDefaultValueMethod())())
    {
      (this->*field.SetValueMethod())((sourceMetadata.*field.GetValueMethod())());
      // A field has been copied. Set the dirty flag
      mDirty = true;
    }
  }
}

void MetadataDescriptor::CleanupHolders()
{
  LOG(LogDebug) << "[MetadataDescriptor] Name storage: "        << sNameHolder.StorageSize()          << " - object count: " << sNameHolder.ObjectCount()       ;
  LOG(LogDebug) << "[MetadataDescriptor] Description storage: " << sDescriptionHolder.StorageSize()   << " - object count: " << sDescriptionHolder.ObjectCount();
  LOG(LogDebug) << "[MetadataDescriptor] Publisher storage: "   << sPublisherHolder.StorageSize()     << " - object count: " << sPublisherHolder.ObjectCount()  ;
  LOG(LogDebug) << "[MetadataDescriptor] Developer storage: "   << sDeveloperHolder.StorageSize()     << " - object count: " << sDeveloperHolder.ObjectCount()  ;
  LOG(LogDebug) << "[MetadataDescriptor] Genre storage: "       << sGenreHolder.StorageSize()         << " - object count: " << sGenreHolder.ObjectCount()      ;
  LOG(LogDebug) << "[MetadataDescriptor] Ratio storage: "       << sRatioHolder.StorageSize()         << " - object count: " << sRatioHolder.ObjectCount()      ;
  LOG(LogDebug) << "[MetadataDescriptor] Core storage: "        << sCoreHolder.StorageSize()          << " - object count: " << sCoreHolder.ObjectCount()       ;
  LOG(LogDebug) << "[MetadataDescriptor] Emulator storage: "    << sEmulatorHolder.StorageSize()      << " - object count: " << sEmulatorHolder.ObjectCount()   ;
  LOG(LogDebug) << "[MetadataDescriptor] Path storage: "        << sPathHolder.StorageSize()          << " - object count: " << sPathHolder.ObjectCount()       ;
  LOG(LogDebug) << "[MetadataDescriptor] File storage: "        << sFileHolder.StorageSize()          << " - object count: " << sFileHolder.ObjectCount()       ;

  sNameHolder.Finalize();
  sDescriptionHolder.Finalize();
  sPublisherHolder.Finalize();
  sDeveloperHolder.Finalize();
  sGenreHolder.Finalize();
  sRatioHolder.Finalize();
  sCoreHolder.Finalize();
  sEmulatorHolder.Finalize();
  sPathHolder.Finalize();
  sFileHolder.Finalize();
}

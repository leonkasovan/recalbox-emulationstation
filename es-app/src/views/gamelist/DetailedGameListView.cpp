#include <RecalboxConf.h>
#include <VideoEngine.h>
#include "views/gamelist/DetailedGameListView.h"
#include "views/ViewController.h"
#include "animations/LambdaAnimation.h"
#include "utils/locale/LocaleHelper.h"
#include "scraping/ScraperSeamless.h"
#include "recalbox/RecalboxStorageWatcher.h"

DetailedGameListView::DetailedGameListView(WindowManager&window, SystemManager& systemManager, SystemData& system)
  : ISimpleGameListView(window, systemManager, system)
  , mEmptyListItem(&system)
  , mPopulatedFolder(nullptr)
  , mList(window)
  , mElapsedTimeOnGame(0)
  , mIsScraping(false)
  , mImage(window)
  , mNoImage(window)
  , mVideo(window)
  , mLblRating(window)
  , mLblReleaseDate(window)
  , mLblDeveloper(window)
  , mLblPublisher(window)
  , mLblGenre(window)
  , mLblPlayers(window)
  , mLblLastPlayed(window)
  , mLblPlayCount(window)
  , mLblFavorite(window)
  , mFolderName(window)
  , mRating(window, 0.f)
  , mReleaseDate(window)
  , mDeveloper(window)
  , mPublisher(window)
  , mGenre(window)
  , mPlayers(window)
  , mLastPlayed(window)
  , mPlayCount(window)
  , mFavorite(window)
  , mDescContainer(window)
  , mDescription(window)
  , mBusy(window)
  , mSettings(RecalboxConf::Instance())
  , mFadeBetweenImage(-1)
{
}

void DetailedGameListView::Initialize()
{
  addChild(&mList);

  mEmptyListItem.Metadata().SetName(_("YOUR LIST IS EMPTY. PRESS START TO CHANGE GAME FILTERS."));
  populateList(mSystem.MasterRoot());

  mList.setCursorChangedCallback([this](const CursorState& state)
                                 {
                                   (void) state;
                                   updateInfoPanel();
                                 });
  mList.SetOverlayInterface(this);

  const float padding = 0.01f;

  mList.setDefaultZIndex(20);
  mList.setPosition(mSize.x() * (0.50f + padding), mList.getPosition().y());
  mList.setSize(mSize.x() * (0.50f - padding), mList.getSize().y());
  mList.setAlignment(HorizontalAlignment::Left);

  // folder components
  for (int i = 3 * 3; --i >= 0; )
  {
    auto* img = new ImageComponent(mWindow);
    addChild(img); // normalised functions required to be added first
    img->setOrigin(0.5f, 0.5f);
    img->setNormalisedMaxSize(0.4f, 0.4f);
    img->setDefaultZIndex(30);
    img->setZIndex(30);
    mFolderContent.push_back(img);
  }

  addChild(&mFolderName);
  mFolderName.setDefaultZIndex(40);

  // image
  mImage.setOrigin(0.5f, 0.5f);
  mImage.setPosition(mSize.x() * 0.25f, mList.getPosition().y() + mSize.y() * 0.2125f);
  mImage.setMaxSize(mSize.x() * (0.50f - 2 * padding), mSize.y() * 0.4f);
  mImage.setDefaultZIndex(30);

  // no image
  mNoImage.setOrigin(mImage.getOrigin());
  mNoImage.setPosition(mImage.getPosition());
  mNoImage.setMaxSize(mImage.getSize());
  mNoImage.setDefaultZIndex(30);

  addChild(&mNoImage);
  addChild(&mImage);

  // video
  mVideo.setOrigin(0.5f, 0.5f);
  mVideo.setPosition(mSize.x() * 0.25f, mList.getPosition().y() + mSize.y() * 0.2125f);
  mVideo.setMaxSize(mSize.x() * (0.50f - 2 * padding), mSize.y() * 0.4f);
  mVideo.setDefaultZIndex(30);
  addChild(&mVideo);

  // Busy
  mBusy.setPosition(mImage.getPosition());
  mBusy.setSize(mImage.getSize());
  mBusy.setText(_("UPDATING..."));

  // metadata labels + values
  mLblRating.setText(_("Rating") + ": ");
  addChild(&mLblRating);
  addChild(&mRating);
  mLblReleaseDate.setText(_("Released") + ": ");
  addChild(&mLblReleaseDate);
  addChild(&mReleaseDate);
  mLblDeveloper.setText(_("Developer") + ": ");
  addChild(&mLblDeveloper);
  addChild(&mDeveloper);
  mLblPublisher.setText(_("Publisher") + ": ");
  addChild(&mLblPublisher);
  addChild(&mPublisher);
  mLblGenre.setText(_("Genre") + ": ");
  addChild(&mLblGenre);
  addChild(&mGenre);
  mLblPlayers.setText(_("Players") + ": ");
  addChild(&mLblPlayers);
  addChild(&mPlayers);
  mLblLastPlayed.setText(_("Last played") + ": ");
  addChild(&mLblLastPlayed);
  mLastPlayed.setDisplayMode(DateTimeComponent::Display::RelativeToNow);
  addChild(&mLastPlayed);
  mLblPlayCount.setText(_("Times played") + ": ");
  addChild(&mLblPlayCount);
  addChild(&mPlayCount);
  if (mSystem.HasFavoritesInTheme())
  {
    mLblFavorite.setText(_("Favorite") + ": ");
    addChild(&mLblFavorite);
    addChild(&mFavorite);
  }

  for (int i = 4; --i >= 0; )
  {
    auto* img = new ImageComponent(mWindow);
    addChild(img); // normalised functions required to be added first
    img->setDefaultZIndex(40);
    img->setThemeDisabled(true);
    mRegions.push_back(img);
  }

  mDescContainer.setPosition(mSize.x() * padding, mSize.y() * 0.65f);
  mDescContainer.setSize(mSize.x() * (0.50f - 2 * padding), mSize.y() - mDescContainer.getPosition().y());
  mDescContainer.setDefaultZIndex(40);
  addChild(&mDescContainer);

  mDescription.setFont(Font::get(FONT_SIZE_SMALL));
  mDescription.setSize(mDescContainer.getSize().x(), 0);
  mDescContainer.addChild(&mDescription);

  initMDLabels();
  initMDValues();
  updateInfoPanel();
}

void DetailedGameListView::onThemeChanged(const ThemeData& theme)
{
  ISimpleGameListView::onThemeChanged(theme);
  mList.applyTheme(theme, getName(), "gamelist", ThemeProperties::All);
  // Set color 2/3 50% transparent of color 0/1
  mList.setColor(2, (mList.Color(0) & 0xFFFFFF00) | ((mList.Color(0) & 0xFF) >> 1));
  mList.setColor(3, (mList.Color(1) & 0xFFFFFF00) | ((mList.Color(1) & 0xFF) >> 1));
  sortChildren();

  for (int i = 0; i < (int) mRegions.size(); i++)
    mRegions[i]->applyTheme(theme, getName(), String("md_region").Append(i + 1).c_str(),
                                  ThemeProperties::Position | ThemeProperties::Size | ThemeProperties::ZIndex | ThemeProperties::Path);

  mImage.applyTheme(theme, getName(), "md_image", ThemeProperties::Position | ThemeProperties::Size | ThemeProperties::ZIndex | ThemeProperties::Rotation);
  mNoImage.applyTheme(theme, getName(), "md_image", ThemeProperties::Position | ThemeProperties::Size | ThemeProperties::ZIndex | ThemeProperties::Rotation);
  mNoImage.applyTheme(theme, getName(), "default_image_path", ThemeProperties::Path);
  mVideo.applyTheme(theme, getName(), "md_video", ThemeProperties::Position | ThemeProperties::Size | ThemeProperties::ZIndex | ThemeProperties::Rotation);

  initMDLabels();
  std::vector<TextComponent*> labels = getMDLabels();
  std::vector<String> names({
                                   "md_lbl_rating",
                                   "md_lbl_releasedate",
                                   "md_lbl_developer",
                                   "md_lbl_publisher",
                                   "md_lbl_genre",
                                   "md_lbl_players",
                                   "md_lbl_lastplayed",
                                   "md_lbl_playcount"
                                 });

  if (mSystem.HasFavoritesInTheme())
  {
    names.push_back("md_lbl_favorite");
  }

  assert(names.size() == labels.size());
  for (unsigned int i = 0; i < (unsigned int)labels.size(); i++)
  {
    labels[i]->applyTheme(theme, getName(), names[i], ThemeProperties::All);
  }

  initMDValues();
  std::vector<Component*> values = getMDValues();
  names = {
    "md_rating",
    "md_releasedate",
    "md_developer",
    "md_publisher",
    "md_genre",
    "md_players",
    "md_lastplayed",
    "md_playcount"
  };

  if (mSystem.HasFavoritesInTheme())
  {
    names.push_back("md_favorite");
  }

  names.push_back("md_folder_name");
  values.push_back(&mFolderName);

  assert(names.size() == values.size());
  for (unsigned int i = 0; i < (unsigned int)values.size(); i++)
  {
    values[i]->applyTheme(theme, getName(), names[i], ThemeProperties::All ^ ThemeProperties::Text);
  }

  mDescContainer.applyTheme(theme, getName(), "md_description", ThemeProperties::Position | ThemeProperties::Size | ThemeProperties::ZIndex);
  mDescription.setSize(mDescContainer.getSize().x(), 0);
  mDescription.applyTheme(theme, getName(), "md_description",
                          ThemeProperties::All ^ (ThemeProperties::Position | ThemeProperties::Size | ThemeProperties::Origin | ThemeProperties::Text));
  mBusy.SetFont(mDescription.getFont());

  if (theme.isFolderHandled())
  {
    mFolderName.applyTheme(theme, getName(), "md_folder_name", ThemeProperties::All);
    for (int i = 0; i < (int) mFolderContent.size(); i++)
    {
      String folderImage("md_folder_image_"); folderImage.Append(i);
      mFolderContent[i]->applyTheme(theme, getName(), folderImage,
                                    ThemeProperties::Position | ThemeProperties::Size | ThemeProperties::ZIndex | ThemeProperties::Rotation);
    }
  }
  else
  {
    // backward compatibility
    auto size = mImage.getSize().isZero() ? mImage.getTargetSize() : mImage.getSize();
    float minSize = Math::min(size.x(), size.y());
    float left = mImage.getPosition().x() - mImage.getOrigin().x() * minSize;
    float top = mImage.getPosition().y() - mImage.getOrigin().y() * minSize;

    mFolderName.setPosition(left, top);
    mFolderName.setZIndex(40);
    mFolderName.setFont(Font::get(FONT_SIZE_EXTRASMALL));
    mFolderName.setColor(0xFFFFFFFF);

    const unsigned int grid = 3; // 3 x 3
    const float relativeMarge = 0.1f;

    float labelHeight = 2.5f * Font::get(FONT_SIZE_EXTRASMALL)->getLetterHeight();
    top += labelHeight;

    minSize = Math::min(size.x(), size.y() - labelHeight);

    const float imgSize = minSize / (grid + 2.0f * relativeMarge);

    // centering with unused space
    left += 0.5f * (minSize - grid * imgSize - 2.0f * relativeMarge);

    for (unsigned int x = 0; x < grid; x++)
    {
      for (unsigned int y = 0; y < grid; y++)
      {
        ImageComponent* img = mFolderContent[x + y * grid];
        img->setMaxSize(imgSize, imgSize);
        img->setPosition(left + imgSize * img->getOrigin().x() + (float)x * (1 + relativeMarge) * imgSize,
                         top + imgSize * img->getOrigin().y() + (float)y * (1 + relativeMarge) * imgSize);
        img->setZIndex(30);
      }
    }
  }

  sortChildren();
}

void DetailedGameListView::initMDLabels()
{
  std::vector<TextComponent*> components = getMDLabels();

  const unsigned int colCount = 2;
  const unsigned int rowCount = (unsigned int) (components.size() / 2);

  Vector3f start(mSize.x() * 0.01f, mSize.y() * 0.625f, 0.0f);

  const float colSize = (mSize.x() * 0.48f) / colCount;
  const float rowPadding = 0.01f * mSize.y();

  for (unsigned int i = 0; i < (unsigned int)components.size(); i++)
  {
    const unsigned int row = i % rowCount;
    Vector3f pos(0.0f, 0.0f, 0.0f);
    if (row == 0)
    {
      pos = start + Vector3f(colSize * ((float)i / (float)rowCount), 0, 0);
    }
    else
    {
      // work from the last component
      Component* lc = components[i - 1];
      pos = lc->getPosition() + Vector3f(0, lc->getSize().y() + rowPadding, 0);
    }

    components[i]->setFont(Font::get(FONT_SIZE_SMALL));
    components[i]->setPosition(pos);
    components[i]->setDefaultZIndex(40);
  }
}

void DetailedGameListView::initMDValues()
{
  std::vector<TextComponent*> labels = getMDLabels();
  std::vector<Component*> values = getMDValues();

  std::shared_ptr<Font> defaultFont = Font::get(FONT_SIZE_SMALL);
  mRating.setSize(defaultFont->getHeight() * 5.0f, defaultFont->getHeight());
  mReleaseDate.setFont(defaultFont);
  mDeveloper.setFont(defaultFont);
  mPublisher.setFont(defaultFont);
  mGenre.setFont(defaultFont);
  mPlayers.setFont(defaultFont);
  mLastPlayed.setFont(defaultFont);
  mPlayCount.setFont(defaultFont);
  mFavorite.setFont(defaultFont);

  float bottom = 0.0f;

  const float colSize = (mSize.x() * 0.48f) / 2;
  for (unsigned int i = 0; i < (unsigned int)labels.size(); i++)
  {
    const float heightDiff = (labels[i]->getSize().y() - values[i]->getSize().y()) / 2;
    values[i]->setPosition(labels[i]->getPosition() + Vector3f(labels[i]->getSize().x(), heightDiff, 0));
    values[i]->setSize(colSize - labels[i]->getSize().x(), values[i]->getSize().y());
    values[i]->setDefaultZIndex(40);

    float testBot = values[i]->getPosition().y() + values[i]->getSize().y();
    if (testBot > bottom)
      bottom = testBot;
  }

  mDescContainer.setPosition(mDescContainer.getPosition().x(), bottom + mSize.y() * 0.01f);
  mDescContainer.setSize(mDescContainer.getSize().x(), mSize.y() - mDescContainer.getPosition().y());
}

void DetailedGameListView::DoUpdateGameInformation(bool update)
{
  FileData* file = (mList.size() == 0 || mList.isScrolling()) ? nullptr : mList.getSelected();

  if (file == nullptr)
  {
    VideoEngine::Instance().StopVideo(false);
    fadeOut(getFolderComponents(), true);
    fadeOut(getGameComponents(), true);
  }
  else
  {
    const bool isFolder = file->IsFolder();
    const bool hasImage = !file->Metadata().Image().IsEmpty();

    if (hasImage && isFolder)
    {
      setScrapedFolderInfo(file);
      switchToFolderScrapedDisplay();
    }
    else
    {
       if (isFolder)
       {
         setFolderInfo((FolderData*) file);
         for(int i = (int)mRegions.size(); --i >= 0; )
           mRegions[i]->setImage(Path());
       }
       else
        setGameInfo(file, update);
      switchDisplay(!isFolder);
    }
  }

  mWindow.UpdateHelpSystem();
}

bool DetailedGameListView::switchToFolderScrapedDisplay()
{
  fadeOut(getGameComponents(false), true);
  fadeOut(getFolderComponents(), true);
  fadeOut(getScrapedFolderComponents(), false);
  return true;
}

bool DetailedGameListView::switchDisplay(bool isGame)
{
  fadeOut(getGameComponents(), !isGame);
  fadeOut(getFolderComponents(), isGame);
  return true;
}

std::vector<Component*> DetailedGameListView::getFolderComponents()
{
  std::vector<Component*> comps;
  for (auto* img: mFolderContent)
  {
    comps.push_back(img);
  }
  comps.push_back(&mFolderName);
  return comps;
}

std::vector<Component*> DetailedGameListView::getGameComponents(bool includeMainComponents)
{
  std::vector<Component*> comps = getMDValues();
  if (includeMainComponents)
  {
    comps.push_back(&mNoImage);
    comps.push_back(&mImage);
    comps.push_back(&mVideo);
    comps.push_back(&mDescription);
  }
  std::vector<TextComponent*> labels = getMDLabels();
  comps.insert(comps.end(), labels.begin(), labels.end());
  return comps;
}

std::vector<Component*> DetailedGameListView::getScrapedFolderComponents()
{
  std::vector<Component*> comps;
  comps.push_back(&mNoImage);
  comps.push_back(&mImage);
  comps.push_back(&mVideo);
  comps.push_back(&mDescription);
  return comps;
}

void DetailedGameListView::setFolderInfo(FolderData* folder)
{
  FileData::List games = folder->GetAllDisplayableItemsRecursively(false, folder->System().Excludes());
  String gameCount(_N("%i GAME AVAILABLE", "%i GAMES AVAILABLE", (int)games.size()));
  gameCount.Replace("%i", String((int)games.size()));
  mFolderName.setText(folder->Name() + " - " + gameCount);

  unsigned char idx = 0;

  for (FileData* game : games)
  {
    if (game->HasThumbnailOrImage())
    {
      mFolderContent[idx]->setImage(game->ThumbnailOrImagePath());
      if (++idx == mFolderContent.size())
        break;
    }
  }
  for (int i = idx; i < (int) mFolderContent.size(); i++)
  {
    mFolderContent[i]->setImage(Path());
  }
  // Kill video on multi-thumbnail folder
  mVideo.setVideo(Path::Empty, 0, 0);
}

void DetailedGameListView::SetImageFading(FileData* game, bool update)
{
  // Setup
  mNoImage.setImage(Path(":/no_image.png"));
  mNoImage.setThemeDisabled(false);

  bool imageExists = game->Metadata().Image().Exists();
  if (game->IsFolder())
  {
    // Just set the image if it exists - if not, a folder preview is displayed
    mImage.setImage(game->Metadata().Image());
    // Let no image display if the image does not exists
    mNoImage.setThemeDisabled(imageExists);
    mNoImage.setOpacity(255);
  }
  else
  {
    // Check equality with previous image
    bool didntExist = !mImage.getImagePath().Exists();
    // Set new image
    mImage.setImage(imageExists ? game->Metadata().Image() : Path(":/no_image.png"));
    // if updating from no image, let's fade
    if (update && imageExists && didntExist) // Start fading!
    {
      mFadeBetweenImage = 255;
      mImage.setOpacity(255 - mFadeBetweenImage);
      mNoImage.setOpacity(mFadeBetweenImage);
    }
    else // Just disable no image if the image exists
    {
      mNoImage.setThemeDisabled(imageExists);
      mNoImage.setOpacity(255);
    }
  }
}

void DetailedGameListView::setGameInfo(FileData* file, bool update)
{
  if(mSystem.Name() != "imageviewer")
    setRegions(file);

  mRating.setValue(file->Metadata().RatingAsString());
  mReleaseDate.setValue(file->Metadata().ReleaseDate());
  mDeveloper.setValue(file->Metadata().Developer().empty() ? _("UNKNOWN") : file->Metadata().Developer());
  mPublisher.setValue(file->Metadata().Publisher().empty() ? _("UNKNOWN") : file->Metadata().Publisher());
  mGenre.setValue(file->Metadata().Genre().empty() ? _("NONE") : file->Metadata().Genre());
  mPlayers.setValue(file->Metadata().PlayersAsString());
  mLastPlayed.setValue(file->Metadata().LastPlayed());
  mPlayCount.setValue(file->Metadata().PlayCountAsString());
  mFavorite.setValue(file->Metadata().Favorite() ? _("YES") : _("NO"));

  int videoDelay = (int) mSettings.AsUInt("emulationstation.videosnaps.delay", VideoComponent::DEFAULT_VIDEODELAY);
  int videoLoop  = (int) mSettings.AsUInt("emulationstation.videosnaps.loop", VideoComponent::DEFAULT_VIDEOLOOP);

  SetImageFading(file, update);

  mBusy.setPosition(mImage.getPosition());
  mBusy.setSize(mImage.getSize());
  mBusy.setOrigin(mImage.getOrigin());

  if (!mSettings.AsBool("system.secondminitft.enabled", false) ||
      !mSettings.AsBool("system.secondminitft.disablevideoines", false))
    mVideo.setVideo(file->Metadata().Video(), videoDelay, videoLoop, AudioModeTools::CanDecodeVideoSound());

  { LOG(LogDebug) << "[GamelistView] Set video " << file->Metadata().Video().ToString() << " for " << file->Metadata().Name() << " => " << file->RomPath().ToString(); }
  mDescription.setText(GetDescription(*file));
  mDescContainer.reset();
}

void DetailedGameListView::setScrapedFolderInfo(FileData* file)
{
  SetImageFading(file, false);
  mVideo.setVideo(Path::Empty, 0, 0);
  mDescription.setText(GetDescription(*file));
  mDescContainer.reset();
}

void DetailedGameListView::fadeOut(const std::vector<Component*>& comps, bool fadingOut)
{
  for (auto* comp : comps)
  {
    // an animation is playing
    //   then animate if reverse != fadingOut
    // an animation is not playing
    //   then animate if opacity != our target opacity
    if ((comp->isAnimationPlaying(0) && comp->isAnimationReversed(0) != fadingOut) ||
        (!comp->isAnimationPlaying(0) && comp->getOpacity() != (fadingOut ? 0 : 255)))
    {
      auto func = [comp](float t)
      {
        comp->setOpacity((unsigned char) (lerp<float>(0.0f, 1.0f, t) * 255));
      };
      comp->setAnimation(new LambdaAnimation(func, 150), 0, nullptr, fadingOut);
    }
  }
}

void DetailedGameListView::launch(FileData* game)
{
  VideoEngine::Instance().StopVideo(true);
  mVideo.setVideo(Path::Empty, 0, 0);

  Vector3f target(Renderer::Instance().DisplayWidthAsFloat() / 2.0f, Renderer::Instance().DisplayHeightAsFloat() / 2.0f, 0);
  if (mImage.hasImage())
    target.Set(mImage.getCenter().x(), mImage.getCenter().y(), 0);

  ViewController::Instance().Launch(game, GameLinkedData(), target);
}

// element order need to follow the one in onThemeChanged
std::vector<TextComponent*> DetailedGameListView::getMDLabels()
{
  std::vector<TextComponent*> ret;
  ret.push_back(&mLblRating);
  ret.push_back(&mLblReleaseDate);
  ret.push_back(&mLblDeveloper);
  ret.push_back(&mLblPublisher);
  ret.push_back(&mLblGenre);
  ret.push_back(&mLblPlayers);
  ret.push_back(&mLblLastPlayed);
  ret.push_back(&mLblPlayCount);
  if (mSystem.HasFavoritesInTheme())
  {
    ret.push_back(&mLblFavorite);
  }
  return ret;
}

// element order need to follow the one in onThemeChanged
std::vector<Component*> DetailedGameListView::getMDValues()
{
  std::vector<Component*> ret;
  ret.push_back(&mRating);
  ret.push_back(&mReleaseDate);
  ret.push_back(&mDeveloper);
  ret.push_back(&mPublisher);
  ret.push_back(&mGenre);
  ret.push_back(&mPlayers);
  ret.push_back(&mLastPlayed);
  ret.push_back(&mPlayCount);
  if (mSystem.HasFavoritesInTheme())
  {
    ret.push_back(&mFavorite);
  }
  return ret;
}

void DetailedGameListView::Update(int deltatime)
{
  ISimpleGameListView::Update(deltatime);

  mBusy.Enable(mIsScraping);
  mBusy.Update(deltatime);

  if (mFadeBetweenImage >= 0)
  {
    mFadeBetweenImage -= deltatime;
    int f = mFadeBetweenImage < 0 ? 0 : mFadeBetweenImage;
    mImage.setOpacity(255 - f);
    mNoImage.setOpacity(f);
  }

  // Cancel video
  if (mList.isScrolling())
    mVideo.setVideo(Path::Empty, 0, 0);

  if (!mSystem.IsScreenshots())
  {
    // Need busy animation?
    ScraperSeamless& scraper = ScraperSeamless::Instance();
    FileData* game = getCursor();
    mIsScraping = false;
    if (game != nullptr)
      if (game->IsGame())
      {
        // Currently scraping?
        mIsScraping = (scraper.HowLong(game) > sMaxScrapingTimeBeforeBusyAnim);
        // Or start scraping?
        if (mElapsedTimeOnGame >= 0) // Valid timer?
          if (mElapsedTimeOnGame += deltatime; mElapsedTimeOnGame > sMaxHoveringTimeBeforeScraping) // Enough time on game?
          {
            // Shutdown timer for the current game
            mElapsedTimeOnGame = -1;
            // Push game into the seamless scraper
            scraper.Push(game, this);
          }
      }
  }
}

void DetailedGameListView::Render(const Transform4x4f& parentTrans)
{
  Transform4x4f trans = parentTrans * getTransform();

  renderChildren(trans);

  Renderer::SetMatrix(trans);
  //Renderer::DrawRectangle(mBusy.getPosition().x(), mBusy.getPosition().y(), mBusy.getSize().x(), mBusy.getSize().y(), 0x00000080);
  mBusy.Render(trans);
}

void DetailedGameListView::OverlayApply(const Vector2f& position, const Vector2f& size, FileData* const& data, unsigned int& color)
{
  (void)color;
  int w = Math::roundi(DetailedGameListView::OverlayGetRightOffset(data));
  if (w != 0)
  {
    int drawn = 1;
    int flagHeight = Math::roundi(mList.getFont()->getHeight(1.f));

    for (int r = Regions::RegionPack::sMaxRegions; --r >= 0;)
      if (Regions::GameRegions region = data->Metadata().Region().Regions[r]; region != Regions::GameRegions::Unknown)
      {
        std::shared_ptr<TextureResource>* flag = mRegionToTextures.try_get(region);
        if (flag == nullptr)
        {
          // Load flag
          std::shared_ptr<TextureResource> texture = TextureResource::get(Path(":/regions/" + Regions::GameRegionsFromEnum(region) + ".svg"), false, true, true);
          mRegionToTextures.insert(region, texture);
          flag = mRegionToTextures.try_get(region);
        }
        // Draw
        int flagWidth = (int) ((float) flagHeight * (float) (*flag)->width() / (float) (*flag)->height());
        int y = Math::roundi((size.y() - (float) flagHeight) / 2.f) + (int)position.y();
        int x = ((int)size.x() - (2 + Math::roundi(mList.getHorizontalMargin()) + flagWidth) * drawn) + (int)position.x();
        Renderer::DrawTexture(**flag, x, y, flagWidth, flagHeight, data == getCursor() ? (unsigned char)255 : (unsigned char)128);
        drawn++;
      }
  }
}

float DetailedGameListView::OverlayGetRightOffset(FileData* const& data)
{
  return ((mList.getFont()->getHeight(1.f) * (71.f / 48.f) + 2.f) * (float)data->Metadata().Region().Count()) + 2.f + mList.getHorizontalMargin();
}

DetailedGameListView::~DetailedGameListView()
{
  for(int i = (int)mFolderContent.size(); --i >= 0; )
    delete mFolderContent[i];
  mFolderContent.clear();

  for(int i = (int)mRegions.size(); --i >= 0; )
    delete mRegions[i];
  mRegions.clear();
}

void DetailedGameListView::setRegions(FileData* file)
{
  String::List regionList = file->Regions().SplitQuoted(',');

  // reinit non used region flags
  for(unsigned long idx = 0; idx < mRegions.size(); idx++)
  {
    if(regionList.size() <= idx)
      mRegions[idx]->setImage(Path());
  }

  if(regionList.empty())
  {
    mRegions[0]->setImage(Path());
    return;
  }

  int i = 0;
  for(auto& region: regionList)
  {
    if(mRegions[i]->getImagePath() != Path(":/regions/" + region + ".svg"))
      mRegions[i]->setImage(Path(":/regions/" + region + ".svg"));
    i++;
  }
}

void DetailedGameListView::ScrapingStageCompleted(FileData* game, Stage stage, MetadataType changes)
{
  // Got result, from the seamless scraper, update game data!
  if (game == getCursor())
    switch(stage)
    {
      case Stage::Text:
      {
        DoUpdateGameInformation(false);
        // Game name
        if ((changes & MetadataType::Name) != 0)
          mList.changeTextAt(mList.getCursorIndex(), GetDisplayName(*game));
        break;
      }
      case Stage::Images:
      {
        if ((changes & (MetadataType::Image | MetadataType::Thumbnail)) != 0)
          DoUpdateGameInformation(true);
        break;
      }
      case Stage::Video:
      {
        if ((changes & MetadataType::Video) != 0)
          DoUpdateGameInformation(false);
        break;
      }
      case Stage::Extra: break; // Nothing to do with extra data for now
      case Stage::Completed: RecalboxStorageWatcher::CheckStorageFreeSpace(mWindow, mSystemManager.GetMountMonitor(), game->RomPath()); break;
      default: break;
    }
  else
    if (stage == Stage::Text)
      if ((changes & MetadataType::Name) != 0)
        for(int i = mList.Count(); -- i>= 0; )
          if (mList.getObjectAt(i) == game)
            mList.changeTextAt(i, GetDisplayName(*game));

  { LOG(LogDebug) << "[Scraper] Scraper stage: " << (int)stage; }
}

// Called when a game is selected in the list whatever how
void DetailedGameListView::OnGameSelected()
{
  // Reset seamless scraping timer
  FileData* game = getCursor();
  if (game != nullptr && game->IsGame()) mElapsedTimeOnGame = 0;

  // Update current game information
  DoUpdateGameInformation(false);
}

String DetailedGameListView::getItemIcon(const FileData& item)
{
  // Crossed out eye for hidden things
  if (item.Metadata().Hidden()) return "\uF070 ";
  // System icon, for Favorite games
  if ((item.IsGame()) && (mSystem.IsVirtual() || item.Metadata().Favorite()))
    return item.System().Descriptor().IconPrefix();
  // Open folder for folders
  if (item.IsFolder()) return "\uF07C ";

  return String();
}

String DetailedGameListView::GetDisplayName(FileData& game)
{
  // Select Icon
  String result = getItemIcon(game);
  // Get name
  result.Append(RecalboxConf::Instance().GetDisplayByFileName() ? game.Metadata().RomFileOnly().ToString() : game.Name());
  return result;
}

void DetailedGameListView::populateList(const FolderData& folder)
{
  mPopulatedFolder = &folder;
  mList.clear();
  mHeaderText.setText(mSystem.FullName());

  // Default filter
  FileData::Filter includesFilter = FileData::Filter::Normal | FileData::Filter::Favorite;
  // Favorites only?
  if (RecalboxConf::Instance().GetFavoritesOnly()) includesFilter = FileData::Filter::Favorite;

  // Get items
  bool flatfolders = mSystem.IsAlwaysFlat() || (RecalboxConf::Instance().GetSystemFlatFolders(mSystem));
  FileData::List items;
  if (flatfolders) folder.GetItemsRecursivelyTo(items, includesFilter, mSystem.Excludes(), false);
  else folder.GetItemsTo(items, includesFilter, mSystem.Excludes(), true);

  // Check emptyness
  if (items.empty()) items.push_back(&mEmptyListItem); // Insert "EMPTY SYSTEM" item

  // Sort
  FileSorts::SortSets set = mSystem.IsVirtual() ? FileSorts::SortSets::MultiSystem :
                            mSystem.Descriptor().IsArcade() ? FileSorts::SortSets::Arcade :
                            FileSorts::SortSets::SingleSystem;
  FileSorts::Sorts sort = FileSorts::Clamp(RecalboxConf::Instance().GetSystemSort(mSystem), set);
  FolderData::Sort(items, FileSorts::Comparer(sort), FileSorts::IsAscending(sort));

  // Region filtering?
  Regions::GameRegions currentRegion = Regions::Clamp((Regions::GameRegions)RecalboxConf::Instance().GetSystemRegionFilter(mSystem));
  bool activeRegionFiltering = false;
  if (currentRegion != Regions::GameRegions::Unknown)
  {
    Regions::List availableRegion = AvailableRegionsInGames(items);
    // Check if our region is in the available ones
    for(Regions::GameRegions region : availableRegion)
    {
      activeRegionFiltering = (region == currentRegion);
      if (activeRegionFiltering) break;
    }
  }

  // Tate flag
  bool onlyTate = RecalboxConf::Instance().GetTateOnly();

  // Add to list
  //mList.reserve(items.size()); // TODO: Reserve memory once
  for (FileData* fd : items)
  {
    // Region filtering?
    int colorIndexOffset = 0;
    if (activeRegionFiltering)
      if (!Regions::IsIn4Regions(fd->Metadata().Region().Pack, currentRegion))
        colorIndexOffset = 2;
    // Tate filtering
    if (onlyTate && fd->Metadata().Rotation() == RotationType::None) continue;
    // Store
    mList.add(GetDisplayName(*fd), fd, colorIndexOffset + (fd->IsFolder() ? 1 : 0), false);
  }
}

void DetailedGameListView::setCursorIndex(int index)
{
  if (index >= mList.size()) index = mList.size() - 1;
  if (index < 0) index = 0;

  mList.setCursorIndex(index);
}
void DetailedGameListView::setCursorStack(FileData* cursor)
{
  std::stack<FolderData*> reverseCursorStack;

  Path systemTopFolderPath = cursor->TopAncestor().RomPath();
  FolderData* parent = cursor->Parent();

  if (systemTopFolderPath == parent->RomPath())
    return;

  while(systemTopFolderPath != parent->RomPath())
  {
    reverseCursorStack.push(parent);
    parent = parent->Parent();
  }

  while(!reverseCursorStack.empty())
  {
    mCursorStack.push(reverseCursorStack.top());
    reverseCursorStack.pop();

    FolderData& tmp = !mCursorStack.empty() ? *mCursorStack.top() : mSystem.MasterRoot();
    populateList(tmp);
  }
}

void DetailedGameListView::setCursor(FileData* cursor)
{
  if(!mList.setCursor(cursor, 0))
  {
    populateList(mSystem.MasterRoot());
    mList.setCursor(cursor);

    // update our cursor stack in case our cursor just got set to some folder we weren't in before
    if(mCursorStack.empty() || mCursorStack.top() != cursor->Parent())
    {
      std::stack<FolderData*> tmp;
      FolderData* ptr = cursor->Parent();
      while((ptr != nullptr) && !ptr->IsRoot())
      {
        tmp.push(ptr);
        ptr = ptr->Parent();
      }

      // flip the stack and put it in mCursorStack
      mCursorStack = std::stack<FolderData*>();
      while(!tmp.empty())
      {
        mCursorStack.push(tmp.top());
        tmp.pop();
      }
    }
  }
}

void DetailedGameListView::removeEntry(FileData* fileData)
{
  FileData::TopLevelFilter filter = FileData::BuildTopLevelFilter();
  if (!mCursorStack.empty() && !fileData->Parent()->HasVisibleGame(filter))
  {
    // remove current folder from stack
    mCursorStack.pop();

    FolderData& cursor = !mCursorStack.empty() ? *mCursorStack.top() : mSystem.MasterRoot();
    populateList(cursor);
  }

  int cursorIndex = getCursorIndex();
  refreshList();
  if(cursorIndex > 0) setCursorIndex(cursorIndex - 1);
}

Regions::List DetailedGameListView::AvailableRegionsInGames()
{
  bool regionIndexes[256];
  memset(regionIndexes, 0, sizeof(regionIndexes));
  // Run through all games
  for(int i = (int)mList.size(); --i >= 0; )
  {
    const FileData& fd = *mList.getObjectAt(i);
    unsigned int fourRegions = fd.Metadata().Region().Pack;
    // Set the 4 indexes corresponding to all 4 regions (Unknown regions will all point to index 0)
    regionIndexes[(fourRegions >>  0) & 0xFF] = true;
    regionIndexes[(fourRegions >>  8) & 0xFF] = true;
    regionIndexes[(fourRegions >> 16) & 0xFF] = true;
    regionIndexes[(fourRegions >> 24) & 0xFF] = true;
  }
  // Rebuild final list
  Regions::List list;
  for(int i = 0; i < (int)sizeof(regionIndexes); ++i )
    if (regionIndexes[i])
      list.push_back((Regions::GameRegions)i);
  // Only unknown region?
  if (list.size() == 1 && regionIndexes[0])
    list.clear();
  return list;
}

Regions::List DetailedGameListView::AvailableRegionsInGames(FileData::List& fdList)
{
  bool regionIndexes[256];
  memset(regionIndexes, 0, sizeof(regionIndexes));
  // Run through all games
  for(const FileData* fd : fdList)
  {
    unsigned int fourRegions = fd->Metadata().Region().Pack;
    // Set the 4 indexes corresponding to all 4 regions (Unknown regions will all point to index 0)
    regionIndexes[(fourRegions >>  0) & 0xFF] = true;
    regionIndexes[(fourRegions >>  8) & 0xFF] = true;
    regionIndexes[(fourRegions >> 16) & 0xFF] = true;
    regionIndexes[(fourRegions >> 24) & 0xFF] = true;
  }
  // Rebuild final list
  Regions::List list;
  for(int i = (int)sizeof(regionIndexes); --i >= 0; )
    if (regionIndexes[i])
      list.push_back((Regions::GameRegions)i);
  // Only unknown region?
  if (list.size() == 1 && regionIndexes[0])
    list.clear();
  return list;
}

void DetailedGameListView::RefreshItem(FileData* game)
{
  if (game == nullptr || !game->IsGame()) { LOG(LogError) << "[DetailedGameListView] Trying to refresh null or empty item"; return; }

  int index = mList.Lookup(game);
  if (index < 0) { LOG(LogError) << "[DetailedGameListView] Trying to refresh a not found item"; return; }
  mList.changeTextAt(index, GetDisplayName(*game));
  if (mList.getCursorIndex() == index) DoUpdateGameInformation(true);
}

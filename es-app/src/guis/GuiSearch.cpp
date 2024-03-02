//
// Created by xizor on 05/10/2019.
//

#include <components/ButtonComponent.h>
#include <components/MenuComponent.h>
#include <guis/GuiTextEditPopupKeyboard.h>
#include <guis/GuiArcadeVirtualKeyboard.h>
#include <views/ViewController.h>
#include <RecalboxConf.h>
#include "components/ScrollableContainer.h"
#include "GuiSearch.h"
#include "GuiNetPlayHostPasswords.h"
#include <VideoEngine.h>
#include <dirent.h>
#include <guis/GuiDownloadFile.h>

#define BUTTON_GRID_VERT_PADDING Renderer::Instance().DisplayHeightAsFloat() * 0.025f
#define BUTTON_GRID_HORIZ_PADDING 10

#define TITLE_HEIGHT (mTitle->getFont()->getLetterHeight() + Renderer::Instance().DisplayHeightAsFloat()*0.0437f )

#define MAX_LINE 2048

GuiSearch::GuiSearch(WindowManager& window, SystemManager& systemManager)
  : Gui(window)
  , mSystemManager(systemManager)
  , mBackground(window, Path(":/frame.png"))
  , mGrid(window, Vector2i(3, 3))
  , mList(nullptr)
  , mSystemData(nullptr)
  , mJustOpen(true)
{
  addChild(&mBackground);
  addChild(&mGrid);

  mMenuTheme = MenuThemeData::getInstance()->getCurrentTheme();

  mBackground.setImagePath(mMenuTheme->menuBackground.path);
  mBackground.setCenterColor(mMenuTheme->menuBackground.color);
  mBackground.setEdgeColor(mMenuTheme->menuBackground.color);

  initGridsNStuff();

  updateSize();
  setPosition((Renderer::Instance().DisplayWidthAsFloat() - mSize.x()) / 2,
              (Renderer::Instance().DisplayHeightAsFloat() - mSize.y()) / 2);

  PopulateGrid("");
}

void GuiSearch::initGridsNStuff()
{
  //init Title
  mTitle = std::make_shared<TextComponent>(mWindow, _("SEARCH") + " : ", mMenuTheme->menuText.font,
                                           mMenuTheme->menuText.color, TextAlignment::Right);
  mGrid.setEntry(mTitle, Vector2i(0, 0), false, true, Vector2i(1, 1));

  //init search textfield
  mSearch = std::make_shared<TextEditComponent>(mWindow);
  mGrid.setEntry(mSearch, Vector2i(0, 1), false, false, Vector2i(3, 1));

  //init search option selector
  mSearchChoices = std::make_shared<OptionListComponent<FolderData::FastSearchContext> >(mWindow, _("SEARCH BY"), false);

  mSearchChoices->add(_("Name"), FolderData::FastSearchContext::Name, true);
  mSearchChoices->add(_("Description"), FolderData::FastSearchContext::Description, false);
  mSearchChoices->add(_("DEVELOPER"), FolderData::FastSearchContext::Developer, false);
  mSearchChoices->add(_("PUBLISHER"), FolderData::FastSearchContext::Publisher, false);
  mSearchChoices->add(_("FILENAME"), FolderData::FastSearchContext::Path, false);
  mSearchChoices->add(_("ALL"), FolderData::FastSearchContext::All, false);

  mSearchChoices->setChangedCallback([this]{
    mGrid.setColWidthPerc(1, mSearchChoices->getSize().x() / mSize.x());
  });
  mGrid.setEntry(mSearchChoices, Vector2i(1, 0), false, false, Vector2i(1, 1));

  //init big center grid with List and Meta
  mGridMeta = std::make_shared<ComponentGrid>(mWindow, Vector2i(4, 3));
  mGridMeta->setEntry(std::make_shared<Component>(mWindow), Vector2i(1, 0), false, false, Vector2i(1, 3));
  mGridMeta->setEntry(std::make_shared<Component>(mWindow), Vector2i(2, 0), false, false, Vector2i(2, 1));
  mText = std::make_shared<TextComponent>(mWindow, "", mMenuTheme->menuText.font,
                                          mMenuTheme->menuText.color, TextAlignment::Center);
  mList = std::make_shared<ComponentList>(mWindow);
  mGridMeta->setEntry(mList, Vector2i(0, 0), true, true, Vector2i(1, 3));

  mList->setCursorChangedCallback([this](CursorState state)
                                  {
                                    (void) state;
                                    if (mList->getChildCount()>0)
                                      populateGridMeta(mList->getCursor());
                                  });
  mList->setFocusLostCallback([this]
                              {
                                clear();
                              });
  mList->setFocusGainedCallback([this]
                                {
                                  if (mList->getChildCount()>0)
                                    populateGridMeta(mList->getCursor());
                                });

  mGrid.setEntry(mGridMeta, Vector2i(0, 2), true, true, Vector2i(3, 1));
  mGridMeta->setEntry(mText, Vector2i(0, 0), false, true, Vector2i(4, 3));


  //init grid for Meta with logo, image etc
  mGridLogoAndMD = std::make_shared<ComponentGrid>(mWindow, Vector2i(2, 6));

  // 3x3 grid to center logo
  mGridLogo = std::make_shared<ComponentGrid>(mWindow, Vector2i(3, 3));

  mGridLogoAndMD->setEntry(mGridLogo, Vector2i(0, 0), false, true, Vector2i(2, 1));

  mMDDeveloper = std::make_shared<TextComponent>(mWindow, "", mMenuTheme->menuTextSmall.font,
                                          mMenuTheme->menuTextSmall.color, TextAlignment::Left);
  mMDDeveloperLabel = std::make_shared<TextComponent>(mWindow, "", mMenuTheme->menuTextSmall.font,
                                                      mMenuTheme->menuTextSmall.color, TextAlignment::Left);
  mMDPublisher = std::make_shared<TextComponent>(mWindow, "", mMenuTheme->menuTextSmall.font,
                                                 mMenuTheme->menuTextSmall.color, TextAlignment::Left);
  mMDPublisherLabel = std::make_shared<TextComponent>(mWindow, "", mMenuTheme->menuTextSmall.font,
                                                      mMenuTheme->menuTextSmall.color, TextAlignment::Left);

  mGridLogoAndMD->setEntry(mMDDeveloperLabel, Vector2i(0, 2), false, false, Vector2i(1, 1));
  mGridLogoAndMD->setEntry(mMDDeveloper, Vector2i(1, 2), false, true, Vector2i(1, 1));
  mGridLogoAndMD->setEntry(mMDPublisherLabel, Vector2i(0, 4), false, false, Vector2i(1, 1));
  mGridLogoAndMD->setEntry(mMDPublisher, Vector2i(1, 4), false, true, Vector2i(1, 1));
  mGridLogoAndMD->setEntry(std::make_shared<Component>(mWindow), Vector2i(0, 1), false, false, Vector2i(2, 1));
  mGridLogoAndMD->setEntry(std::make_shared<Component>(mWindow), Vector2i(0, 3), false, false, Vector2i(2, 1));
  mGridLogoAndMD->setEntry(std::make_shared<Component>(mWindow), Vector2i(0, 5), false, false, Vector2i(2, 1));


  mGridMeta->setEntry(mGridLogoAndMD, Vector2i(2, 1), false, true, Vector2i(1, 1));

  mResultSystemLogo = std::make_shared<ImageComponent>(mWindow);
  mGridLogo->setEntry(mResultSystemLogo, Vector2i(1, 1), false, false, Vector2i(1, 1));

  // selected result thumbnail + video
  mResultThumbnail = std::make_shared<ImageComponent>(mWindow);
  mGridMeta->setEntry(mResultThumbnail, Vector2i(3, 1), false, true, Vector2i(1, 1));
  mResultVideo = std::make_shared<VideoComponent>(mWindow);
  mGridMeta->setEntry(mResultVideo, Vector2i(3, 1), false, true, Vector2i(1, 1));

  // selected result desc + container
  mDescContainer = std::make_shared<ScrollableContainer>(mWindow);
  mResultDesc = std::make_shared<TextComponent>(mWindow, "", mMenuTheme->menuText.font, mMenuTheme->menuText.color);
  mDescContainer->addChild(mResultDesc.get());
  mDescContainer->setAutoScroll(true);
  mGridMeta->setEntry(mDescContainer, Vector2i(2, 2), false, true, Vector2i(2, 1));

  //col with mList
  mGridMeta->setColWidthPerc(0, 0.39f);
  //spacer col
  mGridMeta->setColWidthPerc(1, 0.01f);
  //cols with metadata
  mGridMeta->setColWidthPerc(2, 0.30f);
  mGridMeta->setColWidthPerc(3, 0.30f);
  //spacer row
  mGridMeta->setRowHeightPerc(0, 0.0825f);
}

GuiSearch::~GuiSearch()
{
  if (mList)
  {
    mList->clear();
  }
  mSR2.clear();
}

float GuiSearch::getButtonGridHeight() const
{
  auto menuTheme = MenuThemeData::getInstance()->getCurrentTheme();
  return (mButtonGrid ? mButtonGrid->getSize().y() : menuTheme->menuText.font->getHeight() + BUTTON_GRID_VERT_PADDING);
}

bool GuiSearch::ProcessInput(const class InputCompactEvent & event)
{
  if (event.CancelPressed())
  {
    clear();
    Close();
    return true;
  }
  else if (event.R1Pressed())
  {
    mWindow.pushGui(new GuiArcadeVirtualKeyboard(mWindow, _("SEARCH"), mSearch->getValue(), this));
    return true;
  }
  else if (event.ValidPressed())
  {
    if ((size_t)mList->getCursor() < mSearchResults.size()){  // launch selected rom
      clear();
      launch();
    }else{  // download selected rom to system
      if (mList->size() == 0) return true;
      int ii = mList->getCursor() - mSearchResults.size();
      mWindow.pushGui(new GuiDownloadFile(mWindow, mSR2[ii].url, mSR2[ii].system));
    }
    
    return true;
  }
  else if (event.SelectPressed())
  {
    clear();
    GoToGame();
    Close();
    mWindow.CloseAll();
    return true;
  }
  if (!mSearchResults.empty() && (size_t)mList->getCursor() < mSearchResults.size())
  {
    FileData* cursor = mSearchResults[mList->getCursor()];

    // NETPLAY
    if ((event.XPressed()) && (RecalboxConf::Instance().GetNetplayEnabled())
        && cursor->System().Descriptor().HasNetPlayCores())
    {
      clear();
      mWindow.pushGui(new GuiNetPlayHostPasswords(mWindow, *cursor));
      return true;
    }
    if (event.YPressed())
    {
      if (cursor->IsGame() && cursor->System().HasFavoritesInTheme())
      {
        ViewController::Instance().ToggleFavorite(cursor);
        populateGridMeta(mList->getCursor());
      }
      return true;
    }
  }

  if (event.AnyLeftPressed() || event.AnyRightPressed()) return mSearchChoices->ProcessInput(event);

  return Component::ProcessInput(event);
}

void GuiSearch::updateSize()
{
  const float height = Renderer::Instance().DisplayHeightAsFloat() * 0.85f;
  const float width = Renderer::Instance().DisplayWidthAsFloat() * 0.95f;
  setSize(width, height);
}

void GuiSearch::onSizeChanged()
{
  mBackground.fitTo(mSize, Vector3f::Zero(), Vector2f(-32, -32));

  mSearch->setSize(mSize.x() - 12, mSearch->getSize().y());

  // update grid row/col sizes
  mGrid.setColWidthPerc(0, 0.5f);
  mGrid.setRowHeightPerc(0, TITLE_HEIGHT / mSize.y());
  mGrid.setColWidthPerc(1, mSearchChoices->getSize().x() / mSize.x());
  mGrid.setRowHeightPerc(1, (mSearch->getSize().y() + 8) / mSize.y());

  mGrid.setSize(mSize);
}

bool GuiSearch::getHelpPrompts(Help& help)
{
  help.Clear();
  if (AmIOnTopOfScreen())
  {
    help.Set(HelpType::LeftRight, _("SEARCH IN..."))
        .Set(HelpType::UpDown, _("SELECT"))
        .Set(Help::Cancel(), _("BACK"))
        .Set(HelpType::R, _("KEYBOARD"));
    if (!mList->isEmpty() && (size_t)mList->getCursor() < mSearchResults.size() )
      help.Set(Help::Valid(), _("LAUNCH"))
          .Set(HelpType::Select, _("GO TO GAME"))
          .Set(HelpType::X, _("NETPLAY"))
          .Set(HelpType::Y,
               mSearchResults[mList->getCursor()]->Metadata().Favorite() ? _("Remove from favorite") : _("Favorite"));
    else if (!mList->isEmpty() && (size_t)mList->getCursor() >= mSearchResults.size() )
      help.Set(Help::Valid(), "INSTALL");
    return true;
  }
  return false;
}

void GuiSearch::Update(int deltaTime)
{
  Component::Update(deltaTime);

  if (mJustOpen)
  {
    mWindow.pushGui(new GuiArcadeVirtualKeyboard(mWindow, _("SEARCH"), mSearch->getValue(), this));
    mJustOpen = false;
  }
}

void GuiSearch::Render(const Transform4x4f& parentTrans)
{
  Transform4x4f trans = parentTrans * getTransform();

  renderChildren(trans);

  Renderer::SetMatrix(trans);
  Renderer::DrawRectangle(0.f, 0.f, mSize.x(), mSize.y(), 0x00000011);
}

void GuiSearch::PopulateGrid(const String& search)
{
  if (mList)
    mList->clear();

  if (search.length()>2)
  {
    SystemData* systemData = ViewController::Instance().CurrentView() == ViewController::ViewType::GameList ?
                             ViewController::Instance().CurrentSystem() :
                             nullptr;
    mSearchResults =  mSystemManager.SearchTextInGames(mSearchChoices->getSelected(), search, 100, systemData);
    if (!mSearchResults.empty())
    {
      mText->setValue("");
      ComponentListRow row;
      std::shared_ptr<Component> ed;
      for (auto *game : mSearchResults)
      {
        row.elements.clear();
        String gameName(game->System().Descriptor().IconPrefix());
        gameName.Append(game->Metadata().Name());

        ed = std::make_shared<TextComponent>(mWindow, gameName, mMenuTheme->menuText.font,
                                             mMenuTheme->menuText.color,
                                             TextAlignment::Left);
        row.addElement(ed, true);
        row.makeAcceptInputHandler([this] { launch(); });
        mList->addRow(row, false, true);
      }

      clear();
      mGrid.getCellAt(0, 2)->canFocus = true;
      populateGridMeta(0);
    }
    else
    {
      mText->setValue(_("NO RESULTS"));
      clear();
      if (mList) mList->clear();
      //so we can jump to button if list is empty
      mGrid.getCellAt(0, 2)->canFocus = false;
    }
  }
  else
  {
    mText->setValue(_("TYPE AT LEAST 3 CHARACTERS"));
    clear();
    if (mList) mList->clear();
    mGrid.getCellAt(0, 2)->canFocus = false;
  }
}

void GuiSearch::clear()
{
  mResultThumbnail->setImage(Path::Empty);
  mResultVideo->setVideo(Path::Empty, 0, 0);
  mResultSystemLogo->setImage(Path::Empty);
  mResultDesc->setText("");
  mMDPublisherLabel->setText("");
  mMDPublisher->setText("");
  mMDDeveloperLabel->setText("");
  mMDDeveloper->setText("");
}

//called when changing cursor in mList to populate MD
void GuiSearch::populateGridMeta(int i)
{
  // populate MD from (csv) database game to be downloaded
  if (mSearchResults.size() <= (size_t)i){
    int ii = i - mSearchResults.size();

    // clear();
    mMDPublisherLabel->setText("System : ");
    mMDDeveloperLabel->setText("");
    mMDPublisher->setText(mSR2[ii].system);
    // { LOG(LogDebug) << "[GuiSearch] mSR2[ii].system:" << mSR2[ii].system; }
    mMDDeveloper->setText("");
    // { LOG(LogDebug) << "[GuiSearch] mSR2[ii].title:" << mSR2[ii].title; }
    mResultDesc->setText(" " + mSR2[ii].title + "\n " + String(mSR2[ii].desc).Replace('-','\n'));
    // { LOG(LogDebug) << "[GuiSearch] mSR2[ii].desc:" << mSR2[ii].desc; }
    mDescContainer->setSize(mGridMeta->getColWidth(2) + mGridMeta->getColWidth(3), mGridMeta->getRowHeight(2)*0.9f);
    mResultDesc->setSize(mDescContainer->getSize().x(), 0); // make desc text wrap at edge of container

    // system logo retieved from theme
    SystemData* system = mSystemManager.SystemByName(mSR2[ii].system);
    if (system){
      mResultSystemLogo->applyTheme(system->Theme(), "system", "logo", ThemeProperties::Path);
      mGridLogoAndMD->setRowHeightPerc(0, 0.5f);
      mResultSystemLogo->setResize(mGridLogo->getSize().x() * 0.8f, mGridLogo->getSize().y() * 0.8f);
      ResizeGridLogo();
    }
    
    //screenshot
    mResultThumbnail->setImage(Path(":/no_image.png"));
    mResultThumbnail->setResize(mGridMeta->getColWidth(2) * 0.9f, mGridMeta->getRowHeight(1)*0.9f);

    updateHelpPrompts();
    return;
  }

  //screenshot & video
  mResultThumbnail->setImage(mSearchResults[i]->Metadata().Image());
  mResultThumbnail->setResize(mGridMeta->getColWidth(2) * 0.9f, mGridMeta->getRowHeight(1)*0.9f);
  mResultThumbnail->setKeepRatio(true);
  mResultVideo->setVideo(mSearchResults[i]->Metadata().Video(), 2000, 1);
  mResultVideo->setMaxSize(mGridMeta->getColWidth(2) * 0.9f, mGridMeta->getRowHeight(1)*0.9f);

  //system logo retieved from theme
  mResultSystemLogo->applyTheme(mSearchResults[i]->System().Theme(), "system", "logo", ThemeProperties::Path);
  mGridLogoAndMD->setRowHeightPerc(0, 0.5f);
  mResultSystemLogo->setResize(mGridLogo->getSize().x() * 0.8f, mGridLogo->getSize().y() * 0.8f);
  mResultSystemLogo->setKeepRatio(true);
  ResizeGridLogo();

  //Metadata
  mResultDesc->setText(mSearchResults[i]->Metadata().Description());
  mDescContainer->setSize(mGridMeta->getColWidth(2) + mGridMeta->getColWidth(3), mGridMeta->getRowHeight(2)*0.9f);
  mResultDesc->setSize(mDescContainer->getSize().x(), 0); // make desc text wrap at edge of container
  mMDPublisherLabel->setText(_("Publisher") + " : ");
  mMDDeveloperLabel->setText(_("Developer") + " : ");
  mMDPublisher->setText(mSearchResults[i]->Metadata().Publisher());
  mMDDeveloper->setText(mSearchResults[i]->Metadata().Developer());
  float height = (mMDDeveloperLabel->getFont()->getLetterHeight() + 2) / mGridLogoAndMD->getSize().y();
  mGridLogoAndMD->setRowHeightPerc(2, height);
  mGridLogoAndMD->setRowHeightPerc(4, height);
  int width = (int)Math::max(mMDDeveloperLabel->getSize().x(), mMDPublisherLabel->getSize().x());
  mGridLogoAndMD->setColWidthPerc(0, (float)width / mGridLogoAndMD->getSize().x());
  mGridLogoAndMD->getCellAt(0, 2)->resize = true;
  mGridLogoAndMD->getCellAt(0, 4)->resize = true;

  updateHelpPrompts();
}

void GuiSearch::ResizeGridLogo()
{
  //trying to center the logo in a 3x3 grid by adjusting col and row size
  int height = (int)mResultSystemLogo->getSize().y();
  int width = (int)mResultSystemLogo->getSize().x();

  float spacer2 = (mGridLogo->getSize().x() - (float)width) / 2 / mGridLogo->getSize().x();

  mGridLogo->setColWidthPerc(0, spacer2);
  mGridLogo->setColWidthPerc(1, (float)width / mGridLogo->getSize().x());
  mGridLogo->setColWidthPerc(2, spacer2);

  float spacer = (mGridLogo->getSize().y() - (float)height) / 2 / mGridLogo->getSize().y();

  mGridLogo->setRowHeightPerc(0, spacer);
  mGridLogo->setRowHeightPerc(1, (float)height / mGridLogo->getSize().y());
  mGridLogo->setRowHeightPerc(2, spacer);
}

void GuiSearch::launch()
{
  if (mList->size() != 0)
  {
    VideoEngine::Instance().StopVideo(true);
    mResultVideo->setVideo(Path::Empty, 0, 0);

    int index = mList->getCursor();
    Vector3f target(Renderer::Instance().DisplayWidthAsFloat() / 2.0f, Renderer::Instance().DisplayHeightAsFloat() / 2.0f, 0);
    ViewController::Instance().Launch(mSearchResults[index], GameLinkedData(), target, true);
  }
}

void GuiSearch::GoToGame()
{
  if (mList->size() != 0)
  {
    VideoEngine::Instance().StopVideo(true);
    mResultVideo->setVideo(Path::Empty, 0, 0);

    int index = mList->getCursor();
    ViewController::Instance().selectGamelistAndCursor(mSearchResults[index]);
  }
}

void GuiSearch::ArcadeVirtualKeyboardTextChange(GuiArcadeVirtualKeyboard& /*vk*/, const String& text)
{
  mSearch->setValue(text);
  PopulateGrid(text);
}

void GuiSearch::ArcadeVirtualKeyboardValidated(GuiArcadeVirtualKeyboard& /*vk*/, const String& text)
{
  mSearch->setValue(text);
  mSR2.clear();
  PopulateGrid2(text);
}

void GuiSearch::ArcadeVirtualKeyboardCanceled(GuiArcadeVirtualKeyboard& /*vk*/)
{
}

std::string Format(const char* _string, ...)
{
	va_list	args;
	va_list copy;

	va_start(args, _string);

	va_copy(copy, args);
	const int length = vsnprintf(nullptr, 0, _string, copy);
	va_end(copy);

	std::string result;
	result.resize(length);
	va_copy(copy, args);
	vsnprintf((char*)result.c_str(), (size_t)length + 1, _string, copy);
	va_end(copy);

	va_end(args);

	return result;
}

// Split and allocate memory
// After using the result, free it with 'free_word'
char **split_word(const char *keyword) {
	char *internal_keyword, *word = NULL, **p, **lword;
	const char *start;
	int nword = 0;

	// Skip space in keyword
	start = keyword;
	while (*start == ' ')
		start++;

	// Count word
	internal_keyword = strdup(start);
	word = strtok(internal_keyword, " ");
	while (word != NULL) {
		nword++;
		word = strtok(NULL, " ");	//	Subsequent calls to strtok() use NULL as the input, which tells the function to continue splitting from where it left off
	}
	free(internal_keyword);

	// Allocate list of word
	internal_keyword = strdup(start);
	lword = (char **)malloc(sizeof(char *) * (nword + 1));
	p = lword;
	word = strtok(internal_keyword, " ");
	while (word != NULL) {
		*p = word;
		word = strtok(NULL, " ");	//	Subsequent calls to strtok() use NULL as the input, which tells the function to continue splitting from where it left off
		p++;
	}
	*p = NULL;
	return lword;
}

// Free memory used by internal_keyword and list of word
void free_word(char **lword) {
	free(*lword);	// free internal_keyword (1st entry)
	free(lword);	// free list of word
}

// Find keyword(char **) in string line
int find_keyword2(char *line, char **lword) {
	char **p = lword;
	char *word, *in_line;
	int found = 1;

	in_line = strdup(line);
	SDL_strlwr(in_line);	// make 'input line' lower
	while (*p) {
		word = *p;
		if (*word == '-') {
			word++;
			if (strstr(in_line, word)) {
				found = found & 0;
				break;
			}else{
				found = found & 1;
			}
		}else{
			if (strstr(in_line, word)) {
				found = found & 1;
			}else{
				found = found & 0;
				break;
			}
		}
		p++;
	}
	free(in_line);
	return found;
}

char *my_strtok(char *str, char delimiter) {
    static char *token; // Static variable to keep track of the current token
    if (str != NULL) {
        token = str; // Initialize or reset the token if str is not NULL
    } else if (token == NULL || *token == '\0') {
        return NULL; // No more tokens
    }

    // Find the next occurrence of the delimiter in the current token
    char *delimiterPtr = strchr(token, delimiter);

    if (delimiterPtr != NULL) {
        *delimiterPtr = '\0'; // Replace delimiter with '\0' to terminate the current substring
        char *result = token;
        token = delimiterPtr + 1; // Move to the next character after the delimiter
        return result;
    } else {
        // If no more delimiters are found in the current token
        char *result = token;
        token = NULL; // Signal the end of tokens
        return result;
    }
}

int GuiSearch::SearchCSV(const char *csv_fname, char **lword, unsigned int start_no) { 
	FILE *f;
	char line[MAX_LINE];
	char *category = NULL, *base_url = NULL, *p;
	char *a_name, *a_title, *a_desc;

	f = fopen(csv_fname, "r");
	if (!f) {
		return start_no;
	}
	
	fgets(line, MAX_LINE, f);	// Line 1: #category=snes\r\n
	if ( (p = strchr(line, '=') )) {
		category = strdup(p+1);
		p = strrchr(category, '\r'); if (p) *p = '\0';	// replace \r to \0
		p = strrchr(category, '\n'); if (p) *p = '\0';	// replace \n to \0
	}
	
	fgets(line, MAX_LINE, f);		// Line 2: #url=https://archive.org/download/cylums or #url=https://archive.org/download/cylums_collection.zip/ 
	if ((p = strchr(line, '='))) {
		base_url = strdup(p + 1);
		p = strrchr(base_url, '\r'); // windows end of line
		if (p) {
			*p = '\0';	// replace \r to \0
			if (*(p-1) == '/') *(p-1) = '\0';	// remove trailing /
		}else{
			p = strrchr(base_url, '\n');
			*p = '\0';	// replace \n to \0
			if (*(p-1) == '/') *(p-1) = '\0';	// remove trailing /
		}
	}

	if (!base_url) {
		fclose(f);
		if (category) free(category);
		return start_no;
	}
	while (fgets(line, MAX_LINE, f)) { // Process next line: the real csv data
		a_name = my_strtok(line, '|');
		a_title = my_strtok(NULL, '|');
    a_desc = my_strtok(NULL, '|');

    // add to list if keyword is found in title and rom file doesn't exists 
		if (find_keyword2(a_title, lword) && !Path(Format("/recalbox/share/roms/%s/%s.zip", category, a_name)).Exists()) {
			start_no++;
			
			// remove trailing end-of-line in a_desc
			p = strrchr(a_desc, '\r'); // windows end of line
			if (p) {
				*p = '\0';	// replace \r to \0
			}else{
				p = strrchr(a_desc, '\n');
				if (p) *p = '\0';	// replace \n to \0
			}
      
      mSR2.emplace_back(category, a_title, Format("%s/%s", base_url, a_name), a_desc);

      SystemData* system = mSystemManager.SystemByName(category);
      String prefix = system?system->Descriptor().IconPrefix():"";

			ComponentListRow row;
      std::shared_ptr<Component> ed;
      ed = std::make_shared<TextComponent>(mWindow, prefix + a_title, mMenuTheme->menuText.font,
                                            mMenuTheme->menuText.color,
                                            TextAlignment::Left);
      row.addElement(ed, true);
      // row.makeAcceptInputHandler([this] { download(); });
      mList->addRow(row, false, true);
		}
	}
	fclose(f);
	if (category) free(category);
	if (base_url) free(base_url);
	return start_no;
}

// Novan: edit here
void GuiSearch::PopulateGrid2(const String& search)
{
  if (search.length()<3) return;

  unsigned int n_found = 0; // total result found
	struct dirent *entry;
	DIR *dir;
	char *in_keyword = NULL;
	char **lword;
	char fullpath[MAX_LINE];

  in_keyword = strdup(search.c_str());
	SDL_strlwr(in_keyword);

  if ((dir = opendir("/recalbox/share/system/.emulationstation")) == NULL) {
    mWindow.displayMessage("Error opendir for path /recalbox/share/system/.emulationstation");
		free(in_keyword);
	}else{
		lword = split_word(in_keyword);
		entry = readdir(dir);
		do {
			if (strstr(entry->d_name, ".csv")) {
				sprintf(fullpath, "/recalbox/share/system/.emulationstation/%s", entry->d_name);
				n_found = SearchCSV(fullpath, lword, n_found);
			}
		} while ((entry = readdir(dir)) != NULL);
		free_word(lword);
		free(in_keyword);
		closedir(dir);
    { LOG(LogDebug) << "[GuiSearch] Found " << n_found << " roms in database"; }
	}

  if (n_found) mText->setValue("");
}
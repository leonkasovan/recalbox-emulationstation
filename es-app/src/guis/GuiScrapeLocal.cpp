// GUI for Local Scraping (in /recalbox/share/screenshots etc)
// Created by Dhani Novan on 31/03/2024.
//

#include "GuiScrapeLocal.h"
#include <dirent.h>

#define SCREENSHOT_PATH "/recalbox/share/screenshots"

GuiScrapeLocal::GuiScrapeLocal(WindowManager& window, FileData& game)
  : Gui(window)
  , mGame(game)
  , mBackground(window, Path(":/frame.png"))
  , mGrid(window, Vector2i(2, 2))
  , mList(nullptr)
{
  addChild(&mBackground);
  addChild(&mGrid);

  mMenuTheme = MenuThemeData::getInstance()->getCurrentTheme();

  mBackground.setImagePath(mMenuTheme->menuBackground.path);
  mBackground.setCenterColor(mMenuTheme->menuBackground.color);
  mBackground.setEdgeColor(mMenuTheme->menuBackground.color);

  //init Title
  mTitle = std::make_shared<TextComponent>(mWindow, mGame.RomPath().Filename(), mMenuTheme->menuText.font,
                                           mMenuTheme->menuText.color, TextAlignment::Center);
  mGrid.setEntry(mTitle, Vector2i(0, 0), false, true, Vector2i(2, 1));

  mList = std::make_shared<ComponentList>(mWindow);
  mList->setCursorChangedCallback([this](CursorState) { updateInformations(); });
  mGrid.setEntry(mList, Vector2i(0, 1), true, true, Vector2i(1, 1));

  mThumbnail = std::make_shared<ImageComponent>(mWindow);
  mThumbnail->setImage(Path::Empty);
  mGrid.setEntry(mThumbnail, Vector2i(1, 1), false, ComponentGrid::HAlignment::Right, ComponentGrid::VAlignment::Top);

  mGrid.setRowHeightPerc(0, 0.1f);
  
  setSize(Renderer::Instance().DisplayWidthAsFloat() * 0.95f, Renderer::Instance().DisplayHeightAsFloat() * 0.8f);
  setPosition((Renderer::Instance().DisplayWidthAsFloat() - mSize.x()) / 2,
              (Renderer::Instance().DisplayHeightAsFloat() - mSize.y()) / 2);

  PopulateGrid();
}

GuiScrapeLocal::~GuiScrapeLocal()
{
  if (mList)
  {
    mList->clear();
  }
  filenames.clear();
}

void GuiScrapeLocal::onSizeChanged()
{
  mBackground.fitTo(mSize, Vector3f::Zero(), Vector2f(-32, -32));
  mGrid.setSize(mSize);
}

bool GuiScrapeLocal::getHelpPrompts(Help& help)
{
  mGrid.getHelpPrompts(help);
  help.Set(Help::Cancel(), _("CANCEL")).Set(Help::Valid(), _("SCRAPE"));
  return true;
}

bool GuiScrapeLocal::ProcessInput(const class InputCompactEvent & event)
{
  if (event.CancelPressed())
  {
    Close();
    return true;
  }else if (event.ValidPressed()){
    char fullpath[1024];
    sprintf(fullpath, SCREENSHOT_PATH"/%s", filenames[mList->getCursorIndex()].c_str());
    mGame.Metadata().SetImagePath(Path(fullpath));
    Close();
    return true;
  }
  return Component::ProcessInput(event);
}

void GuiScrapeLocal::PopulateGrid()
{

  if (mList) mList->clear();

  ComponentListRow row;
  std::shared_ptr<TextComponent> ed;
  struct dirent *entry;
  DIR *dir1;
  dir1 = opendir(SCREENSHOT_PATH);
  if (dir1) {
    entry = readdir(dir1);
    do {
        if (strstr(entry->d_name, ".png")) { 
            { LOG(LogDebug) << "[GuiScrapeLocal.cpp] entry->d_name=" << entry->d_name; }
            ed = std::make_shared<TextComponent>(mWindow, entry->d_name, mMenuTheme->menuText.font, mMenuTheme->menuText.color,TextAlignment::Left);
            row.elements.clear();
            row.addElement(ed, true);
            mList->addRow(row, false, true);
            filenames.push_back(entry->d_name);
        }
    } while ((entry = readdir(dir1)) != NULL);
    closedir(dir1);
  }

  if (mList->size() > 0) mList->setCursorIndex(0);
  updateInformations();
}

//called when changing cursor in mList to populate MD
void GuiScrapeLocal::updateInformations()
{
  mThumbnail->setImage(Path::Empty);
  if (mList->isEmpty()) return;
  char fullpath[1024];
  sprintf(fullpath, SCREENSHOT_PATH"/%s", filenames[mList->getCursorIndex()].c_str());
  mThumbnail->setImage(Path(fullpath));
  mThumbnail->setResize(mGrid.getColWidth(1) * 0.9f, mGrid.getRowHeight(1) * 0.9f);
  mThumbnail->setKeepRatio(true);
//   mThumbnail->setOrigin(0.5f, 0.5f);
//   mThumbnail->setPosition(5,5);

//   updateHelpPrompts();
}
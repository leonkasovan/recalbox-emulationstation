//
// Created by bkg2k on 11/10/23.
//

#include "WizardBase.h"
#include "utils/locale/LocaleHelper.h"
#include "themes/MenuThemeData.h"
#include <help/Help.h>
#include <components/MenuComponent.h> // for makeButtonGrid

WizardBase::WizardBase(WindowManager& window, const String& title, int pageCount, bool autoNext)
  : Gui(window)
  , mBackground(window)
  , mGrid(window, { 12, 13 })
  , mPageCount(pageCount)
  , mCurrentPage(-1)
  , mAutoNext(autoNext)
{
  // Window's background
  addChild(&mBackground);
  // Add grid as the main component
  addChild(&mGrid);

  // Get theme
  auto menuTheme = MenuThemeData::getInstance()->getCurrentTheme();
  mBackground.setImagePath(menuTheme->menuBackground.path);
  mBackground.setCenterColor(menuTheme->menuBackground.color);
  mBackground.setEdgeColor(menuTheme->menuBackground.color);

  // 12 x 13
  // +-+---------------------------------------------------------------------+-+
  // | |                             TITLE                                   | |  0 = Text size%
  // +-+------+------+------+------+------+------+------+------+------+------+-+
  // | |      |      |      |      |      |      |      |      |      |      | |  1 \.
  // +-+------+------+------+------+------+------+------+------+------+------+-+     |
  // | |      |      |      |      |      |      |      |      |      |      | |  2  |
  // +-+------+------+------+------+------+------+------+------+------+------+-+     |
  // | |      |      |      |      |      |      |      |      |      |      | |  3  |
  // +-+------+------+------+------+------+------+------+------+------+------+-+     |
  // | |      |      |      |      |      |      |      |      |      |      | |  4  |
  // +-+------+------+------+------+------+------+------+------+------+------+-+     |
  // | |      |      |      |      |      |      |      |      |      |      | |  5  |
  // +-+------+------+------+------+------+------+------+------+------+------+-+     |- Auto size
  // | |      |      |      |      |      |      |      |      |      |      | |  6  |
  // +-+------+------+------+------+------+------+------+------+------+------+-+     |
  // | |      |      |      |      |      |      |      |      |      |      | |  7  |
  // +-+------+------+------+------+------+------+------+------+------+------+-+     |
  // | |      |      |      |      |      |      |      |      |      |      | |  8  |
  // +-+------+------+------+------+------+------+------+------+------+------+-+     |
  // | |      |      |      |      |      |      |      |      |      |      | |  9  |
  // +-+------+------+------+------+------+------+------+------+------+------+-+     |
  // | |      |      |      |      |      |      |      |      |      |      | | 10 /
  // +-+------+------+------+------+------+------+------+------+------+------+-+
  // | |  Page/Total                                          Buttons        | | 11
  // +-+---------------------------------------------------------------------+-+
  // +-+---------------------------------------------------------------------+-+ 12 = 2% margin
  //  0    1      2      3      4      5      6      7      8      9     10   11
  //  2%   \______________________________________________________________/   2%
  //                                Auto size

  // Title
  mTitle = std::make_shared<TextComponent>(window, _S(title), menuTheme->menuTitle.font, menuTheme->menuTitle.color, TextAlignment::Center);
  mGrid.setEntry(mTitle, Vector2i(1, 0), false, true, Vector2i(10,1) );

  // Set Window position/size
  setSize(Renderer::Instance().DisplayWidthAsFloat() * 0.9f, Renderer::Instance().DisplayHeightAsFloat() * 0.9f);
  setPosition((Renderer::Instance().DisplayWidthAsFloat() - mSize.x()) / 2, (Renderer::Instance().DisplayHeightAsFloat() - mSize.y()) / 2);

  mBackground.fitTo(mSize, Vector3f::Zero(), Vector2f(-32, -32));
  mGrid.setSize(mSize);

  // Sizes
  mGrid.setColWidthPerc(0, 0.04f, false);
  mGrid.setColWidthPerc(11, 0.04f, true);
  mGrid.setRowHeightPerc(0, mTitle->getFont()->getLetterHeight() * 2.6f / mSize.y(), false);
  mGrid.setRowHeightPerc(11, menuTheme->menuText.font->getLetterHeight() * 2.6f / mSize.y(), false);
  mGrid.setRowHeightPerc(12, 0.04f, true);

  //mGrid.SetColumnHighlight(true, 1, 10);
  //mGrid.SetRowHighlight(true, 1, 10);
}

bool WizardBase::ProcessInput(const InputCompactEvent& event)
{
  switch(OnKeyReceived(mCurrentPage, event))
  {
    case Move::Backward: { SetPage(mCurrentPage - 1); return true; }
    case Move::Forward: { SetPage(mCurrentPage + 1); return true; }
    case Move::Close: { Close(); return true; }
    case Move::None:
    default: break;
  }
  return Component::ProcessInput(event);
}

void WizardBase::Update(int deltaTime)
{
  Component::Update(deltaTime);

  if (mCurrentPage < 0)
    SetPage(0);
}

void WizardBase::SetPage(int page)
{
  mCurrentPage = Math::clampi(page, 0, mPageCount - 1);

  // Wipe everything
  mGrid.ClearEntries();

  // Components
  std::shared_ptr<Component> component;
  Rectangle where;
  for(int index = 0; OnComponentRequired(mCurrentPage, index, where, component); ++index)
    mGrid.setEntry(component, { Math::clampi((int)where.Left(), 0, 9) + 1, Math::clampi((int)where.Top(), 0, 9) + 1 }, false, true, { Math::clampi((int)where.Width(), 1, 10), Math::clampi((int)where.Height(), 1, 10) });

  // Buttons
  String buttonName;
  std::vector< std::shared_ptr<ButtonComponent> > buttons;
  for(int index = 0; OnButtonRequired(mCurrentPage, index, buttonName); ++index)
    buttons.push_back(std::make_shared<ButtonComponent>(mWindow, buttonName, String::Empty, std::bind(&WizardBase::DoButtonClick, this, index)));
  if (mAutoNext)
    buttons.push_back(std::make_shared<ButtonComponent>(mWindow, (_F(" \uf180 {0} ") / (mCurrentPage < mPageCount - 1 ? _("NEXT") : _("CLOSE"))).ToString(), String::Empty, std::bind(&WizardBase::DoButtonClick, this, -1)));
  std::shared_ptr<ComponentGrid> buttonGrid = makeButtonGrid(mWindow, buttons);
  mGrid.setEntry(buttonGrid, Vector2i(6, 11), true, ComponentGrid::HAlignment::Right, ComponentGrid::VAlignment::Center, false, Vector2i(5, 1));
  if (mAutoNext)
    buttonGrid->setCursorTo(buttons.back());
  mGrid.setCursor({ 10, 11 });

  mGrid.onSizeChanged();
  updateHelpPrompts();
}

void WizardBase::DoButtonClick(int index)
{
  if (index < 0)
  {
    if (mCurrentPage < mPageCount - 1) SetPage(mCurrentPage + 1);
    else Close();
  }
  else switch(OnButtonClick(mCurrentPage, index))
  {
    case Move::Backward: { SetPage(mCurrentPage - 1); break; }
    case Move::Forward: { SetPage(mCurrentPage + 1); break; }
    case Move::Close: { Close(); break; }
    case Move::None:
    default: break;
  }
}

bool WizardBase::getHelpPrompts(Help& help)
{
  help.Clear();
  if (mCurrentPage > 0) help.Set(Help::Cancel(), _("BACK"));

  OnHelpRequired(mCurrentPage, help);

  return true;
}

void WizardBase::UpdatePage()
{
  SetPage(mCurrentPage);
}

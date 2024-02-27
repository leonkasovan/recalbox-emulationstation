#include "components/ScraperSearchComponent.h"

#include "components/TextComponent.h"
#include "components/ScrollableContainer.h"
#include "components/ImageComponent.h"
#include "components/RatingComponent.h"
#include "components/DateTimeComponent.h"
#include "components/AnimatedImageComponent.h"
#include "utils/Log.h"
#include "guis/GuiTextEditPopup.h"
#include "guis/GuiTextEditPopupKeyboard.h"
#include "utils/locale/LocaleHelper.h"
#include "themes/MenuThemeData.h"

ScraperSearchComponent::ScraperSearchComponent(WindowManager& window, bool lowResolution)
  : Component(window)
  , mGrid(window, Vector2i(7, 6))
  , mLowResolution(lowResolution)
  , mBusyAnim(window)
{
	addChild(&mGrid);
	auto menuTheme = MenuThemeData::getInstance()->getCurrentTheme();

	// GRID: 7 x 6
	// +-+-------------------------------------------------------------------------------------------------------------+-+
	// | | GAME TITLE                                                                                                  | | 0 = 10% or Text height
  // | +-------------------------------+--------------------+-------------------+-------------------+----------------+ |
  // | |  ###########################  |        DEVELOPER : | SEGA              |          RATING : | @@@OO          | | 1 = 10% or Text height
  // | |  #                         #  +--------------------+-------------------+-------------------+----------------+ |
  // | |  #                         #  |        PUBLISHER : | NINTENDO          |    RELEASE DATE : | 1891/12/03     | | 2 = 10% or Text height
  // | |  #                         #  +--------------------+-------------------+-------------------+----------------+ |
  // | |  #                         #  |            GENRE : | SHOOT'EM-UP       |         PLAYERS : | 1-2            | | 3 = 10% or Text height
  // | |  #                         #  +--------------------+-------------------+-------------------+----------------+ |
  // | |  #                         #  |                                                                             | |
  // | |  #                         #  | Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod     | |
  // | |  #                         #  | tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim        | | 4 = 30%
  // | |  ###########################  | veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea     | |
  // | |                               | commodo consequat. Duis aute irure dolor in reprehenderit in voluptate      | |
  // | +-------------------------------+ velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat  | |
  // | |                               | cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id   | |
  // | |                               | est laborum.                                                                | |
  // | |   @O Animated busy            |                                                                             | |
  // | |   OO                          |                                                                             | | 5 = 30%
  // | |                               |                                                                             | |
  // | |                               |                                                                             | |
  // +-+-------------------------------+-----------------------------------------------------------------------------+-+
  //  0              1                            2                  3                   4                   5        6
  //  5%            30%                          15%                15%                 15%                 15%       5%

  auto font = menuTheme->menuTextSmall.font;
  const unsigned int mdColor = menuTheme->menuText.color;
  const unsigned int mdLblColor = menuTheme->menuText.color;

  // Game Name
  mResultName = std::make_shared<TextComponent>(mWindow, "RESULT NAME", menuTheme->menuText.font, menuTheme->menuText.color);
  mGrid.setEntry(mResultName, Vector2i(1, 0), false, true, Vector2i(5, 1));

  // Image thumbnail
  mResultThumbnail = std::make_shared<ImageComponent>(mWindow);
  if(!lowResolution)
    mGrid.setEntry(mResultThumbnail, Vector2i(1, 1), false, false, Vector2i(1, 4));

  // selected result desc + container
  mDescContainer = std::make_shared<ScrollableContainer>(mWindow);
  mResultDesc = std::make_shared<TextComponent>(mWindow, "RESULT DESC", font, menuTheme->menuText.color);
  mDescContainer->addChild(mResultDesc.get());
  mDescContainer->setAutoScroll(true);
  // show description on the right
  if(!lowResolution)
    mGrid.setEntry(mDescContainer, Vector2i(2, 4), false, false, Vector2i(4, 2));
  mResultDesc->setSize(mDescContainer->getSize().x(), 0); // make desc text wrap at edge of container

  // Labels
  mLabelDeveloper = std::make_shared<TextComponent>(mWindow, _("Developer").UpperCaseUTF8().Append(':'), font, mdLblColor, TextAlignment::Right);
  mLabelPublisher = std::make_shared<TextComponent>(mWindow, _("Publisher").UpperCaseUTF8().Append(':'), font, mdLblColor, TextAlignment::Right);
  mLabelGenre = std::make_shared<TextComponent>(mWindow, _("Genre").UpperCaseUTF8().Append(':'), font, mdLblColor, TextAlignment::Right);
  mLabelRating = std::make_shared<TextComponent>(mWindow, _("Rating").UpperCaseUTF8().Append(':'), font, mdLblColor, TextAlignment::Right);
  mLabelReleaseDate = std::make_shared<TextComponent>(mWindow, _("Released").UpperCaseUTF8().Append(':'), font, mdLblColor, TextAlignment::Right);
  mLabelPlayers = std::make_shared<TextComponent>(mWindow, _("Players").UpperCaseUTF8().Append(':'), font, mdLblColor, TextAlignment::Right);
  mGrid.setEntry(mLabelDeveloper, Vector2i(2, 1), false, false, Vector2i(1, 1));
  mGrid.setEntry(mLabelPublisher, Vector2i(2, 2), false, false, Vector2i(1, 1));
  mGrid.setEntry(mLabelGenre, Vector2i(2, 3), false, false, Vector2i(1, 1));
  mGrid.setEntry(mLabelRating, Vector2i(4, 1), false, false, Vector2i(1, 1));
  mGrid.setEntry(mLabelReleaseDate, Vector2i(4, 2), false, false, Vector2i(1, 1));
  mGrid.setEntry(mLabelPlayers, Vector2i(4, 3), false, false, Vector2i(1, 1));

  // Value
  mValueDeveloper = std::make_shared<TextComponent>(mWindow, "", font, mdColor);
  mGrid.setEntry(mValueDeveloper, Vector2i(3, 1), false, false, Vector2i(1, 1));
  mValuePublisher = std::make_shared<TextComponent>(mWindow, "", font, mdColor);
  mGrid.setEntry(mValuePublisher, Vector2i(3, 2), false, false, Vector2i(1, 1));
  mValueGenre = std::make_shared<TextComponent>(mWindow, "", font, mdColor);
  mGrid.setEntry(mValueGenre, Vector2i(3, 3), false, false, Vector2i(1, 1));
  mValueRating = std::make_shared<RatingComponent>(mWindow, menuTheme->menuText.color, 0.f);
  mGrid.setEntry(mValueRating, Vector2i(5, 1), false, false, Vector2i(1, 1));
  mValueReleaseDate = std::make_shared<DateTimeComponent>(mWindow);
  mValueReleaseDate->setColor(mdColor);
  mValueReleaseDate->setHorizontalAlignment(TextAlignment::Left);
  mGrid.setEntry(mValueReleaseDate, Vector2i(5, 2), false, false, Vector2i(1, 1));
  mValuePlayers = std::make_shared<TextComponent>(mWindow, "", font, mdColor);
  mGrid.setEntry(mValuePlayers, Vector2i(5, 3), false, false, Vector2i(1, 1));

  mRunning = true;
}

void ScraperSearchComponent::onSizeChanged()
{
	mGrid.setSize(mSize);
	
	if(mSize.x() == 0 || mSize.y() == 0) return;

  // column widths
  if(mLowResolution)
  {
    mGrid.setColWidthPerc(0, 0.05f);
    mGrid.setColWidthPerc(1, 0.0f);
    mGrid.setColWidthPerc(2, 0.25f);
    mGrid.setColWidthPerc(3, 0.25f);
    mGrid.setColWidthPerc(4, 0.20f);
    mGrid.setColWidthPerc(5, 0.20f);
    mGrid.setColWidthPerc(6, 0.05f);
  } else {
    mGrid.setColWidthPerc(0, 0.05f);
    mGrid.setColWidthPerc(1, 0.30f);
    mGrid.setColWidthPerc(2, 0.15f);
    mGrid.setColWidthPerc(3, 0.15f);
    mGrid.setColWidthPerc(4, 0.15f);
    mGrid.setColWidthPerc(5, 0.15f);
    mGrid.setColWidthPerc(6, 0.05f);
  }

	// row heights
  auto menuTheme = MenuThemeData::getInstance()->getCurrentTheme();
  auto font = menuTheme->menuTextSmall.font;
	float firstRowPercent = (mResultName->getFont()->getHeight() * 1.6f) / mGrid.getSize().y();
  float textRowPercent  = (font->getHeight() * 1.6f) / mGrid.getSize().y();
  float otherRowPercent = (1.0f - (firstRowPercent + textRowPercent * 3.0f)) / 2.0f;
	mGrid.setRowHeightPerc(0, firstRowPercent); // result name
  mGrid.setRowHeightPerc(1, textRowPercent);
  mGrid.setRowHeightPerc(2, textRowPercent);
  mGrid.setRowHeightPerc(3, textRowPercent);
  mGrid.setRowHeightPerc(4, mLowResolution ? 0.001f : otherRowPercent);
  mGrid.setRowHeightPerc(5, otherRowPercent);

  // Resize title & description
	const float boxartCellScale = 0.9f;
	mResultThumbnail->setResize(mGrid.getColWidth(1) * boxartCellScale, mGrid.getRowHeight(1, 4) * boxartCellScale);
  mResultThumbnail->setKeepRatio(true);
	mDescContainer->setSize(mGrid.getColWidth(2, 5) * boxartCellScale, mGrid.getRowHeight(4, 5) * boxartCellScale);
	mResultDesc->setSize(mDescContainer->getSize().x(), 0); // make desc text wrap at edge of container

  // Resize Labels
  mLabelDeveloper->setSize(mGrid.getColWidth(3) * boxartCellScale, mLabelDeveloper->getFont()->getHeight());
  mLabelPublisher->setSize(mGrid.getColWidth(3) * boxartCellScale, mLabelPublisher->getFont()->getHeight());
  mLabelGenre->setSize(mGrid.getColWidth(3) * boxartCellScale, mLabelGenre->getFont()->getHeight());
  mLabelRating->setSize(mGrid.getColWidth(5) * boxartCellScale, mLabelRating->getFont()->getHeight());
  mLabelReleaseDate->setSize(mGrid.getColWidth(5) * boxartCellScale, mLabelReleaseDate->getFont()->getHeight());
  mLabelPlayers->setSize(mGrid.getColWidth(5) * boxartCellScale, mLabelPlayers->getFont()->getHeight());

  // Resize Values
	mValueDeveloper->setSize(mGrid.getColWidth(3) * boxartCellScale, mValueDeveloper->getFont()->getHeight());
  mValuePublisher->setSize(mGrid.getColWidth(3) * boxartCellScale, mValuePublisher->getFont()->getHeight());
  mValueGenre->setSize(mGrid.getColWidth(3) * boxartCellScale, mValueGenre->getFont()->getHeight());
  mValueRating->setSize(mGrid.getColWidth(5) * boxartCellScale, mValueDeveloper->getFont()->getHeight() * 0.65f);
  mValueReleaseDate->setSize(mGrid.getColWidth(5) * boxartCellScale, mValueDeveloper->getFont()->getHeight());
  mValuePlayers->setSize(mGrid.getColWidth(5) * boxartCellScale, mValuePublisher->getFont()->getHeight());

  mGrid.onSizeChanged();
  if(!mLowResolution)
  {
    mBusyAnim.setPosition(mGrid.getColWidth(0), mGrid.getRowHeight(0, 4));
    mBusyAnim.setSize(mGrid.getColWidth(1), mGrid.getRowHeight(5));
  }
}

void ScraperSearchComponent::UpdateInfoPane(const FileData* game)
{
	if (game != nullptr)
	{
		mResultName->setText(game->Name().ToUpperCaseUTF8());
		mResultDesc->setText(game->Metadata().Description().ToUpperCaseUTF8());
		mDescContainer->reset();

		// Image
		Path image = game->Metadata().Image().IsEmpty() ? game->Metadata().Thumbnail() : game->Metadata().Image();
    mResultThumbnail->setImage(image);

    // metadata
		mValueRating->setValue(game->Metadata().Rating());
		mValueReleaseDate->setValue(game->Metadata().ReleaseDateAsString().ToUpperCase());
		mValueDeveloper->setText(game->Metadata().Developer().ToUpperCaseUTF8());
		mValuePublisher->setText(game->Metadata().Publisher().ToUpperCaseUTF8());
		mValueGenre->setText(game->Metadata().Genre().ToUpperCaseUTF8());
		mValuePlayers->setText(game->Metadata().PlayersAsString().ToUpperCase());
		mGrid.onSizeChanged();
	}
	else
	{
		mResultName->setText("");
		mResultDesc->setText("");

    // Image
    mResultThumbnail->setImage(Path());

    // metadata
		mValueRating->setValue("");
		mValueReleaseDate->setValue("");
		mValueDeveloper->setText("");
		mValuePublisher->setText("");
		mValueGenre->setText("");
		mValuePlayers->setText("");
	}
}

bool ScraperSearchComponent::ProcessInput(const InputCompactEvent& event)
{
	return Component::ProcessInput(event);
}

void ScraperSearchComponent::Render(const Transform4x4f& parentTrans)
{
	Transform4x4f trans = parentTrans * getTransform();

	renderChildren(trans);

	if (mRunning)
	{
		Renderer::SetMatrix(trans);
		Renderer::DrawRectangle(0.f, 0.f, mSize.x(), mSize.y(), 0x00000011);

    if(!mLowResolution) mBusyAnim.Render(trans);
	}
}

void ScraperSearchComponent::Update(int deltaTime)
{
  Component::Update(deltaTime);

	if (mRunning && !mLowResolution)
    mBusyAnim.Update(deltaTime);
}

bool ScraperSearchComponent::getHelpPrompts(Help& help)
{
	mGrid.getHelpPrompts(help);
	return true;
}

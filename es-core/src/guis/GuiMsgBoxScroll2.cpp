#include <guis/GuiMsgBoxScroll2.h>
#include <components/TextComponent.h>
#include <components/ButtonComponent.h>
#include <components/MenuComponent.h> // for makeButtonGrid

#define ES_DIR1 "/recalbox/share_init/system/.emulationstation"
#define BUFFER_SIZE 1024

int ReadHelpContent(const char *filename, int content_number, String &content){
    FILE *fi = fopen(filename, "r");
    if (fi == NULL) {
        return -1;
    }

    if (content_number <= 0) {
        fclose(fi);
        return -2;
    }

    char buffer[BUFFER_SIZE];
    int current_content = 0;
    size_t bytes_read;
    char *pos;
    content = "";

    while ((bytes_read = fread(buffer, 1, BUFFER_SIZE, fi)) > 0) {
        pos = buffer;
        while ((pos = strchr(pos, '|')) != NULL) {
            current_content++;
            if (current_content == content_number) {
                pos++; // Move past the '|'
                if (*pos == '\r') pos++;
                if (*pos == '\n') pos++;
                while (((size_t)(pos - buffer) < bytes_read) && (*pos != '|') ){
                    content.Append(*pos);
                    pos++;
                }
                if ((size_t)(pos - buffer) == bytes_read){
                    int done = 0;
                    while (!done && (bytes_read = fread(buffer, 1, BUFFER_SIZE, fi)) > 0) {
                        pos = buffer;
                        while (((size_t)(pos - buffer) < bytes_read) && (*pos != '|') ){
                            content.Append(*pos);
                            pos++;
                        }
                        if (*pos == '|') done = 1;
                    }
                }
                fclose(fi);
                return 0;
            }
            pos++;
        }
    }
    fclose(fi);
    return -3;
}

GuiMsgBoxScroll2::GuiMsgBoxScroll2(WindowManager& window,
                                 const String& title, const String& text,
                                 const String& name1, const std::function<void()>& func1,
                                 const String& name2, const std::function<void()>& func2,
                                 const String& name3, const std::function<void()>& func3,
                                 TextAlignment align, float height)
  : Gui(window)
  , mBackground(window, Path(":/frame.png"))
  , mGrid(window, Vector2i(1, 3))
  , mSpace(Renderer::Instance().DisplayHeightAsInt() / 40)
{
  if (mSpace > 20) mSpace = 20;
  if (mSpace < 6) mSpace = 6;
	(void)height;

	float width = Renderer::Instance().DisplayWidthAsFloat() * 0.8f; // max width
	auto menuTheme = MenuThemeData::getInstance()->getCurrentTheme();

	mBackground.setImagePath(menuTheme->menuBackground.path);
	mBackground.setCenterColor(menuTheme->menuBackground.color);
	mBackground.setEdgeColor(menuTheme->menuBackground.color);

	mTitle = std::make_shared<TextComponent>(mWindow, title, menuTheme->menuTitle.font, menuTheme->menuTitle.color, TextAlignment::Center);
	mGrid.setEntry(mTitle, Vector2i(0, 0), false, true);

	mMsg = std::make_shared<TextComponent>(mWindow, text, menuTheme->menuTextSmall.font, menuTheme->menuTextSmall.color, align);

	mMsgContainer = std::make_shared<VerticalScrollableContainer>(mWindow);
	mMsgContainer->addChild(mMsg.get());

	mGrid.setEntry(mMsgContainer, Vector2i(0, 1), false, false);

	// create the buttons
	mButtons.push_back(std::make_shared<ButtonComponent>(mWindow, name1, name1, std::bind(&GuiMsgBoxScroll2::deleteMeAndCall, this, func1)));
	if(!name2.empty())
		mButtons.push_back(std::make_shared<ButtonComponent>(mWindow, name2, name3, std::bind(&GuiMsgBoxScroll2::deleteMeAndCall, this, func2)));
	if(!name3.empty())
		mButtons.push_back(std::make_shared<ButtonComponent>(mWindow, name3, name3, std::bind(&GuiMsgBoxScroll2::deleteMeAndCall, this, func3)));

	// set accelerator automatically (button to press when "b" is pressed)
	if(mButtons.size() == 1)
	{
		mAcceleratorFunc = mButtons.front()->getPressedFunc();
	}else
  {
		for (auto& mButton : mButtons)
			if (String buttonUC = mButton->getText().ToUpperCase(); buttonUC == "OK" || buttonUC == "NO") // #TODO Do not rely on button texts
			{
				mAcceleratorFunc = mButton->getPressedFunc();
				break;
			}
	}

	// put the buttons into a ComponentGrid
	mButtonGrid = makeButtonGrid(mWindow, mButtons);
	mGrid.setEntry(mButtonGrid, Vector2i(0, 2), true, false, Vector2i(1, 1));
	mMsg->setSize(width, 0);
	const float msgHeight = Math::min(Renderer::Instance().DisplayHeightAsFloat() * 0.5f, mMsg->getSize().y());
    { LOG(LogError) << "msgHeight=" << msgHeight << " mMsg->getSize().y()=" << mMsg->getSize().y(); }
	mMsgContainer->setSize(width, msgHeight);
	setSize(width + mSpace*2, mButtonGrid->getSize().y() + msgHeight + mTitle->getSize().y());

	// center for good measure
	setPosition((Renderer::Instance().DisplayWidthAsFloat() - mSize.x()) / 2, (Renderer::Instance().DisplayHeightAsFloat() - mSize.y()) / 2);

	addChild(&mBackground);
	addChild(&mGrid);

    content_number = 1;
    String content = ""; 
    if (ReadHelpContent(ES_DIR1 "/recalbox_help.txt",content_number,content) == 0)
        mMsg->setText(content);
}

bool GuiMsgBoxScroll2::ProcessInput(const InputCompactEvent& event)
{
    String content = "";
	// special case for when GuiMsgBox comes up to report errors before anything has been configured
	/* when it's not configured, allow to remove the message box too to allow the configdevice window a chance */
	if(mAcceleratorFunc && event.AskForConfiguration())
	{
		mAcceleratorFunc();
		return true;
	}

	if (mMsgContainer->ProcessInput(event)) return true;

	if (event.L1Pressed()){
        --content_number;
        if (content_number < 1) content_number = 1;
        if (ReadHelpContent(ES_DIR1 "/recalbox_help.txt",content_number,content) == 0){
            mMsg->setText(content);
            mMsgContainer->reset();
            // onSizeChanged();
        }
	}else if (event.R1Pressed()){
        ++content_number;
        if (ReadHelpContent(ES_DIR1 "/recalbox_help.txt",content_number,content) == 0){
            mMsg->setText(content);
            mMsgContainer->reset();
            // onSizeChanged();
        }else{
            --content_number;
        }
    }else if (event.L2Pressed()){
        mMsg->setText("L2");
    }else if (event.R2Pressed()){
        mMsg->setText("R2");
    }
	return Component::ProcessInput(event);
}

void GuiMsgBoxScroll2::onSizeChanged()
{
	mGrid.setSize(mSize);
	mGrid.setRowHeightPerc(0, mTitle->getFont()->getHeight() / mSize.y());
	mGrid.setRowHeightPerc(2, mButtonGrid->getSize().y() / mSize.y());

	// update messagebox size
	mMsg->setSize(mMsgContainer->getSize().x(), 0); // make desc text wrap at edge of container
	mGrid.onSizeChanged();

	mBackground.fitTo(mSize, Vector3f::Zero(), Vector2f(-32, -32));
}

void GuiMsgBoxScroll2::deleteMeAndCall(const std::function<void()>& func)
{
	Close();
	if(func) func();
}

GuiMsgBoxScroll2* GuiMsgBoxScroll2::SetDefaultButton(int index)
{
  if ((unsigned int)index < (unsigned int)mButtons.size())
    mButtonGrid->setCursorTo(mButtons[index]);

  return this;
}

bool GuiMsgBoxScroll2::getHelpPrompts(Help& help)
{
  mGrid.getHelpPrompts(help);
  help.Set(HelpType::LR, "Prev|Next")
      .Set(HelpType::L2R2, "First|Last");
  return true;
}
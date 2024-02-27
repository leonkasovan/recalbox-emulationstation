//
// Created by Dhani Novan on 27/02/24.
//

#include <RecalboxConf.h>
#include "GuiDownloadFile.h"
#include "components/TextComponent.h"
#include "components/MenuComponent.h"
#include <utils/Files.h>
#include <utils/locale/LocaleHelper.h>
#include <utils/network/Url.h>
#include <MainRunner.h>
#include <sys/stat.h>
#include <sstream>
#include <locale>

#define BUTTON_GRID_VERT_PADDING Renderer::Instance().DisplayHeightAsFloat() * 0.025f
#define BUTTON_GRID_HORIZ_PADDING 10

#define TITLE_HEIGHT (mTitle->getFont()->getLetterHeight() + TITLE_VERT_PADDING)

// Get filename from Url
String get_filename(const String mUrl){
  const char* p = mUrl.c_str(); // Char pointer is faster
  for(int i = (int)mUrl.size(); --i >= 0;){
    if (p[i] == '/'){
        return Url::URLDecode(mUrl.SubString(i + 1));
    }
  }
  return "";
}

std::string addThousandSeparator(const std::string& input) {
    std::string result;
    size_t length = input.length();

    // Determine the position to start inserting dots
    size_t firstSeparatorPos = length % 3;
    if (firstSeparatorPos == 0)
        firstSeparatorPos = 3;

    // Iterate through the string and insert dots
    for (size_t i = 0; i < length; ++i) {
        if (i > 0 && (i % 3 == firstSeparatorPos))
            result.push_back('.');

        result.push_back(input[i]);
    }

    return result;
}

GuiDownloadFile::GuiDownloadFile(WindowManager& window, const String& Url, const String& system)
  : Gui(window)
  , mUrl(Url)
  , mSystem(system)
  , mTotalSize(0)
  , mCurrentSize(0)
  , mSender(*this)
  , mBackground(window, Path(":/frame.png"))
  , mGrid(window, Vector2i(3, 4))
{
  destinationFileName = get_filename(Url::URLDecode(mUrl));
  { LOG(LogDebug) << "[GuiDownloadFile] mUrl:" << mUrl; }
  { LOG(LogDebug) << "[GuiDownloadFile] destinationFileName:" << destinationFileName; }

  if (mSystem.length())
    destination = Path(sDownloadFolder) / mSystem / destinationFileName;
  else
    destination = Path(sDownloadFolder) / destinationFileName;

  mDownloadedSize = "Downloaded: [F0] ";
  mError = "Error downloading... Please retry later!";
  mMessage = "Filename: "+destinationFileName+"\nFilesize: [F1]\nDestination: "+destination.ToString()+"\nTime elapsed: [F2]\nEstimated time: [F3]";

  addChild(&mBackground);
  addChild(&mGrid);

  std::shared_ptr<MenuTheme> menuTheme = MenuThemeData::getInstance()->getCurrentTheme();

  const float height = Renderer::Instance().DisplayHeightAsFloat() * (Renderer::Instance().Is480pOrLower() ? 0.7f : 0.5f);
  const float width = Renderer::Instance().DisplayWidthAsFloat() * (Renderer::Instance().Is480pOrLower() ? 0.8f : 0.6f);

  // Title
  mTitle = std::make_shared<TextComponent>(mWindow, "DOWNLOADING...", menuTheme->menuTitle.font, menuTheme->menuTitle.color, TextAlignment::Center);
  mGrid.setEntry(mTitle, Vector2i(1, 0), false, false, Vector2i(1,1) );

  // Text
  mText = std::make_shared<TextComponent>(mWindow, mMessage, menuTheme->menuTextSmall.font, menuTheme->menuTextSmall.color, TextAlignment::Left);
  mGrid.setEntry(mText, Vector2i(1, 1), false, false, Vector2i(1,1) );

  // Progress bar
  mBar = std::make_shared<ProgressBarComponent>(mWindow, 1);
  mGrid.setEntry(mBar, Vector2i(1, 2), false, true, Vector2i(1,1) );

  // ETA
  mFooter = std::make_shared<TextComponent>(mWindow, _("START DOWNLOADING..."), menuTheme->menuTextSmall.font, menuTheme->menuTextSmall.color, TextAlignment::Left);
  mGrid.setEntry(mFooter, Vector2i(1, 3), false, false, Vector2i(1,1) );

  // Background
  mBackground.setImagePath(menuTheme->menuBackground.path);
  mBackground.setCenterColor(menuTheme->menuBackground.color);
  mBackground.setEdgeColor(menuTheme->menuBackground.color);
  mBackground.fitTo({ width, height }, Vector3f::Zero(), Vector2f(-32, -32));

  mGrid.setColWidthPerc(0, 0.02f);
  mGrid.setColWidthPerc(1, 0.96f);
  mGrid.setColWidthPerc(2, 0.02f);

  // Set grid size
  float titlePercent = (mTitle->getFont()->getLetterHeight() * 2.6f) / height;
  float pbPercent = 0.08f;
  float etaPercent = (mFooter->getFont()->getLetterHeight() * 3.6f) / height;
  float textPercent = 1.0f - (titlePercent + pbPercent + etaPercent);
  constexpr float mainColWidth = 0.96f;
  constexpr float marginWidth = (1.0f - mainColWidth) / 2;

  mGrid.setColWidthPerc(0, marginWidth, false);
  mGrid.setColWidthPerc(1, mainColWidth, false);
  mGrid.setColWidthPerc(2, marginWidth, false);
  mGrid.setRowHeightPerc(0, titlePercent, false);
  mGrid.setRowHeightPerc(1, textPercent, false);
  mGrid.setRowHeightPerc(2, pbPercent, false);
  mGrid.setRowHeightPerc(3, etaPercent, false);

  mText->setSize(width * mainColWidth, 0);

  mGrid.setSize(width, height);

  // Window
  setSize(width, height);
  setPosition((Renderer::Instance().DisplayWidthAsFloat() - width) / 2,
              (Renderer::Instance().DisplayHeightAsFloat() - height) / 2);

  // Avoid sleeping!
  mIsProcessing = true;

  // start the thread if not aleady done
  Thread::Start("Download_File");
}

GuiDownloadFile::~GuiDownloadFile()
{
  Thread::Stop();
}

bool GuiDownloadFile::ProcessInput(const InputCompactEvent& event)
{
  if (event.CancelPressed())
  {
    mFileRequest.Cancel();
    Close();
  }
  return Component::ProcessInput(event);
}

bool GuiDownloadFile::getHelpPrompts(Help& help)
{
  mGrid.getHelpPrompts(help);
  help.Set(Help::Cancel(), _("CANCEL"));
  return true;
}

void GuiDownloadFile::Run()
{
  { LOG(LogDebug) << "[DownloadFile] Start"; }

  // Set boot partition R/W
  if (system("mount -o remount,rw /boot") != 0)
  {
    { LOG(LogError) << "[DownloadFile] Cannot mount /boot RW!"; }
    mSender.Send(-1);
  }
  mTimeReference = DateTime();

  // Check free bytes on share partition
  if (RecalboxSystem::isFreeSpaceLimit())
  {
    String message = _("You must have at least %dGB free on 'SHARE' partition!").Replace("%d", String(RecalboxSystem::GetMinimumFreeSpaceOnSharePartition() >> 30));
    mWindow.displayMessage(message);
    Close();
    mSender.Send(-1);
    return;
  }

  if (destinationFileName.length() == 0){
      { LOG(LogError) << "[DownloadFile] Invalid URL: " << mUrl; }
      mWindow.displayMessage("Invalid URL. Can not extract filename from Url:\n"+mUrl);
      Close();
      mSender.Send(-1);
      return;
  }
  { LOG(LogDebug) << "[DownloadFile] Target path " << destination.ToString(); }

  mTimeReference = DateTime();
  mFileRequest.Execute(mUrl, destination, this);

  if (mTotalSize != 0){
    if (destination.Size() == mTotalSize) //Download complete
    {
      Close();
      return;
    }else if (destination.Size() && mTotalSize == -1){  //Download complete but server report mTotalSize == -1
      Close();
      return;
    }else{  //Download NOT complete
      // Try again
    }
  }

  (void)destination.Delete();
  mSender.Send(-1);
}

void GuiDownloadFile::DownloadProgress(const HttpClient&, long long int currentSize, long long int expectedSize)
{
  // Store data and synchronize
  mTotalSize = expectedSize;
  mCurrentSize = currentSize;
  mSender.Send(0);
}

void GuiDownloadFile::ReceiveSyncMessage(int code)
{
  if (code == 0)
  {
    // Load size into progress bar component
    mBar->setMaxValue(mTotalSize);
    mBar->setCurrentValue(mCurrentSize);

    // Elapsed time
    if (mCurrentSize != 0)
    {
      TimeSpan elapsed = DateTime() - mTimeReference;
      TimeSpan eta((elapsed.TotalMilliseconds() * (mTotalSize - mCurrentSize)) / mCurrentSize);

      String text = mMessage;
      text.Replace("[F1]", addThousandSeparator(std::to_string(mTotalSize)));
      text.Replace("[F2]", elapsed.ToTimeString());
      text.Replace("[F3]", eta.ToTimeString());
      mText->setText(text);

      text = mDownloadedSize;
      text.Replace("[F0]", addThousandSeparator(std::to_string(mCurrentSize)));
      mFooter->setText(text);

      mTitle->setText("DOWNLOADING... "+mBar->getText());
    }
  }
  else if (code < 0)
  {
    mFooter->setText(mError);
    mGrid.onSizeChanged();
  }
}


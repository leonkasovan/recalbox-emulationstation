#pragma once

#include <components/ITextListComponentOverlay.h>
#include <utils/String.h>
#include <memory>
#include <audio/AudioManager.h>

enum class HorizontalAlignment : char
{
    Left,
    Center,
    Right
};

struct TextListData
{
  std::shared_ptr<TextCache> textCache;
	signed char colorId;
  signed char colorBackgroundId;
  HorizontalAlignment hzAlignement;
  bool useHzAlignment;
};

//A graphical list. Supports multiple colors for rows and scrolling.
template <typename T>
class TextListComponent : public IList<TextListData, T>
{
protected:
	using IList<TextListData, T>::mEntries;
	using IList<TextListData, T>::listUpdate;
	using IList<TextListData, T>::listInput;
	using IList<TextListData, T>::listRenderTitleOverlay;
	using IList<TextListData, T>::getTransform;
	using IList<TextListData, T>::mSize;
	using IList<TextListData, T>::mCursor;
  using typename IList<TextListData, T>::Entry;

public:
	using IList<TextListData, T>::size;
	using IList<TextListData, T>::isScrolling;
	using IList<TextListData, T>::stopScrolling;

	explicit TextListComponent(WindowManager& window);

	void SetOverlayInterface(ITextListComponentOverlay<T>* overlay) { mOverlay = overlay; }

  [[nodiscard]] const std::shared_ptr<Font>& getFont() const { return mFont; }

	bool ProcessInput(const InputCompactEvent& event) override;
	void Update(int deltaTime) override;
	void Render(const Transform4x4f& parentTrans) override;
	void applyTheme(const ThemeData& theme, const String& view, const String& element, ThemeProperties properties) override;

	void add(const String& name, const T& obj, int colorId, bool toTheBeginning = false);
  void add(const String& name, const T& obj, int colorId, signed char colorBackgroundId, HorizontalAlignment alignment);
  void changeTextAt(int index, const String& name);
  void changeBackgroundColorAt(int index, int colorIndex);
  int Lookup(T object);

  inline void setSelectedAt(int index, const T& object) { mEntries[index].object = object; }
	inline void setAlignment(HorizontalAlignment align) { mAlignment = align; }
	inline void setCursorChangedCallback(const std::function<void(CursorState)>& func) { mCursorChangedCallback = func; }
	inline void setFont(const std::shared_ptr<Font>& font)
	{
		mFont = font;
		for (auto& entry : mEntries)
			entry.data.textCache.reset();
	}

	inline void setUppercase(bool uppercase) 
	{
		(void)uppercase;
		mUppercase = true; // TODO: Check
		for (auto& entry : mEntries)
			entry.data.textCache.reset();
	}

	inline void setSelectorHeight(float selectorScale) { mSelectorHeight = selectorScale; }
	inline void setSelectorOffsetY(float selectorOffsetY) { mSelectorOffsetY = selectorOffsetY; }
	inline void setSelectorColor(unsigned int color) { mSelectorColor = color; updateBarColor(); }
	inline void setSelectedColor(unsigned int color) { mSelectedColor = color; updateBarColor(); }
	inline void setScrollSound(AudioManager::AudioHandle sound) { mScrollSound = sound; }
	inline void setColor(unsigned int id, unsigned int color) { mColors[id] = color; }
	inline void setSound(AudioManager::AudioHandle sound) { mScrollSound = sound; }
	inline void setLineSpacing(float lineSpacing) { mLineSpacing = lineSpacing; }
  inline void setHorizontalMargin(float horizontalMargin) { mHorizontalMargin = horizontalMargin; }

  [[nodiscard]] inline float EntryHeight() const { return mFont->getSize() * mLineSpacing; }
  [[nodiscard]] inline unsigned int Color(unsigned int id) const { return mColors[id]; }

  [[nodiscard]] inline float getHorizontalMargin() const { return mHorizontalMargin; }

  protected:
	virtual void onScroll(int amt) { (void)amt; AudioManager::Instance().PlaySound(mScrollSound); }
	virtual void onCursorChanged(const CursorState& state);

private:
  void updateBarColor()
  {
    unsigned char lr = mSelectorColor >> 24;
    unsigned char lg = (mSelectorColor >> 16) & 0xFF;
    unsigned char lb = (mSelectorColor >> 8) & 0xFF;
    unsigned char la = mSelectedColor & 0xFF;
    unsigned char rr = mSelectedColor >> 24;
    unsigned char rg = (mSelectedColor >> 16) & 0xFF;
    unsigned char rb = (mSelectedColor >> 8) & 0xFF;
    unsigned char ra = mSelectedColor & 0xFF;
    mBarColor = (((lr + rr) >> 1) << 24) |
                (((lg + rg) >> 1) << 16) |
                (((lb + rb) >> 1) << 8) |
                ((la + ra) >> 1);
  }

  enum class MarqueeSteps
  {
    LeftPause,     //!< Start with a smart pause
    ScrollToRight, //!< Scroll until th elast char is visible
    RightPause,    //!< Smart pause again to let the user read the end
    RollOver,      //!< Scroll until text #2 left part is on the start position and then restart
  };

  static constexpr int sBarFixedTime = 200;
  static constexpr int sBarFadeTime = 600;
  static constexpr int sMarqueeSpeed1 = 80; // In pixel per seconds
  static constexpr int sMarqueeSpeed2 = 160; // In pixel per seconds
  static constexpr int sMarqueePause = 1000; // In milliseconds
  static constexpr unsigned int COLOR_ID_COUNT = 8;

  ImageComponent mSelectorImage;
  unsigned int mColors[COLOR_ID_COUNT];
  std::shared_ptr<Font> mFont;
  std::function<void(CursorState state)> mCursorChangedCallback;
  AudioManager::AudioHandle mScrollSound;

  ITextListComponentOverlay<T>* mOverlay;

  int mMarqueeOffset;
  int mMarqueeTime;
  MarqueeSteps mMarqueeStep;
	HorizontalAlignment mAlignment;

  float mHorizontalMargin;
	float mLineSpacing;
	float mSelectorHeight;
	float mSelectorOffsetY;
	unsigned int mSelectorColor;
	unsigned int mSelectedColor;
  unsigned int mBarColor;
  int mBarTimer;
  bool mShowBar;
  bool mUppercase;
};

template<typename T>
int TextListComponent<T>::Lookup(const T object)
{
  for(int i = mEntries.size(); --i >= 0; )
    if (mEntries[i].object == object)
      return i;
  return -1;
}

template <typename T>
TextListComponent<T>::TextListComponent(WindowManager& window)
  :	IList<TextListData, T>(window)
  , mSelectorImage(window)
  , mColors{ 0x0000FFFF, 0x00FF00FF}
  , mFont(Font::get(FONT_SIZE_MEDIUM))
  , mScrollSound(0)
  , mOverlay(nullptr)
  , mMarqueeOffset(0)
  , mMarqueeTime(0)
  , mMarqueeStep(MarqueeSteps::LeftPause)
  , mAlignment(HorizontalAlignment::Center)
  , mHorizontalMargin(0)
  , mLineSpacing(1.5f)
  , mSelectorHeight(mFont->getSize() * 1.5f)
  , mSelectorOffsetY(0)
  , mSelectorColor(0x000000FF)
  , mSelectedColor(0)
  , mBarColor(0x00000080)
  , mBarTimer(-1)
  , mShowBar(false)
  , mUppercase(false)
{
}

template <typename T>
void TextListComponent<T>::Render(const Transform4x4f& parentTrans)
{
	Transform4x4f trans = parentTrans * getTransform();
  trans.round();
	std::shared_ptr<Font>& font = mFont;

	if(size() == 0) return;

	const float entrySize = font->getSize() * mLineSpacing;

	int startEntry = 0;

	//number of entries that can fit on the screen simultaniously
	int screenCount = (int)mSize.y() / (int)entrySize; //(int)(mSize.y() / entrySize + 0.5f);
	
	if(size() >= screenCount)
	{
		startEntry = mCursor - screenCount/2;
		if(startEntry < 0) startEntry = 0;
		if(startEntry >= size() - screenCount) startEntry = size() - screenCount;
	}

	int listCutoff = startEntry + screenCount;
	if(listCutoff > size())	listCutoff = size();

  // clip to inside margins
  Vector3f dim(mSize.x(), mSize.y(), 0);
  dim = trans * dim - trans.translation();
  Renderer::Instance().PushClippingRect(Vector2i((int)(trans.translation().x()/* + mHorizontalMargin*/), (int)trans.translation().y()),
                                        Vector2i((int)(dim.x() /*- mHorizontalMargin*/), (int)dim.y()));

  // draw selector bar
	if(startEntry < listCutoff)
	{
		if (mSelectorImage.hasImage())
    {
			mSelectorImage.setPosition(0.f, (mCursor - startEntry)*entrySize + mSelectorOffsetY, 0.f);
      mSelectorImage.Render(trans);
		} else
    {
			Renderer::SetMatrix(trans);
			Renderer::DrawRectangle(0.f, (mCursor - startEntry)*entrySize + mSelectorOffsetY, mSize.x(), mSelectorHeight, mSelectorColor);
		}
	}

	// Draw text items
  float y = 0;
  float leftMargin = 0.f;
  float rightMargin = 0.f;
  float previousLeftMargin = 0.f;
  float previousRightMargin = 0.f;
  for (int i = startEntry; i <= listCutoff; i++)
	{
    // Last line might be out of list due to display constraints
    if (i >= size()) continue;

		typename IList<TextListData, T>::Entry& entry = mEntries[i];

		if ((unsigned int)entry.data.colorBackgroundId < COLOR_ID_COUNT)
    {
      Renderer::SetMatrix(trans);
      Renderer::DrawRectangle(0.f, (float)(i - startEntry) * entrySize + mSelectorOffsetY, mSize.x(), mSelectorHeight,
                         mColors[entry.data.colorBackgroundId]);
    }

    unsigned int color = (mCursor == i && (mSelectedColor != 0)) ? mSelectedColor : mColors[entry.data.colorId];

		if(!entry.data.textCache)
			entry.data.textCache = std::unique_ptr<TextCache>(font->buildTextCache(mUppercase ? entry.name.ToUpperCaseUTF8() : entry.name, 0, 0, 0x000000FF));

		entry.data.textCache->setColor(color);

		Vector3f offset(leftMargin, y, 0);
		Vector2f size(mSize.x() - (leftMargin + rightMargin), entrySize);

    if (mOverlay != nullptr)
    {
      // overlay?
      leftMargin = mOverlay->OverlayGetLeftOffset(entry.object);
      rightMargin = mOverlay->OverlayGetRightOffset(entry.object);
      if (leftMargin != previousLeftMargin || rightMargin != previousRightMargin)
      {
        Renderer::Instance().PopClippingRect();
        Renderer::Instance().PushClippingRect(Vector2i((int)(trans.translation().x() + leftMargin), (int)trans.translation().y()),
                                              Vector2i((int)(dim.x() - (leftMargin + rightMargin)), (int)dim.y()));
      }
      previousLeftMargin = leftMargin;
      previousRightMargin = rightMargin;
    }

    switch(entry.data.useHzAlignment ? entry.data.hzAlignement : mAlignment)
		{
      case HorizontalAlignment::Left:
        offset[0] = mHorizontalMargin;
        break;
      case HorizontalAlignment::Center:
        offset[0] = (size.x() - entry.data.textCache->metrics.size.x()) / 2;
        if(offset[0] < mHorizontalMargin) offset[0] = mHorizontalMargin;
        break;
      case HorizontalAlignment::Right:
        offset[0] = (size.x() - entry.data.textCache->metrics.size.x()) - mHorizontalMargin;
        if(offset[0] < mHorizontalMargin)	offset[0] = mHorizontalMargin;
        break;
		  default : break;
		}

    if(mCursor == i) offset[0] -= mMarqueeOffset;

		Transform4x4f drawTrans = trans;
		drawTrans.translate(offset);
		Renderer::SetMatrix(drawTrans);
		font->renderTextCache(entry.data.textCache.get());

    if (mCursor == i && mMarqueeOffset > 0)
    {
      offset[0] += entry.data.textCache->metrics.size.x() + mSize.x() / 4;
      drawTrans = trans;
      drawTrans.translate(offset);
      Renderer::SetMatrix(drawTrans);
      font->renderTextCache(entry.data.textCache.get());
    }

    y += entrySize;
	}

  Renderer::Instance().PopClippingRect();

  // Draw bar
  if (mShowBar || mBarTimer >= 0)
  {
    int verticalSize = (int)dim.y() - 4;
    int maxOffset = (int)mEntries.size() - screenCount;
    if (maxOffset > 0)
    {
      int barHeight = (int) (((float) verticalSize * (float) screenCount) / (float) mEntries.size());

      if ((barHeight < Renderer::Instance().DisplayHeightAsInt() / 40) &&
          (verticalSize >= Renderer::Instance().DisplayHeightAsInt() / 20))
        barHeight = Renderer::Instance().DisplayHeightAsInt() / 40;
      int yOffset = (int) (((float) startEntry * (float) (verticalSize - barHeight)) / (float)maxOffset);

      int alpha = mShowBar || mBarTimer > sBarFadeTime ? 0xFF : (255 * mBarTimer) / sBarFadeTime;

      int screenWidth = Renderer::Instance().DisplayWidthAsInt();
      int barWidth = screenWidth > 1024 ? 6 : screenWidth > 480 ? 4 : 2;
      Renderer::SetMatrix(trans);
      Renderer::DrawRectangle((int) mSize.x() - 2 - barWidth, 2 + yOffset, barWidth, barHeight, (mBarColor & 0xFFFFFF00) | alpha);
    }
  }

  // Overlay?
  if (mOverlay != nullptr)
  {
    Renderer::Instance().PushClippingRect(Vector2i((int) (trans.translation().x()/* + mHorizontalMargin*/), (int) trans.translation().y()),
                                          Vector2i((int) (dim.x() /*- mHorizontalMargin*/), (int) dim.y()));

    // Draw overlay
    y = 0;
    for (int i = startEntry; i < listCutoff; i++)
    {
      typename IList<TextListData, T>::Entry& entry = mEntries[i];

      Vector3f position(leftMargin, y, 0);
      Vector2f size(mSize.x(), entrySize);

      unsigned int color = (mCursor == i && (mSelectedColor != 0)) ? mSelectedColor : mColors[entry.data.colorId];

      Transform4x4f drawTrans = trans;
      drawTrans.translate(position);
      Renderer::SetMatrix(drawTrans);

      mOverlay->OverlayApply(Vector2f(0), size, entry.object, color);

      y += entrySize;
    }

    Renderer::Instance().PopClippingRect();
  }

  listRenderTitleOverlay(trans);

	Component::renderChildren(trans);
}

template <typename T>
bool TextListComponent<T>::ProcessInput(const InputCompactEvent& event)
{
	if(size() > 0)
	{
    if (event.AnyPrimaryDownPressed())
    {
      listInput(1);
      mShowBar = true;
      return true;
    }
    else if (event.AnyPrimaryUpPressed())
    {
      listInput(-1);
      mShowBar = true;
      return true;
    }
    else if (event.R1Pressed())
    {
      listInput(10);
      mShowBar = true;
      return true;
    }
    else if (event.L1Pressed())
    {
      listInput(-10);
      mShowBar = true;
      return true;
    }
    else if (event.AnyPrimaryDownReleased() || event.AnyPrimaryUpReleased() || event.R1Released() || event.L1Released())
    {
      mBarTimer = sBarFadeTime + sBarFixedTime;
      mShowBar = false;
      stopScrolling();
    }
	}

	return Component::ProcessInput(event);
}

template <typename T>
void TextListComponent<T>::Update(int deltaTime)
{
  if (mBarTimer > 0) mBarTimer -= deltaTime;

	listUpdate(deltaTime);
	if(!isScrolling() && size() > 0 && mEntries[mCursor].data.textCache)
	{
		//if we're not scrolling and this object's text goes outside our size, marquee it!
		const float textWidth = mEntries[mCursor].data.textCache->metrics.size.x();

		//it's long enough to marquee
    float width = mSize.x() - mHorizontalMargin - 2;
    if (mOverlay != nullptr) width -= mOverlay->OverlayGetLeftOffset(mEntries[mCursor].object) +
                                      mOverlay->OverlayGetRightOffset(mEntries[mCursor].object);
    else width -= mHorizontalMargin;

		if (textWidth > width)
    {
      switch (mMarqueeStep)
      {
        case MarqueeSteps::LeftPause:
        {
          mMarqueeOffset = 0;
          if (mMarqueeTime > sMarqueePause) { mMarqueeTime = 0; mMarqueeStep = MarqueeSteps::ScrollToRight; }
          break;
        }
        case MarqueeSteps::ScrollToRight:
        {
          mMarqueeOffset = (mMarqueeTime * sMarqueeSpeed1) / 1000;
          if (mMarqueeOffset >= (int)(textWidth - width)) { mMarqueeTime = 0; mMarqueeOffset = (int)(textWidth - width); mMarqueeStep = MarqueeSteps::RightPause;}
          break;
        }
        case MarqueeSteps::RightPause:
        {
          mMarqueeOffset = (int)(textWidth - width);
          if (mMarqueeTime > sMarqueePause) { mMarqueeTime = 0; mMarqueeStep = MarqueeSteps::RollOver; }
          break;
        }
        case MarqueeSteps::RollOver:
        {
          mMarqueeOffset = (int)(textWidth - width) + (mMarqueeTime * sMarqueeSpeed2) / 1000;
          if (mMarqueeOffset >= textWidth + mSize.x() / 4) { mMarqueeTime = 0; mMarqueeOffset = 0; mMarqueeStep = MarqueeSteps::LeftPause;}
          break;
        }
        default: break;
      }
      mMarqueeTime += deltaTime;
    }
	}

  Component::Update(deltaTime);
}

//list management stuff
template <typename T>
void TextListComponent<T>::add(const String& name, const T& obj, int color, bool toTheBeginning)
{
	assert((unsigned int)color < COLOR_ID_COUNT);

	typename IList<TextListData, T>::Entry entry;
	entry.name = name;
	entry.object = obj;
	entry.data.colorId = color;
  entry.data.colorBackgroundId = -1;
	entry.data.useHzAlignment = false;
	if (toTheBeginning) {
    ((IList< TextListData, T >*)this)->unshift(entry);
	} else {
    ((IList< TextListData, T >*)this)->add(entry);
	}
}

template <typename T>
void TextListComponent<T>::add(const String& name, const T& obj, int color, signed char colorBackground, HorizontalAlignment align)
{
  assert((unsigned int)color < COLOR_ID_COUNT);

  typename IList<TextListData, T>::Entry entry;
  entry.name = name;
  entry.object = obj;
  entry.data.colorId = color;
  entry.data.colorBackgroundId = colorBackground;
  entry.data.useHzAlignment = true;
  entry.data.hzAlignement = align;
  ((IList< TextListData, T >*)this)->add(entry);
}

template <typename T>
void TextListComponent<T>::changeTextAt(int index, const String& name)
{
  ((IList< TextListData, T >*)this)->changeCursorName(index, name);
}

template <typename T>
void TextListComponent<T>::changeBackgroundColorAt(int index, int colorIndex)
{
  ((IList< TextListData, T >*)this)->getSelectedEntryAt(index).colorBackgroundId = colorIndex;
}

template <typename T>
void TextListComponent<T>::onCursorChanged(const CursorState& state)
{
  mMarqueeStep = MarqueeSteps::LeftPause;
	mMarqueeOffset = 0;
	mMarqueeTime = 0;

	if(mCursorChangedCallback)
		mCursorChangedCallback(state);
}

template <typename T>
void TextListComponent<T>::applyTheme(const ThemeData& theme, const String& view, const String& element, ThemeProperties properties)
{
	Component::applyTheme(theme, view, element, properties);

	const ThemeElement* elem = theme.getElement(view, element, "textlist");
	if (elem == nullptr)
		return;

	if (hasFlag(properties, ThemeProperties::Color))
	{
		if(elem->HasProperty("selectorColor"))
			setSelectorColor((unsigned int)elem->AsInt("selectorColor"));
		if(elem->HasProperty("selectedColor"))
			setSelectedColor((unsigned int)elem->AsInt("selectedColor"));
		if(elem->HasProperty("primaryColor"))
			setColor(0, (unsigned int)elem->AsInt("primaryColor"));
		if(elem->HasProperty("secondaryColor"))
			setColor(1, (unsigned int)elem->AsInt("secondaryColor"));
	}

	setFont(Font::getFromTheme(elem, properties, mFont));
	
	if (hasFlag(properties, ThemeProperties::Sound) && elem->HasProperty("scrollSound"))
  {
    Path soundPath = Path(elem->AsString("sound"));
    setSound(AudioManager::Instance().LoadSound(soundPath));
  }

	if (hasFlag(properties, ThemeProperties::Alignment))
	{
		if(elem->HasProperty("alignment"))
		{
			const String& str = elem->AsString("alignment");
			if(str == "left")
				setAlignment(HorizontalAlignment::Left);
			else if(str == "center")
				setAlignment(HorizontalAlignment::Center);
			else if(str == "right")
				setAlignment(HorizontalAlignment::Right);
			else
      { LOG(LogError) << "[TextListComponent] Unknown TextListComponent alignment \"" << str << "\"!"; }
		}
		if(elem->HasProperty("horizontalMargin"))
		{
			mHorizontalMargin = elem->AsFloat("horizontalMargin") * (this->mParent ? this->mParent->getSize().x() : Renderer::Instance().DisplayWidthAsFloat());
		}
	}

	if (hasFlag(properties, ThemeProperties::ForceUppercase) && elem->HasProperty("forceUppercase"))
		setUppercase(elem->AsBool("forceUppercase"));

	if (hasFlag(properties, ThemeProperties::LineSpacing))
	{
		if(elem->HasProperty("lineSpacing"))
			setLineSpacing(elem->AsFloat("lineSpacing"));
		if(elem->HasProperty("selectorHeight"))
		{
			setSelectorHeight(elem->AsFloat("selectorHeight") * Renderer::Instance().DisplayHeightAsFloat());
		} else {
			setSelectorHeight(mFont->getSize() * mLineSpacing);
		}
		if(elem->HasProperty("selectorOffsetY"))
		{
			float scale = this->mParent ? this->mParent->getSize().y() : Renderer::Instance().DisplayHeightAsFloat();
			setSelectorOffsetY(elem->AsFloat("selectorOffsetY") * scale);
		} else {
			setSelectorOffsetY(0.0);
		}
	}

	if (elem->HasProperty("selectorImagePath"))
	{
		Path path(elem->AsString("selectorImagePath"));
		bool tile = elem->HasProperty("selectorImageTile") && elem->AsBool("selectorImageTile");
		mSelectorImage.setImage(path, tile);
		mSelectorImage.setSize(mSize.x(), mSelectorHeight);
		mSelectorImage.setColorShift(mSelectorColor);
	} else {
		mSelectorImage.setImage(Path());
	}
}

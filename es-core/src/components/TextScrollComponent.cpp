//
// Created by bkg2k on 04/12/23.
//

#include "TextScrollComponent.h"

#include "components/TextScrollComponent.h"
#include "Renderer.h"
#include "utils/Log.h"
#include "WindowManager.h"
#include "themes/ThemeData.h"

TextScrollComponent::TextScrollComponent(WindowManager&window)
  : Component(window)
  , mFont(Font::get(FONT_SIZE_MEDIUM))
  , mStep(ScrollSteps::LeftPause)
  , mMarqueeTime(0)
  , mOffset(0)
  , mColor(0x000000FF)
  , mOriginColor(0x000000FF)
  , mBgColor(0)
  , mColorOpacity(0xFF)
  , mBgColorOpacity(0)
  , mHorizontalAlignment(TextAlignment::Left)
  , mVerticalAlignment(TextAlignment::Center)
  , mRenderBackground(false)
  , mUppercase(false)
{
  mPosition = Vector3f::Zero();
  mSize = Vector2f::Zero();
}

TextScrollComponent::TextScrollComponent(WindowManager&window, const String& text, const std::shared_ptr<Font>& font, unsigned int color)
  : TextScrollComponent(window)
{
  mFont = font;
  mText = text;
  mColor = color;
  mColorOpacity = (unsigned char)(color & 0xFF);
  mOriginColor = color;
  onTextChanged();
}

TextScrollComponent::TextScrollComponent(WindowManager&window, const String& text, const std::shared_ptr<Font>& font, unsigned int color, TextAlignment align)
  : TextScrollComponent(window)
{
  mFont = font;
  mText = text;
  mColor = color;
  mColorOpacity = (unsigned char)(color & 0xFF);
  mOriginColor = color;
  mHorizontalAlignment = align;
  onTextChanged();
}

TextScrollComponent::TextScrollComponent(WindowManager&window, const String& text, const std::shared_ptr<Font>& font, unsigned int color, TextAlignment align,
                             Vector3f pos, Vector2f size, unsigned int bgcolor)
  : TextScrollComponent(window, text, font, color, align)
{
  mBgColor = bgcolor;
  mBgColorOpacity = (unsigned char)(bgcolor & 0xFF);
  mPosition = pos;
  mSize = size;
}

void TextScrollComponent::setFont(const std::shared_ptr<Font>& font)
{
  mFont = font;
  onTextChanged();
}

void TextScrollComponent::setColor(unsigned int color)
{
  mColor = color;
  unsigned char opacity = mColor & 0x000000FF;
  Component::setOpacity(opacity);

  mColorOpacity = mColor & 0x000000FF;

  onColorChanged();
}

//  Set the color of the background box
void TextScrollComponent::setBackgroundColor(unsigned int color)
{
  mBgColor = color;
  mBgColorOpacity = mBgColor & 0x000000FF;
}

//  Scale the opacity
void TextScrollComponent::setOpacity(unsigned char opacity)
{
  // This method is mostly called to do fading in-out of the Text component element.
  // Therefore, we assume here that opacity is a fractional value (expressed as an int 0-255),
  // of the opacity originally set with setColor() or setBackgroundColor().

  unsigned char o = (unsigned char)((float)opacity / 255.f * (float) mColorOpacity);
  mColor = (mColor & 0xFFFFFF00) | (unsigned char) o;

  unsigned char bgo = (unsigned char)((float)opacity / 255.f * (float)mBgColorOpacity);
  mBgColor = (mBgColor & 0xFFFFFF00) | (unsigned char)bgo;

  onColorChanged();

  Component::setOpacity(opacity);
}

void TextScrollComponent::setText(const String& text)
{
  mText = text;
  if (mUppercase) mText.UpperCaseUTF8();
  onTextChanged();
}

void TextScrollComponent::setUppercase(bool uppercase)
{
  mUppercase = uppercase;
  onTextChanged();
}

void TextScrollComponent::Update(int deltaTime)
{
  mOffset = 0;
  if (mTextCache && mTextCache->metrics.size.x() > mSize.x())
  {
    int textWidth = mTextCache->metrics.size.x();
    int width = mSize.x();
    if (textWidth > width)
    {
      switch (mStep)
      {
        case ScrollSteps::LeftPause:
        {
          mOffset = 0;
          if (mMarqueeTime > sScrollPause) { mMarqueeTime = 0; mStep = ScrollSteps::ScrollToRight; }
          break;
        }
        case ScrollSteps::ScrollToRight:
        {
          mOffset = (mMarqueeTime * sScrollSpeed1) / 1000;
          if (mOffset >= (int)(textWidth - width)) { mMarqueeTime = 0; mOffset = (int)(textWidth - width); mStep = ScrollSteps::RightPause;}
          break;
        }
        case ScrollSteps::RightPause:
        {
          mOffset = (int)(textWidth - width);
          if (mMarqueeTime > sScrollPause) { mMarqueeTime = 0; mStep = ScrollSteps::RollOver; }
          break;
        }
        case ScrollSteps::RollOver:
        {
          mOffset = (int)(textWidth - width) + (mMarqueeTime * sScrollSpeed2) / 1000;
          if (mOffset >= textWidth + mSize.x() / 4) { mMarqueeTime = 0; mOffset = 0; mStep = ScrollSteps::LeftPause;}
          break;
        }
        default: break;
      }
      mMarqueeTime += deltaTime;
    }
  }

  Component::Update(deltaTime);
}

void TextScrollComponent::Render(const Transform4x4f& parentTrans)
{
  if(mThemeDisabled) return;
  Transform4x4f trans = parentTrans * getTransform();

  if (mRenderBackground)
  {
    Renderer::SetMatrix(trans);
    Renderer::DrawRectangle(0.f, 0.f, mSize.x(), mSize.y(), mBgColor);
  }

  if(mTextCache)
  {
    const Vector2f& textSize = mTextCache->metrics.size;
    float xOff = 0;
    float yOff = 0;
    switch(mVerticalAlignment)
    {
      case TextAlignment::Bottom: yOff = (getSize().y() - textSize.y()); break;
      case TextAlignment::Center: yOff = (getSize().y() - textSize.y()) / 2.0f; break;
      case TextAlignment::Top:
      case TextAlignment::Left:
      case TextAlignment::Right: break;
    }
    switch(mHorizontalAlignment)
    {
      case TextAlignment::Bottom: xOff = (getSize().x() - textSize.x()); break;
      case TextAlignment::Center: if (mTextCache->metrics.size.x() <= mSize.x()) xOff = (getSize().x() - textSize.x()) / 2.0f; break;
      case TextAlignment::Top:
      case TextAlignment::Left:
      case TextAlignment::Right: break;
    }
    Vector3f off(xOff + ((mHorizontalAlignment == TextAlignment::Right) ? mOffset : - mOffset), yOff, 0);

    // Get clipping area
    Vector2i clipPos((int)trans.translation().x(), (int)trans.translation().y());
    Vector3f dimScaled = trans * Vector3f(mSize.x(), mSize.y(), 0);
    Vector2i clipDim((int)(dimScaled.x() - trans.translation().x()), (int)(dimScaled.y() - trans.translation().y()));
    Renderer::Instance().PushClippingRect(clipPos, clipDim);

    // Render text
    trans.translate(off);
    trans.round();
    Renderer::SetMatrix(trans);
    mFont->renderTextCache(mTextCache.get());
    if (mOffset != 0)
    {
      int subOffset = mTextCache->metrics.size.x() + mSize.x() / 4;
      if (mHorizontalAlignment == TextAlignment::Right) subOffset = -subOffset;
      trans.translate(subOffset, 0);
      trans.round();
      Renderer::SetMatrix(trans);
      mFont->renderTextCache(mTextCache.get());
    }

    Renderer::Instance().PopClippingRect();
  }
}

void TextScrollComponent::onTextChanged()
{
  if(!mFont || mText.empty())
  {
    mTextCache.reset();
    return;
  }

  String text = mUppercase ? mText.ToUpperCaseUTF8() : mText;
  mTextCache = std::shared_ptr<TextCache>(mFont->buildTextCache(text, Vector2f(0, 0), (mColor >> 8 << 8) | mOpacity, mSize.x(), TextAlignment::Left, 0));
}

void TextScrollComponent::onColorChanged()
{
  if(mTextCache)
    mTextCache->setColor(mColor);
}

void TextScrollComponent::setHorizontalAlignment(TextAlignment align)
{
  if (align != TextAlignment::Left && align != TextAlignment::Right)
    align = TextAlignment::Left;
  mHorizontalAlignment = align;
  onTextChanged();
}

void TextScrollComponent::applyTheme(const ThemeData& theme, const String& view, const String& element, ThemeProperties properties)
{
  Component::applyTheme(theme, view, element, properties);

  const ThemeElement* elem = theme.getElement(view, element, "textscroll");
  if(elem == nullptr)
    return;

  if (hasFlag(properties, ThemeProperties::Color) && elem->HasProperty("color"))
    setColor((unsigned int)elem->AsInt("color"));

  setRenderBackground(false);
  if (hasFlag(properties, ThemeProperties::Color) && elem->HasProperty("backgroundColor")) {
    setBackgroundColor((unsigned int)elem->AsInt("backgroundColor"));
    setRenderBackground(true);
  }

  if(hasFlag(properties, ThemeProperties::Alignment) && elem->HasProperty("alignment"))
  {
    String str = elem->AsString("alignment");
    if      (str == "left")   setHorizontalAlignment(TextAlignment::Left);
    else if (str == "center") setHorizontalAlignment(TextAlignment::Center);
    else if (str == "right")  setHorizontalAlignment(TextAlignment::Right);
    else
    { LOG(LogError) << "[TextScrollComponent] Unknown text alignment string: " << str; }
  }

  if (hasFlag(properties, ThemeProperties::Text) && elem->HasProperty("text"))
    setText(elem->AsString("text"));

  if (hasFlag(properties, ThemeProperties::ForceUppercase) && elem->HasProperty("forceUppercase"))
    setUppercase(elem->AsBool("forceUppercase"));

  setFont(Font::getFromTheme(elem, properties, mFont));
}

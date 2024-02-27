//
// Created by bkg2k on 13/10/23.
//

#include <components/PictureComponent.h>
#include <utils/Log.h>
#include <Renderer.h>
#include <help/Help.h>
#include <themes/ThemeData.h>
#include <utils/locale/LocaleHelper.h>

PictureComponent::PictureComponent(WindowManager&window, bool keepRatio, const Path& imagePath, bool forceLoad, bool dynamic)
  : Component(window)
  , mPath(imagePath)
  , mColorShift(0xFFFFFFFF)
  , mFadeOpacity(0.0f)
  , mFading(false)
  , mForceLoad(forceLoad)
  , mDynamic(dynamic)
  , mVisible(true)
  , mKeepRatio(keepRatio)
{
  if (!imagePath.IsEmpty()) setImage(imagePath);
}

void PictureComponent::setImage(const Path& path, bool tile) {
  if ( (mPath == path) && mTexture && (mTexture->isTiled() == tile) ) {
    return;
  }
  mPath = path;

  if (path.IsEmpty() || !ResourceManager::fileExists(path)) {
    mTexture.reset();
  } else {
    mTexture = TextureResource::get(path, tile, mForceLoad, mDynamic);
  }
}

void PictureComponent::setImage(const char* image, size_t length, bool tile)
{
  mPath = "!";
  mTexture.reset();
  mTexture = TextureResource::get(Path::Empty, tile);
  mTexture->initFromMemory(image, length);
}


void PictureComponent::setImage(const std::shared_ptr<TextureResource>& texture) {
  mPath = "!";
  mTexture = texture;
}

void PictureComponent::setColor(unsigned int color) {
  mColorShift = color;
  // Grab the opacity from the color shift because we may need to apply it if
  // fading textures in
  mOpacity = color & 0xff;
}

void PictureComponent::setOpacity(unsigned char opacity)
{
  mOpacity = opacity;
  mColorShift = (mColorShift >> 8 << 8) | mOpacity;
}

void PictureComponent::Render(const Transform4x4f& parentTrans)
{
  if (mThemeDisabled || !mVisible) return;

  Transform4x4f trans = parentTrans * getTransform();
  Renderer::SetMatrix(trans);

  if (mTexture && mOpacity > 0) {
    if (mTexture->isInitialized())
      Renderer::DrawTexture(*mTexture, 0/*Math::roundi(mPosition.x())*/, 0/*Math::roundi(mPosition.y())*/, Math::roundi(mSize.x()), Math::roundi(mSize.y()), mKeepRatio, mColorShift);
    else
    {
      { LOG(LogError) << "[ImageComponent] Image texture is not initialized!"; }
      mTexture.reset();
    }
  }

  Component::renderChildren(trans);
}

void PictureComponent::fadeIn(bool textureLoaded) {
  if (!mForceLoad) {
    if (!textureLoaded) {
      // Start the fade if this is the first time we've encountered the unloaded texture
      if (!mFading) {
        // Start with a zero opacity and flag it as fading
        mFadeOpacity = 0;
        mFading = true;
        // Set the colours to be translucent
        mColorShift = (mColorShift >> 8 << 8) | 0;
      }
    }
    else if (mFading) {
      // The texture is loaded and we need to fade it in. The fade is based on the frame rate
      // and is 1/4 second if running at 60 frames per second although the actual value is not
      // that important
      int opacity = mFadeOpacity + 255 / 15;
      // See if we've finished fading
      if (opacity >= 255) {
        mFadeOpacity = 255;
        mFading = false;
      }
      else {
        mFadeOpacity = (unsigned char)opacity;
      }
      // Apply the combination of the target opacity and current fade
      float newOpacity = (float)mOpacity * ((float)mFadeOpacity / 255.0f);
      mColorShift = (mColorShift >> 8 << 8) | (unsigned char)newOpacity;
    }
  }
}

void PictureComponent::applyTheme(const ThemeData& theme, const String& view, const String& element, ThemeProperties properties)
{
  const ThemeElement* elem = theme.getElement(view, element, "image");
  if (elem == nullptr) {
    return;
  }

  Vector2f scale = getParent() != nullptr ? getParent()->getSize() : Vector2f(Renderer::Instance().DisplayWidthAsFloat(), Renderer::Instance().DisplayHeightAsFloat());

  if (hasFlag(properties, ThemeProperties::Position) && elem->HasProperty("pos")) {
    Vector2f denormalized = elem->AsVector("pos") * scale;
    setPosition(Vector3f(denormalized.x(), denormalized.y(), 0));
  }

  if (hasFlag(properties,ThemeProperties::Size)) {
    if (elem->HasProperty("size")) {
      setSize(elem->AsVector("size") * scale);
      setKeepRatio(false);
    } else if (elem->HasProperty("maxSize")) {
      setSize(elem->AsVector("maxSize") * scale);
      setKeepRatio(true);
    }
  }

  // position + size also implies origin
  if ((hasFlag(properties, ThemeProperties::Origin) || (hasFlags(properties, ThemeProperties::Position, ThemeProperties::Size))) && elem->HasProperty("origin")) {
    setOrigin(elem->AsVector("origin"));
  }

  if (hasFlag(properties, ThemeProperties::Path) && elem->HasProperty("path")) {
    bool tile = (elem->HasProperty("tile") && elem->AsBool("tile"));
    setImage(Path(elem->AsString("path")), tile);
  }

  if (hasFlag(properties, ThemeProperties::Color) && elem->HasProperty("color")) {
    setColor((unsigned int)elem->AsInt("color"));
  }

  if (hasFlag(properties, ThemeProperties::Rotation)) {
    if (elem->HasProperty("rotation")) {
      setRotationDegrees(elem->AsFloat("rotation"));
    }
    if (elem->HasProperty("rotationOrigin")) {
      setRotationOrigin(elem->AsVector("rotationOrigin"));
    }
  }

  if (hasFlag(properties, ThemeProperties::ZIndex) && elem->HasProperty("zIndex")) {
    setZIndex(elem->AsFloat("zIndex"));
  } else {
    setZIndex(getDefaultZIndex());
  }

  if (hasFlag(properties, ThemeProperties::Position) && elem->HasProperty("disabled"))
  {
    mThemeDisabled = elem->AsBool("disabled");
  }
}


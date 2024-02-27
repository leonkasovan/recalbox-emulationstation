//
// Created by bkg2k on 13/10/23.
//
#pragma once

#include "components/base/Component.h"
#include <utils/String.h>
#include <memory>
#include "resources/TextureResource.h"

class PictureComponent : public Component
{
  public:
    PictureComponent(WindowManager&window, bool keepRatio, const Path& imagePath, bool forceLoad, bool dynamic);
    PictureComponent(WindowManager&window, bool forceLoad, bool dynamic)
      : PictureComponent(window, false, Path::Empty, forceLoad, dynamic)
    {
    }
    PictureComponent(WindowManager&window, bool keepRatio, const Path& imagePath, bool forceLoad)
      : PictureComponent(window, keepRatio, imagePath, forceLoad, true)
    {
    }
    PictureComponent(WindowManager&window, bool forceLoad)
      : PictureComponent(window, false, Path::Empty, forceLoad, true)
    {
    }
    explicit PictureComponent(WindowManager& window, bool keepRatio, const Path& imagePath)
      : PictureComponent(window, keepRatio, imagePath, false, true)
    {
    }
    explicit PictureComponent(WindowManager& window)
      : PictureComponent(window, false, Path::Empty, false, true)
    {
    }

    ~PictureComponent() override = default;

    //Loads the image at the given filepath. Will tile if tile is true (retrieves texture as tiling, creates vertices accordingly).
    void setImage(const Path& path, bool tile = false);
    [[nodiscard]] Path getImage() const { return mPath; };

    //Loads an image from memory.
    void setImage(const char* image, size_t length, bool tile = false);
    //Use an already existing texture.
    void setImage(const std::shared_ptr<TextureResource>& texture);

    void setOpacity(unsigned char opacity) override;

    // Resize the image to be as large as possible but fit within a box of this size.
    // Can be set before or after an image is loaded.
    // Never breaks the aspect ratio. setMaxSize() and setResize() are mutually exclusive.
    void setKeepRatio(bool keepRatio) { mKeepRatio = keepRatio; }

    void setColor(unsigned int color) override;

    void Render(const Transform4x4f& parentTrans) override;

    void applyTheme(const ThemeData& theme, const String& view, const String& element, ThemeProperties properties) override;

    /*!
     * @brief Set component visibility
     * @param enabled True (default) to render the component, false to hide it
     */
    void SetVisible(bool visible) { mVisible = visible; }

    /*!
     * @brief Set component visibility
     * @param enabled True (default) to render the component, false to hide it
     */
    [[nodiscard]] bool Visible() const { return mVisible; }

  private:
    Path mPath;
    std::shared_ptr<TextureResource> mTexture;
    unsigned int mColorShift;
    unsigned char mFadeOpacity;
    bool mFading;
    bool mForceLoad;
    bool mDynamic;
    bool mVisible;
    bool mKeepRatio;

    void fadeIn(bool textureLoaded);
};


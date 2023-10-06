//
// Created by bkg2k on 05/06/23.
//
#pragma once

#include <utils/os/fs/Path.h>

/*!
 * @brief Used by different board to require special global action
 */
class ISpecialGlobalAction
{
  public:
    //! Destructor
    virtual ~ISpecialGlobalAction() {}

    /*!
     * @brief Disable OSD
     * @param imagePath OSD image
     * @param x X coordinate in the range of 0.0 (left) ... 1.0 (right)
     * @param y Y coordinate in the range of 0.0 (up) ... 1.0 (bottom)
     * @param width Width in the range of 0.0 (invisible) ... 1.0 (full screen width)
     * @param height Height in the range of 0.0 (invisible) ... 1.0 (full screen height)
     * @param autoCenter if true, x/y are ignored and the image is screen centered
     */
    virtual void EnableOSDImage(const Path& imagePath, float x, float y, float width, float height, float alpha, bool autoCenter) = 0;

    /*!
     * @brief Disable OSD
     */
    virtual void DisableOSDImage() = 0;
};

//
// Created by gugue_u on 13/01/23.
//

#include "utils/os/fs/Path.h"
#include "utils/datetime/DateTime.h"

#pragma once

class SaveState
{
  public:
    explicit SaveState(const Path& path);

    [[nodiscard]] Path GetPath() const { return mPath; }
    [[nodiscard]] Path GetThrumbnail() const { return mThumbnailPath; }
    [[nodiscard]] int GetSlotNumber() const { return mSlotNumber; }
    [[nodiscard]] bool GetIsAuto() const { return mIsAuto; }
    [[nodiscard]] DateTime GetDateTime() const { return mDateTime; }

  private:
    Path mPath;
    Path mThumbnailPath;
    DateTime mDateTime;
    int mSlotNumber;
    bool mIsAuto;
};
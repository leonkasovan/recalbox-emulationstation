//
// Created by digi on 11/27/23.
//

#pragma once

#include "utils/String.h"

class RPiEepromUpdater
{

  public:
    RPiEepromUpdater();

    [[nodiscard]] bool IsUpdateAvailable() const
    { return mUpdateAvailable; }

    [[nodiscard]] String CurrentVersion() const
    { return mCurrentVersion; }

    [[nodiscard]] String LastVersion() const
    { return mLastversion; }

    [[nodiscard]] bool Error() const
    { return mError; }

    bool Update() const;

  private:
    String ExtractVersion(String cmdResult, String updateType);

    std::pair<String, int> Run(bool autoupdate) const;

    bool mError;
    String mCurrentVersion;
    String mLastversion;
    bool mUpdateAvailable;
};

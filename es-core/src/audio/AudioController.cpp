//
// Created by bkg2k on 13/08/2020.
//

#include <utils/IniFile.h>
#include "AudioController.h"

bool AudioController::GetSpecialAudio()
{
  bool result = false;

  // GPI audio configuration must not be changed
  IniFile recalboxBootConf(Path("/boot/recalbox-boot.conf"), false, true);
  if (recalboxBootConf.AsString("case") == "GPiV1:1") result = true;

  return result;
}

String AudioController::SetDefaultPlayback(const String& playbackName)
{
  if (!mHasSpecialAudio)
  {
    String playback = mController.SetDefaultPlayback(playbackName);
    return playback;
  }

  return playbackName;
}

void AudioController::DisableNotification()
{
  mController.DisableNotification();
}

void AudioController::EnableNotification()
{
  mController.EnableNotification();
}

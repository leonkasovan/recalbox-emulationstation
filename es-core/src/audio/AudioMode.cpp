//
// Created by gugue_u on 24/03/2021.
//

#include <RecalboxConf.h>
#include <VideoEngine.h>
#include "AudioMode.h"
#include "AudioManager.h"
#include <views/ViewController.h>


bool AudioModeTools::CanPlayMusic()
{
  AudioMode audioMode = RecalboxConf::Instance().GetAudioMode();
  switch (audioMode)
  {
    case AudioMode::MusicsOnly:
    case AudioMode::MusicsAndVideosSound:
    case AudioMode::MusicsXorVideosSound:
      return true;
    case AudioMode::VideosSoundOnly:
    case AudioMode::None:
      break;
  }
  return false;
}

bool AudioModeTools::CanDecodeVideoSound()
{
  AudioMode audioMode = RecalboxConf::Instance().GetAudioMode();
  switch (audioMode)
  {
    case AudioMode::VideosSoundOnly:
    case AudioMode::MusicsAndVideosSound:
    case AudioMode::MusicsXorVideosSound: return true;
    case AudioMode::MusicsOnly:
    case AudioMode::None: break;
  }
  return false;
}

AudioMode AudioModeTools::AudioModeFromString(const String& audioMode)
{
  if (audioMode == "musiconly") return AudioMode::MusicsOnly;
  if (audioMode == "videosoundonly") return AudioMode::VideosSoundOnly;
  if (audioMode == "musicandvideosound") return AudioMode::MusicsAndVideosSound;
  if (audioMode == "none") return AudioMode::None;
  return AudioMode::MusicsXorVideosSound;
}

const String& AudioModeTools::AudioModeFromEnum(AudioMode audioMode)
{
  switch (audioMode)
  {
    case AudioMode::MusicsXorVideosSound: break;
    case AudioMode::MusicsOnly:
    {
      static String sScraper = "musiconly";
      return sScraper;
    }
    case AudioMode::VideosSoundOnly:
    {
      static String sFileName = "videosoundonly";
      return sFileName;
    }
    case AudioMode::MusicsAndVideosSound:
    {
      static String sFileName = "musicandvideosound";
      return sFileName;
    }
    case AudioMode::None:
    {
      static String sFileName = "none";
      return sFileName;
    }
  }
  static String sFileName = "musicxorvideosound";
  return sFileName;
}
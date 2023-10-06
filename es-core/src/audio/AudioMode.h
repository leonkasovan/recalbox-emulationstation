//
// Created by gugue_u on 24/03/2021.
//

#pragma once

enum class AudioMode
{
    MusicsOnly,
    VideosSoundOnly,
    MusicsAndVideosSound,
    MusicsXorVideosSound,
    None
};

class AudioModeTools
{
  public:
    static bool CanPlayMusic();
    static bool CanDecodeVideoSound();

    static AudioMode AudioModeFromString(const String& audioMode);
    static const String& AudioModeFromEnum(AudioMode audioMode);
};
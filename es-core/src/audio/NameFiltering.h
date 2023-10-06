//
// Created by bkg2k on 14/08/2020.
//
#pragma once

#include <utils/String.h>

class NameFiltering
{
  public:
    enum class Source
    {
      Card,
      Device,
      Mixer,
    };

    /*!
     * @brief Filter some words and replace some others to make ALSA naming more user-friendly
     * @param sourceString source string
     * @param from Name source
     * @return filtered string
     */
    static String Filter(const String& sourceString, Source from);
};

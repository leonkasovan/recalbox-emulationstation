//
// Created by bkg2k on 23/07/23.
//
#pragma once

#include <utils/String.h>

class Url
{
  public:
    /*!
     * @brief Encode the given string in an Url-compliant form
     * @param url Url to encode
     * @return Encoded url
     */
    static String URLEncode(const String& url);

    /*!
     * @brief Decode the given url-compliant string into a plein text string
     * @param url Url to decode
     * @return Decoded url
     */
    static String URLDecode(const String& url);
};


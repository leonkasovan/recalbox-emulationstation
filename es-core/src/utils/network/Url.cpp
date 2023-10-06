//
// Created by bkg2k on 23/07/23.
//

#include "Url.h"

String Url::URLEncode(const String& url)
{
  static const char* HexaChars = "0123456789ABCDEF";

  String result;
  const char* p = url.c_str();

  for (int i = (int)url.length(); --i >= 0;)
  {
    unsigned char C = (unsigned char)*p++;
    if (((C >= 'a') && (C <= 'z')) ||
        ((C >= 'A') && (C <= 'Z')) ||
        ((C >= '0') && (C <= '9')) ||
        (C == '_') ||
        (C == '*') ||
        (C == '.') ||
        (C == '-')) result.Append((char)C);
    else
    {
      char buffer[3] = { '%', HexaChars[C >> 4], HexaChars[C & 0xF] };
      result.Append(buffer, 3);
    }
  }

  return result;
}

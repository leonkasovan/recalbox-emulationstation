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

String Url::URLDecode(const String& url)
{
  String result;
  result.reserve(url.size()); // Allocate only once
  const char* p = url.c_str();

  for (int i = (int)url.length(); --i >= 0;)
    if (char C = *p++; C != '%') result.Append(C);
    else
    {
      char h = *p++ | 0x20; // high quartet lowercase
      char l = *p++ | 0x20; // low quartet lowercase
      result.Append((char)(((h <= 0x39 ? h - 0x30 : (h - 'a') + 10) << 4) | (l <= 0x39 ? l - 0x30 : (l - 'a') + 10)));
      i -= 2;
    }

  return result;
}


//
// Created by bkg2k on 10/11/2019.
//
#pragma once

#include <deque>

#include <utils/os/fs/Path.h>

class ThemeException : public std::exception
{
  private:
    String msg;

  public:
    const char* what() const noexcept override { return msg.c_str(); }

    ThemeException() = default;
    explicit ThemeException(const String& s) { msg = s; }
    ThemeException(const String& s, const std::deque<Path>& p) { msg = AddFiles(p) + s; }

    static String AddFiles(const std::deque<Path>& deque)
    {
      String result;
      result = "from theme \"" + deque.front().ToString() + "\"\n";
      for (auto it = deque.begin() + 1; it != deque.end(); it++)
        result += "  (from included file \"" + (*it).ToString() + "\")\n";
      result += "    ";
      return result;
    }
};


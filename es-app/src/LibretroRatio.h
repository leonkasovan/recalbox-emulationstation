//
// Created by matthieu on 03/04/16.
//
#pragma once

#include <map>
#include <utils/String.h>

class LibretroRatio
{
  public :
    static const std::map<String, String>& GetRatio();
};

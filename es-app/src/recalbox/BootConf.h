//
// Created by digitalLumberjack on 10/03/23.
//
#pragma once

#include "utils/IniFile.h"
#include "utils/cplusplus/StaticLifeCycleControler.h"
#include "hardware/Case.h"
#include "hardware/RotationType.h"

class BootConf : public IniFile
               , public StaticLifeCycleControler<BootConf>
{
  public:
    BootConf();

    String GetCase();
    BootConf& SetCase(const String& caze);
    RotationType GetRotation();
    BootConf& SetRotation(RotationType rotation);
};

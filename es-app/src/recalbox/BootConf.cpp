//
// Created by digitalLumberjack on 10/03/23.
//

#include "BootConf.h"

BootConf::BootConf()
    : IniFile(Path("/boot/recalbox-boot.conf"), false, true),
      StaticLifeCycleControler<BootConf>("BootConf")
{
}

String BootConf::GetCase()
{
  String currentCase = AsString("case");
  String::List splitted = currentCase.Split(':');
  if (!splitted.empty())
    return splitted[0];
  return "none";
}

BootConf& BootConf::SetCase(const String& caze)
{
  SetString("case", caze);
  return *this;
}

RotationType BootConf::GetRotation()
{
  return (RotationType)AsInt("screen.rotation",0);
}

BootConf& BootConf::SetRotation(RotationType rotation)
{
  SetInt("screen.rotation", (int)rotation);
  return *this;
}
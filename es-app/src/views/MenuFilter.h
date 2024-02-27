//
// Created by digi on 10/13/23.
//

#pragma once

class MenuFilter
{
  public:
    enum Menu
    {
      Main,
      Exit,
      Search,
      GamelistOptions
    };

    static bool ShouldDisplayMenu(const enum Menu menu)
    { (void)menu; return RecalboxConf::Instance().GetMenuType() != RecalboxConf::Menu::None; }

    enum MenuEntry
    {
      HDMode,
      Widescreen,
      PiEeprom,
    };

    static bool ShouldDisplayMenuEntry(const enum MenuEntry menuEntry)
    {
      switch (menuEntry)
      {
        case HDMode:
          return Board::Instance().GetBoardType() == BoardType::PCx64 ||
                 Board::Instance().GetBoardType() == BoardType::Pi5;
        case Widescreen:
          return Board::Instance().GetBoardType() == BoardType::PCx64 ||
                 Board::Instance().GetBoardType() == BoardType::Pi5 ||
                 Board::Instance().GetBoardType() == BoardType::Pi4 ||
                 Board::Instance().GetBoardType() == BoardType::Pi400;
        case PiEeprom:
          return Board::Instance().GetBoardType() == BoardType::Pi4 ||
                 Board::Instance().GetBoardType() == BoardType::Pi400 ||
                 Board::Instance().GetBoardType() == BoardType::Pi5;
        default: return false;
      }
    }
};

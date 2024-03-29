//
// Created by bkg2k on 02/11/2020.
//
#pragma once

//! Board model/generation
enum class BoardType
{
  // Undetected
  UndetectedYet, // Not yet detected
  Unknown,       // Unknown hardware
  // RaspberryPi
  Pi0,           // Pi 0, 0W
  Pi02,          // Pi0 II
  Pi1,           // Pi 1, A, B, A+, B+
  Pi2,           // Pi 2B
  Pi3,           // Pi 3B
  Pi3plus,       // Pi 3B+
  Pi4,           // Pi 4B
  Pi400,         // Pi 400
  Pi5,          // Pi 5B
  UnknownPi,     // Unknown Pi with higher revisions
  // Odroid
  OdroidAdvanceGo,      // Odroid advance go 1 & 2
  OdroidAdvanceGoSuper, // Odroid advance go 3 (super)
  // Anbernic
  RG351V,
  RG351P,
  RG353P,
  RG353V,
  RG353M,
  RG503,
  // PC
  PCx86,
  PCx64,
};

class BoardTypeUtil
{
  public:
    static bool IsRaspberryPi(BoardType board)
    {
      return board == BoardType::Pi0 || board == BoardType::Pi02 || board == BoardType::Pi3plus ||
             board == BoardType::Pi3 || board == BoardType::Pi4 || board == BoardType::Pi400 ||
             board == BoardType::Pi5;
    }
};


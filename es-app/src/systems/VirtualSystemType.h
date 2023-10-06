//
// Created by bkg2k on 25/06/23.
//
#pragma once

//! Virtual system types
enum class VirtualSystemType
{
  None,                //!< Not a virtual system
  Ports,               //!< Ports
  Favorites,           //!< Favorite games
  LastPlayed,          //!< Last played games, fixed order
  Multiplayers,        //!< From 2+ players
  AllGames,            //!< All games metasystem
  Lightgun,            //!< Lightgun compatible games
  Genre,               //!< Virtual systèm per genre
  Arcade,              //!< Arcade global system
  Tate,                //!< Tate system
  ArcadeManufacturers, //!< Arcade manufacturers (or publishers)
};
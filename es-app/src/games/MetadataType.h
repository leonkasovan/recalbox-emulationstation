//
// Created by bkg2k on 21/06/23.
//
#pragma once

#include <utils/cplusplus/Bitflags.h>

enum class MetadataType
{
  // Zero'ed
  None        = 0x00000000, //!< Zero'ed bitflag
  // Path - Not editable
  Path        = 0x00000001, //!< Path
  // Texts
  Name        = 0x00000002, //!< Game name
  Synopsis    = 0x00000004, //!< Synopsis text
  Publisher   = 0x00000008, //!< Publisher
  Developer   = 0x00000010, //!< Developer
  Players     = 0x00000020, //!< Players
  ReleaseDate = 0x00000040, //!< Epoch of release date
  P2K         = 0x00000080, //!< P2k file & path
  Crc32       = 0x00000100, //!< CRC32
  Region      = 0x00000200, //!< Game regions (comma separated)
  Rating      = 0x00000400, //!< Rating (float 0.0 - 1.0)
  Genre       = 0x00000800, //!< Text genre
  GenreId     = 0x00001000, //!< Normalized genres
  // Media
  Image       = 0x00002000, //!< Main image
  Thumbnail   = 0x00004000, //!< Secondary image (thumbnail)
  Video       = 0x00008000, //!< Video snap
  Manual      = 0x00010000, //!< Manuel pdf/txt
  Maps        = 0x00020000, //!< Game maps (images)
  Marquee     = 0x00040000, //!< Marquee
  Wheels      = 0x00080000, //!< Logo
  // Extra data
  LastPlayed  = 0x00100000, //!< Last played date
  PlayCount   = 0x00200000, //!< Play counter
  Favorite    = 0x00400000, //!< Game is favorite
  Rotation    = 0x00800000, //!< Rotation data for TATE or special games
  Hidden      = 0x01000000, //!< Game is hidden
  Adult       = 0x02000000, //!< Adult state
  // Hooked data
  Emulator    = 0x04000000, //!< Emulator
  Core        = 0x08000000, //!< Core
  Ratio       = 0x10000000, //!< Screen ratio
  LastPatch   = 0x20000000, //!< Last selected patch
  TimePlayed       = 0x40000000, //!< Time played in seconds
};

DEFINE_BITFLAG_ENUM(MetadataType, int)
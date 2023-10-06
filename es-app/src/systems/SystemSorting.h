//
// Created by bkg2k on 16/01/2022.
//
// As part of the RECALBOX Project
// http://www.recalbox.com
//
#pragma once

class SystemData;

enum class SystemSorting
{
  Default,                                    //!< Default sorting as defined in the xml file
  Name,                                       //!< Sort by system name (not including manufacturer)
  ReleaseDate,                                //!< Sort by release date of the original hardware
  SystemTypeThenName,                         //!< Sort by system type (console, computer, ...), then by name
  SystemTypeThenReleaseDate,                  //!< Sort by system type (console, computer, ...), then by release data
  ManufacturerThenName,                       //!< Sort by manufacturer name, then system name
  ManufacturerThenReleaseData,                //!< Sort by manufacturer name, then system name
  SystemTypeThenManufacturerThenName,         //! Sort by type, then manufacturer, then name
  SystemTypeThenManufacturerThenReleasdeDate, //! Sort by type, then manufacturer, then release date
};

//! Sort by name only
int SortingName(SystemData* const& a, SystemData* const& b);

//! Sort by release date only
int SortingReleaseDate(SystemData* const& a, SystemData* const& b);

//! Sort by type then by name
int Sorting1Type2Name(SystemData* const& a, SystemData* const& b);

//! Sort by type then by release date
int Sorting1Type2ReleaseDate(SystemData* const& a, SystemData* const& b);

//! Sort by manufacturer then by name
int Sorting1Manufacturer2Name(SystemData* const& a, SystemData* const& b);

//! Sort by manufacturer then by release date
int Sorting1Manufacturer2ReleaseDate(SystemData* const& a, SystemData* const& b);

//! Sort by type then by manufacturer then by name
int Sorting1Type2Manufacturer3Name(SystemData* const& a, SystemData* const& b);

//! Sort by type then by manufacturer then by release date
int Sorting1Type2Manufacturer3ReleaseDate(SystemData* const& a, SystemData* const& b);


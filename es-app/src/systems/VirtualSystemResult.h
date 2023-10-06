//
// Created by bkg2k on 25/06/23.
//
#pragma once

// Forward déclaration
class SystemData;

/*!
 * @brief Resulting object from the multi-threaded virtual system builder
 */
struct VirtualSystemResult
{
  SystemData* mNewSystem; //!< Newly created system
  int mIndex;             //!< Index (position) in the virtual system list
};

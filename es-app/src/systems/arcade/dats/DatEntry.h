//
// Created by bkg2k on 17/06/23.
//

#pragma once

#include "utils/String.h"
#include "utils/hash/Md5.h"
#include <systems/arcade/dats/RomFileHolder.h>

class DatEntry
{
  public:
    //! Empty constructor
    DatEntry() : mParent(nullptr) {}

    /*!
     * @brief Deserialize the given line into the given DatEntry
     * @param line Text line to deserialize
     * @param to Target entry to fill with deserialized data
     * @param rom output rom name
     * @param parent output parent name
     * @return True if the deserialization id successful, false if error occurred
    */
    static bool Deserialize(const String& line, [[out]] DatEntry& to, [[out]] String& rom, [[out]] String& parent);

    //! Has parent?
    [[nodiscard]] bool HasParent() const { return mParent != nullptr; }

    //! Get parent
    [[nodiscard]] const DatEntry* Parent() const { return mParent; }

    //! Get name
    [[nodiscard]] const String& Name() const { return mFileName; }

    //! Get rom count
    [[nodiscard]] int RomCount() const { return (int)mRomList.size(); }

    [[nodiscard]] const RomFileHolder& Rom(int index) const
    {
      if ((unsigned int)index < mRomList.size()) return mRomList[index];
      static RomFileHolder __nullrom(String::Empty, 0);
      return __nullrom;
    }

  private:
    // Allow DatContent to set parent to keep the method private
    friend class DatContent;

    //! Rom list type
    typedef std::vector<RomFileHolder> RomList;

    String mFileName;  //!< Game filename w/o extension
    RomList mRomList;  //!< Rom list
    DatEntry* mParent; //!< Parent reference

    /*!
     * @brief Add rom to the internal list
     * @param rom Rom file
     * @param hash Stringized hash
     */
    void AddRom(const String& rom, const String& hash);

    /*!
     * @brief Set parent reference
     * @param parent Parent game
     */
    void SetParent(DatEntry* parent) { mParent = parent; }
};

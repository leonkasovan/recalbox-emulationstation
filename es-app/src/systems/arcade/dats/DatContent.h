//
// Created by bkg2k on 17/06/23.
//
#pragma once

#include <utils/os/fs/Path.h>
#include <utils/storage/HashMap.h>
#include "DatEntry.h"
#include "games/FileData.h"
#include "ScanResult.h"
#include "utils/Zip.h"
#include "utils/storage/Set.h"

class DatContent
{
  public:
    /*!
     * @brief Load the given flat database
     * @param flatDatabasdePath flat database path (*.fdt)
     */
    explicit DatContent(const Path& flatDatabasdePath);

    /*!
     * @brief Check if the dat is valid and contains dat entries
     * @return True if the dat is valid, false otherwise
     */
    [[nodiscard]] bool IsValid() const { return !mEntryList.empty(); }

    /*!
     * @brief Lookup DatEntry for the given game, then scan file content and return an emulatiopn status
     * @param game Game to scan
     * @param failures Detail of failures
     * @return Bool if the gam eis suported, false otherwise
     */
    bool Scan(const FileData& game, ScanResult::Result::RomFailList& failures);

  private:
    //! Entry list type
    typedef std::vector<DatEntry> DatEntryList;
    //! Entry map type
    typedef HashMap<String, DatEntry*> DatEntryMap;

    //! Entry list
    DatEntryList mEntryList;
    //! Entry map
    DatEntryMap mEntryMap;

    /*!
     * @brief Load the given database
     * @param flatDatabasdePath flat database path (*.fdt)
     */
    void Load(const Path& flatDatabasdePath);

    /*!
     * @brief Lookup the DatEntry that match the filename of the given game
     * @param game Game to retrieve a DatEntry from
     * @return DatEntry reference or nullptr if there is no match
     */
    const DatEntry* Lookup(const FileData& game);

    /*!
     * @brief Get the MD5 from the given file
     * @param file File to hash
     * @param md5 MD5 result
     * @return True if the result is ok, false if an error occurred
     */
    static bool Md5File(const Path& file, [[out]] MD5::DigestMd5& md5);

    /*!
     * @brief Look for the given rom into the given zip file
     * It looks for a file name match and a crc32 match
     * @param zip Zipped game
     * @param rom Rom
     * @return True if the rom is found, false otherwize
     */
    static bool LookForRom(const Zip& zip, const RomFileHolder& rom, [[out]] bool& filefound, [[out]] unsigned int& crc32);

    /*!
     * @brief Add unknwon files from zip and chd folder
     * @param romPath Zip path
     * @param zippedGame Zipped game
     * @param processedFiles Already processed files, both rom & chd
     * @param failures Failure list where unknown files are added
     */
    void AddUnknownFile(const Path& romPath, const Zip& zippedGame, const HashSet<String>& processedFiles, ScanResult::Result::RomFailList& failures);
};

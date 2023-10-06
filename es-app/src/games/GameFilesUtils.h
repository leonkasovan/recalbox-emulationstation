//
// Created by gugue_u on 12/05/2021.
//


#include "FileData.h"
#include "SaveState.h"
#include <utils/storage/Set.h>
#include <views/gamelist/ISimpleGameListView.h>

class GameFilesUtils
{
  public:
    static HashSet<String> GetGameSubFiles(FileData& game);
    static HashSet<String> GetGameExtraFiles(FileData& fileData);
    static bool HasAutoPatch(const FileData* fileData);
    static Path GetSubDirPriorityPatch(const FileData* fileData);
    static std::vector<Path> GetSoftPatches(const FileData* fileData);
    static std::vector<SaveState> GetGameSaveStateFiles(FileData& game);
    static HashSet<String> GetGameSaveFiles(FileData& game);
    static HashSet<String> GetMediaFiles(FileData& fileData);

    static bool ContainsMultiDiskFile(const String& extensions)
    {
      return extensions.Contains(".m3u") || extensions.Contains(".cue") ||
             extensions.Contains(".ccd") || extensions.Contains(".gdi");
    }

    static void ExtractUselessFiles(const Path& path, HashSet<String>& list);
    static void ExtractUselessFilesFromCue(const Path& path, HashSet<String>& list);
    static void ExtractUselessFilesFromCcd(const Path& path, HashSet<String>& list);
    static void ExtractUselessFilesFromM3u(const Path& path, HashSet<String>& list);
    static void ExtractUselessFilesFromGdi(const Path& path, HashSet<String>& list);
    static String ExtractFileNameFromLine(const String& line);
    static void AddIfExist(const Path& path, HashSet<String>& list);
    static constexpr int sMaxGdiFileSize = (10 << 10); // 10 Kb

    static void DeleteSelectedFiles(FileData& fileData, HashSet<String>&, HashSet<String>&);
    static void DeleteAllFiles(FileData& fileData);

    static bool IsMediaShared(FileData& system, const Path& mediaPath);

    static void DeleteFoldersRecIfEmpty(FolderData* folderData);

    /*!
     * @brief Remove any character block between () or [], including enclosing characters
     * @param str String to process
     * @return Result string
     */
    static String RemoveParenthesis(const String& str);
};



#pragma once

#include <vector>
#include "utils/String.h"

class Path
{
  private:
    //! Unix separator (char)
    static constexpr char sSeparator = '/';
    //! Unis seperator (string)
    static constexpr const char* sSeparatorString = "/";
    //! Double seperator
    static constexpr const char* sDoubleSeparatorString = "//";
    //! Extension separator (char)
    static constexpr char sExtensionSeparator = '.';

    //! Single dot in path
    static constexpr const char* sSingleDotPath = "/./";
    //! Extension separator (char)
    static constexpr char sDotDirectory = '.';

    //! Path
    String mPath;

    /*!
     * @brief Normalize the path string: Remove tailing separator and remove double separators
     */
    void Normalize();

    /*!
     * @brief Resolve symbolic link
     * @param source path to resolve
     * @return resolved path
     */
    static String ResolveSymbolicLink(const char* source);

  public:
    typedef std::vector<Path> PathList;

    //! Empty path
    static const Path Empty;

    /*
     * Constructors
     */

    /*!
     * @brief Default constructor to initialize an empty path
     */
    Path() = default;

    /*!
     * @brief Build a path from a string
     * @param path string path
     */
    explicit Path(const String& path)
      : mPath(path)
    {
      Normalize();
    }

    /*!
     * @brief Build a path from a char*
     * @param path string path
     */
    explicit Path(const char* path)
      : mPath(path)
    {
      Normalize();
    }

    /*!
     * @brief Build a path from a char* and a length
     * @param path string path
     * @param length string length
     */
    explicit Path(const char* path, int length)
      : mPath(path, length)
    {
      Normalize();
    }

    /*!
     * @brief Move constructor from string
     * @param path source path
     */
    explicit Path(String&& path)
      : mPath(std::move(path))
    {
      Normalize();
    }

    /*!
     * @brief Copy constructor from another path
     * @param path source path
     */
    Path(const Path& path) = default;

    /*!
     * @brief Move constructor from another path
     * @param path source path
     */
    Path(Path&& path) noexcept
      : mPath(std::move(path.mPath))
    {
    }

    /*
     * Converters
     */

    /*!
     * @brief Return current path string
     * @return path string
     */
    [[nodiscard]] const String& ToString() const { return mPath; }

    /*!
     * @brief Return current path string
     * @return path string
     */
    [[nodiscard]] const char* ToChars() const { return mPath.c_str(); }

    /*
     * Path manipulations
     */

    /*!
     * @brief Check whether the path is empty or not
     * @return True if the path is empty
     */
    [[nodiscard]] bool IsEmpty() const { return mPath.empty(); }

    /*!
     * @brief Return the component count of the path
     * @return component count
     */
    [[nodiscard]] int ItemCount() const;

    /*!
     * @brief Return the component at index n of the current path.
     * Ff the indexd is out of bounds, an empty string is returned
     * @param index index of the path component to extract
     * @return path component or empty string
     */
    [[nodiscard]] String Item(int index) const;

    /*!
     * @brief Return path from the first component up to the component at index n of the current path.
     * Ff the index is out of bounds, the whole path is returned
     * @param index index of the last path component to extract
     * @return path component or empty string
     */
    [[nodiscard]] String UptoItem(int index) const;

    /*!
     * @brief Return path from the index component up to the last component of the current path.
     * If the index is out of bounds, an empty path is returned
     * @param index index of the last path component to extract
     * @return path component or empty string
     */
    [[nodiscard]] String FromItem(int index) const;

    /*!
     * @brief Return the directory part of the current path. /path/to/file => /path/to
     * @return Directory
     */
    [[nodiscard]] Path Directory() const;

    /*!
     * @brief Return the filename: the last component of the path. /path/to/file.ext => file.ext
     * @return filename
     */

    [[nodiscard]] String Filename() const;

    /*!
     * @brief Return the filename less the extension if any. /path/to/file.ext => file
     * @return Filename without extension
     */
    [[nodiscard]] String FilenameWithoutExtension() const;

    /*!
     * @brief Return the file extension if any. /path/to/file.ext => .ext
     * @return file extension inclusing the leading dot
     */
    [[nodiscard]] String Extension() const;

    /*!
     * @brief Replace or add current extension by the given one
     * @param newext New extension
     * @return New path
     */
    [[nodiscard]] Path ChangeExtension(const String& newext) const;

    /*
     * Real filesystem
     */

    /*!
     * @brief Check if the path exist on the filesystem
     * @return True if the path exists
     */
    [[nodiscard]] bool Exists() const;

    /*!
     * @brief Check if the current path is a file
     * @return True if the path is a file
     */
    [[nodiscard]] bool IsFile() const;

    /*!
     * @brief Check if the current path is a symbolic link
     * @return True if the path is a symbolic link
     */
    [[nodiscard]] bool IsSymLink() const;

    /*!
     * @brief Check if the current path is a directory
     * @return True if the path is a directory
     */
    [[nodiscard]] bool IsDirectory() const;

    /*!
     * @brief Check if the current path is hidden
     * @return True if the path is hidden
     */
    [[nodiscard]] bool IsHidden() const;

    /*!
     * @brief Try to create the whole path, from the very first ancestor up to the latest child
     * @return True if the final path exists
     */
    [[nodiscard]] bool CreatePath() const;

    /*!
     * @brief Delete the path
     * @return True if the path has been deletec
     */
    [[nodiscard]] bool Delete() const;

    /*!
     * @brief Get size of the file
     * @return Size in bytes
     */
    [[nodiscard]] long long Size() const;

    /*
     * Operator overloading
     */

    /*!
     * @brief Add a new component to the current path and return the rersulting path
     * @param path path to add
     * @return new combined path
     */
    Path operator / (const char* path) const;

    /*!
     * @brief Add a new component to the current path and return the rersulting path
     * @param path path to add
     * @return new combined path
     */
    Path operator / (const String& path) const;

    /*!
     * @brief Add a new component to the current path and return the rersulting path
     * @param path path to add
     * @return new combined path
     */
    Path operator / (const Path& path) const;

    /*!
     * @brief Path comparison
     * @param to path to compare to
     * @return True if the current pass is lower than the 'to'
     */
    bool operator < (const Path& to) const { return mPath < to.mPath; }

    /*!
     * @brief Path comparison
     * @param to path to compare to
     * @return True if the current pass is lower than the 'to'
     */
    bool operator < (const String& to) const { return mPath < to; }

    /*!
     * @brief Path comparison
     * @param to path to compare to
     * @return True if the current pass is lower than the 'to'
     */
    bool operator < (const char* to) const { return mPath < to; }

    /*!
     * @brief Path comparison
     * @param to path to compare to
     * @return True if the current pass is higher than the 'to'
     */
    bool operator > (const Path& to) const { return mPath > to.mPath; }

    /*!
     * @brief Path comparison
     * @param to path to compare to
     * @return True if the current pass is higher than the 'to'
     */
    bool operator > (const String& to) const { return mPath > to; }

    /*!
     * @brief Path comparison
     * @param to path to compare to
     * @return True if the current pass is higher than the 'to'
     */
    bool operator > (const char* to) const { return mPath > to; }

    /*!
     * @brief Path comparison
     * @param to path to compare to
     * @return True if the current pass is higher than the 'to'
     */
    bool operator == (const Path& to) const { return mPath == to.mPath; }

    /*!
     * @brief Path comparison
     * @param to path to compare to
     * @return True if the current pass is higher than the 'to'
     */
    bool operator == (const String& to) const { return mPath == to; }

    /*!
     * @brief Path comparison
     * @param to path to compare to
     * @return True if the current pass is higher than the 'to'
     */
    bool operator == (const char* to) const { return mPath == to; }

    /*!
     * @brief Path comparison
     * @param to path to compare to
     * @return True if the current pass is higher than the 'to'
     */
    bool operator != (const Path& to) const { return mPath != to.mPath; }

    /*!
     * @brief Path comparison
     * @param to path to compare to
     * @return True if the current pass is higher than the 'to'
     */
    bool operator != (const String& to) const { return mPath != to; }

    /*!
     * @brief Path comparison
     * @param to path to compare to
     * @return True if the current pass is higher than the 'to'
     */
    bool operator != (const char* to) const { return mPath != to; }

    /*!
     * @brief Default copy assignator
     * @param src source path
     * @return destination path
     */
    Path& operator = (const Path& src) = default;

    /*!
     * @brief Default move assignator
     * @param src source path
     * @return destination path
     */
    Path& operator = (Path&& src) = default;

    /*!
     * @brief copy assignator
     * @param src source path
     * @return destination path
     */
    Path& operator = (const char* src) { mPath = src; Normalize(); return *this; };

    /*!
     * @brief copy assignator
     * @param src source path
     * @return destination path
     */
    Path& operator = (const String& src) { mPath = src; Normalize(); return *this; };

    /*
     * Path convertions
     */

    /*!
     * @brief Check if the path is an absolute path
     * @return True if the path is absolute
     */
    [[nodiscard]] bool IsAbsolute() const { return ((!mPath.empty()) && (mPath[0] == '/')); }

    /*!
     * @brief Convert the current path to its absolute representation into a new path object
     * @return Absolute path
     */
    [[nodiscard]] Path ToAbsolute() const;

    /*!
     * @brief Convert the current path to its absolute representation into a new path object
     * if the current path is relative, use the given base as parent path
     * @return Absolute path
     */
    [[nodiscard]] Path ToAbsolute(const Path& base) const;

    /*!
     * @brief Convert the current path into its canonical form
     * @return
     */
    [[nodiscard]] Path ToCanonical() const;

    /*!
     * @brief Try to remove the parent path from the current path
     * if and only if the current path starts with the parent path
     * @param to Parent path to remove
     * @param on Once the method exists, this parameter contains true if the parent path has been actually removed
     * @return Relative path to the parent path (or unchanged source path)
     */
    Path MakeRelative(const Path& to, bool& ok) const;

    /*!
     * @brief Check if the current path starts with the given path
     * @param path path to check
     * @return True if the current path starts with the given path
     */
    [[nodiscard]] bool StartWidth(const Path& path) const;

    /*!
     * @brief Check if the current path starts with the given path
     * @param path path to check
     * @return True if the current path starts with the given path
     */
    [[nodiscard]] bool StartWidth(const String& path) const { return StartWidth(Path(path)); }

    /*!
     * @brief Get an escaped string representation of the current path
     * @return bach compatible escaped path
     */
    [[nodiscard]] String MakeEscaped() const;

    /*
     * Static helpers
     */

    /*!
     * @brief Get home directory
     * @return home directory or cwd if the home does not exist
     */
    static Path Home();

    /*!
     * @brief Get current working directory
     * @return Current working directory ot empty path of the operation fails
     */
    static Path Cwd();

    /*!
     * @brief Get root path
     * @return Always return /
     */
    static Path Root() { return Path("/"); }

    /*
     * Folder content
     */

    /*!
     * @brief Get file/directory list from the current directory path
     * @param storeFolders True to store folders in the list. False to get files only
     * @return file and directory list, or empty list if the current path is invalid or not a directory
     */
    [[nodiscard]] PathList GetDirectoryContent(bool storeFolders = true) const;

    /*!
     * @brief Get file/directory list recursively from the current directory path
     * @param storeFolders True to store folders in the list. False to get files only
     * @return file and directory list, or empty list if the current path is invalid or not a directory
     */
    [[nodiscard]] PathList RecurseDirectoryContent(bool storeFolders = true) const;

    /*!
     * @brief Check if the current folder contains at least a file
     * @return True if it contains at least a file, false otherwise
     */
    bool ContainsFile();

    /*!
     * @brief Check if the current folder contains at least a folder
     * @return True if it contains at least a folder, false otherwise
     */
    bool ContainsFolders();

    /*!
     * @brief Check if the current folder contains at least a file or a folder
     * @return True if it contains at least a file or a folder, false otherwise
     */
    bool ContainsFileOrFolders();

    /*!
     * @brief Rename a from "from" to "to"
     * @param from Source path
     * @param to Destination path
     * @return True if the renaming/move is successful
     */
    static bool Rename(const Path& from, const Path& to);
};

#include <hash_fun.h>
/*!
 * @brief Specialized template for hashing String objects
 */
template<> struct std::hash<Path>
{
  size_t operator()(const Path& __val) const noexcept { return __val.ToString().Hash(); }
};

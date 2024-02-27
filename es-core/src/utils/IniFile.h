//
// Last modification by Maksthorr on 28/04/2023
//
#pragma once

#include <utils/os/fs/Path.h>
#include <utils/storage/HashMap.h>
#include <utils/storage/Set.h>
#include "utils/os/system/Mutex.h"

class IniFile
{
  public:
    /*!
     * @brief Constructor
     * @param confpath File to load
     * @param extraSpace if True, add extra space around separator
     * @param autoBackup automatically manager backup file of the confpath
     */
    explicit IniFile(const Path& confpath, bool extraSpace, bool autoBackup);

    /*!
     * @brief Constructor
     * @param confpath File to load
     * @param fallbackpath File to load if confpath has not been loaded
     * @param extraSpace if True, add extra space around separator
     * @param autoBackup automatically manager backup file of the confpath
     */
    explicit IniFile(const Path& confpath, const Path& fallbackpath, bool extraSpace, bool autoBackup);

    //! Destructor
    virtual ~IniFile()
    {
      Save();
    }

    /*!
     * @brief Save the configuration file and backup the current one
     * @return True if the operation is successful
     */
    bool Save();

    /*!
     * @brief Cancel all pending changes
     */
    void Cancel()
    {
      mPendingDelete.clear();
      mPendingWrites.clear();
    }

    /*!
     * @brief Delete (comment) key
     * @param name Key name
     */
    void Delete(const String &name);

    /*!
     * @brief Check if a key is defined in this configuration
     * @param name key to check
     * @return True if the key is defined
     */
    [[nodiscard]] bool IsDefined(const String& name) const
    {
      return !mPendingDelete.contains(name) && (mConfiguration.contains(name) || mPendingWrites.contains(name));
    }

    /*!
     * @brief Get string value from the given key
     * @param name Key
     * @return Value or empty string if the key does not exist
     */
    [[nodiscard]] String AsString(const String& name) const;

    /*!
     * @brief Get string value from the given key or return the default value
     * @param name Key
     * @param defaultValue Default value
     * @return Value or default value if the key does not exist
     */
    [[nodiscard]] String AsString(const String &name, const String &defaultValue) const;

    /*!
     * @brief Get string value from the given key
     * @param name Key
     * @return Value or empty string if the key does not exist
     */
    [[nodiscard]] String AsString(const char* name) const { return AsString(String(name)); };

    /*!
     * @brief Get string value from the given key or return the default value
     * @param name Key
     * @param defaultValue Default value
     * @return Value or default value if the key does not exist
     */
    [[nodiscard]] String AsString(const char* name, const char* defaultValue) const { return AsString(String(name), defaultValue); }

    /*!
     * @brief Get value from the given key in List format
     * @param name Key
     * @return Value
     */
    [[nodiscard]] [[nodiscard]] String::List AsStringList(const String& name) const { return AsString(String(name)).Split(','); }

    /*!
     * @brief Get boolean value from the given key or return the default value
     * @param name Key
     * @param defaultValue Default value (optional, false by default)
     * @return Value or default value if the key does not exist
     */
    [[nodiscard]] bool AsBool(const String& name, bool defaultValue = false) const;

    /*!
     * @brief Get boolean value from the given key or return the default value
     * @param name Key
     * @param defaultValue Default value (optional, false by default)
     * @return Value or default value if the key does not exist
     */
    [[nodiscard]] bool AsBool(const char* name, bool defaultValue = false) const { return AsBool(String(name), defaultValue); }

    /*!
     * @brief Get value as unsigned int from the given key or return the default value
     * @param name Key
     * @param defaultValue Default value (optional, 0 by default)
     * @return Value or default value if the key does not exist
     */
    [[nodiscard]] unsigned int AsUInt(const String& name, unsigned int defaultValue = 0) const;

    /*!
     * @brief Get value as signed int from the given key or return the default value
     * @param name Key
     * @param defaultValue Default value (optional, 0 by default)
     * @return Value or default value if the key does not exist
     */
    [[nodiscard]] int AsInt(const String& name, int defaultValue = 0) const;

    /*!
     * @brief Get value as signed int from the given key or return the default value
     * @param name Key
     * @param defaultValue Default value (optional, 0 by default)
     * @return Value or default value if the key does not exist
     */
    [[nodiscard]] int AsInt(const char* name, int defaultValue = 0) const { return AsInt(String(name), defaultValue); }

    /*!
     * @brief Set the value as string of the given key
     * @param name Key
     * @param value Value to set
     */
    void SetString(const String &name, const String &value);

    /*!
     * @brief Set the value as boolean of the given key
     * @param name Key
     * @param value Value to set
     */
    void SetBool(const String &name, bool value);

    /*!
     * @brief Set the value as an unsigned int of the given key
     * @param name Key
     * @param value Value to set
     */
    void SetUInt(const String &name, unsigned int value);

    /*!
     * @brief Set the value as a signed int of the given key
     * @param name Key
     * @param value Value to set
     */
    void SetInt(const String &name, int value);

    /*!
     * @brief Set the value as a string list, comma separated, of the given key
     * @param name Key
     * @param values string list
     */
    void SetList(const String &name, const String::List &values);

    /*!
     * @brief Check if the given key exists in the configuration file
     * @param name Key to check
     * @return True if the key exists, false otherwise
     */
    [[nodiscard]] bool Exists(const String& name) const { return mConfiguration.contains(name); }

    /*!
     * @brief Check if a value is in the given named list
     * @param name Key from which to obtain the list
     * @param value Value to seek for in the list
     * @return True if the list exists and the value is found. False otherwise
     */
    [[nodiscard]] bool isInList(const String &name, const String &value) const;

    /*!
     * @brief Check if there is at least one key starting with the given string
     * @param startWidth String
     * @return True if at least one key starts with the given string
     */
    [[nodiscard]] bool HasKeyStartingWith(const String& startWidth) const;

    /*!
     * @brief Check if the given key exists
     * @param key Key name
     * @return True if the jey exists
     */
    [[nodiscard]] bool HasKey(const String& key) const;

    /*!
     * @brief Get all keys ending with the given string
     * @param startWidth String
     * @return Key list
     */
    String::List GetKeyEndingWith(const String& startWidth);

    /*!
     * @brief Check if the given line is a valide 'key=value'
     * @param line Text line to seek for key/value
     * @param key Extracted key if found
     * @param value Extracted value if found
     * @param isCommented Set tu true if the key/value is commented using ';', false otherwise
     * @return true if a valid key=value has been found
     */
    static bool IsValidKeyValue(const String& line, String& key, String& value, bool& isCommented);

    /*!
     * @brief Check if this instance has loaded a file and has keys and values
     * @return True if at least one key/value pair has been loaded
     */
    [[nodiscard]] bool IsValid() const { return mValid; }

    /*!
     * @brief Called after loading the file
     */
    virtual void OnLoad() {}

    /*!
     * @brief Called after saving the file
     */
    virtual void OnSave() const {}

    /*!
     * @brief Clear configuration and reset everything with fallback
     */
    bool ResetWithFallback();

  private:
    //! Save guardian
    Mutex mLocker;
    //! Configuration map: key, value - Read from file
    HashMap<String, String> mConfiguration;
    //! Configuration map: key, value - Pending writes
    HashMap<String, String> mPendingWrites;
    //! Configuration set: key - Pending deleted (commented)
    HashSet<String> mPendingDelete;
    //! File path
    Path mFilePath;
    //! Fallback File path
    Path mFallbackFilePath;
    //! Extra spaces
    bool mExtraSpace;
    //! Automatic backup
    bool mAutoBackup;
    //! This object is valid and has keys/values
    bool mValid;

    /*!
     * @brief Load configuration file
     * @return True if a configuration file has been loaded successfully
     */
    bool Load();

    /*!
     * @brief Load content into the given string
     * @param content Content string
     * @return True if loading is ok, false otherwise
     */
    bool LoadContent([[out]] String& content);

    /*!
     * @brief Extract the value from the given key
     * @param key Key
     * @return Value or empty string if the key does not exists
     */
    [[nodiscard]] const String& ExtractValue(const String& key) const;
};


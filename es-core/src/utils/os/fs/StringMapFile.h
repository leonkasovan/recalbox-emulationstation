#pragma once

#include <utils/String.h>
#include <utils/storage/HashMap.h>

class StringMapFile
{
  private:
    //! Path of map file
    String mPath;
    //! Map of simple key/values
    HashMap<String, String> mMap;

  public:
    /*!
     * @brief Constructor
     * @param path target file path
     */
    explicit StringMapFile(const String& path)
      : mPath(path),
        mMap()
    {
    }

    /*!
     * @brief Load file's keys and values into our internal map
     */
    void Load();

    /*!
     * @brief Save internal map contents into file as 'key=value'
     */
    void Save();

    /*!
     * @brief Lookup a key into ou map and return the appropriate value
     * @param key Key to lookup
     * @param defaultvalue default value if the key does not exist in the map
     * @return Value associated to the key, or default value if the key does not exists
     */
    String GetString(const String& key, const String& defaultvalue);

    /*!
     * @brief Lookup a key into ou map and return the appropriate value
     * @param key Key to lookup
     * @param defaultvalue default value if the key does not exist in the map
     * @return Value associated to the key, or default value if the key does not exists
     */
    int GetInt(const String& key, int defaultvalue);

    /*!
     * @brief Lookup a key into ou map and return the appropriate value
     * @param key Key to lookup
     * @param defaultvalue default value if the key does not exist in the map
     * @return Value associated to the key, or default value if the key does not exists
     */
    bool GetBool(const String& key, bool defaultvalue);

    /*!
     * @brief Set the value for the given key
     * @param key Key
     * @param value Value to set
     */
    void SetString(const String& key, const String& value);

    /*!
     * @brief Set the value for the given key
     * @param key Key
     * @param value Value to set
     */
    void SetInt(const String& key, int value);

    /*!
     * @brief Set the value for the given key
     * @param key Key
     * @param value Value to set
     */
    void SetBool(const String& key, bool value);
};


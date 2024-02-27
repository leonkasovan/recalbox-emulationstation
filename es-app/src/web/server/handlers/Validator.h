//
// Created by bkg2k on 30/04/2020.
//
#pragma once

#include <string>
#include <utils/storage/HashMap.h>
#include "utils/String.h"

class Validator
{
  public:
    //! Configuration Datatype
    enum class Types
    {
      StringFree,        //!< Free String
      StringConstrained, //!< Chars-restricted string
      StringPicker,      //!< String value among a pre-defined list
      StringMultiPicker, //!< String value among a pre-defined list
      IntRange,          //!< Numeric value among a pre-defined range
      Bool,              //!< Boolean
    };

  private:
    //! Data type
    Types mType;

    //! String/Int list
    String mList;
    //! String/Int display info
    String mDisplay;
    //! Int range lower value
    int mLower;
    //! Int range higher value
    int mHigher;

    /*!
     * @brief Make a quick join of c strings
     * @param list c-strin glist
     * @param joiner Separator
     * @return Joined string
     */
    static String JoinConstChars(const std::vector<const char*>& list, char joiner)
    {
      String result;
      for(const char* cstring : list)
      {
        if (!result.empty()) result.Append(joiner);
        result.Append(cstring);
      }
      return result;
    }

  public:
    /*!
     * @brief Default constructor: Free string
     */
    Validator()
      : mType(Types::StringFree),
        mLower(0),
        mHigher(0)
    {
    }

    /*!
     * @brief Char restricted string
     */
    explicit Validator(const char* charList)
      : mType(Types::StringConstrained),
        mList(charList),
        mLower(0),
        mHigher(0)
    {
    }

    /*!
     * @brief String list constructor
     * @param list string list
     */
    explicit Validator(bool multi, const std::vector<const char*>&  list)
      : mType(multi ? Types::StringMultiPicker : Types::StringPicker),
        mList('|' + JoinConstChars(list, '|')),
        mLower(0),
        mHigher(0)
    {
    }

    /*!
     * @brief String list constructor
     * @param list string list
     */
    Validator(const String::List&  list, bool multi)
      : mType(multi ? Types::StringMultiPicker : Types::StringPicker),
        mList('|' + String::Join(list, "|")),
        mLower(0),
        mHigher(0)
    {
    }

    /*!
     * @brief String list constructor w/ display info
     * @param list string list
     */
    explicit Validator(const HashMap<String, String>&  map, bool multi)
      : mType(multi ? Types::StringMultiPicker : Types::StringPicker)
      , mList('|')
      , mDisplay('|')
      , mLower(0)
      , mHigher(0)
    {
      for(const auto& item : map)
      {
        mList.Append(item.first).Append('|');
        mDisplay.Append(item.second).Append('|');
      }
    }

    /*!
     * @brief Int range constructor
     * @param from lower range value
     * @param to higher range value
     */
    explicit Validator(int from, int to)
      : mType(Types::IntRange),
        mLower(from),
        mHigher(to)
    {
    }

    /*!
     * @brief Bool constructor
     * @param list
     */
    explicit Validator(bool)
      : mType(Types::Bool),
        mLower(0),
        mHigher(0)
    {
    }

    /*!
     * @brief Validate the given string
     * @param value String to validate
     * @param outvalue Normalized output value
     * @return True if the string is valid, false otherwise
     */
    bool Validate(String& value) const;

    /*
     * Getters
     */

    //! Get type
    [[nodiscard]] Types Type() const { return mType; }

    //! Get type as string
    [[nodiscard]] const char* TypeAsString() const;

    //! Get validation data
    [[nodiscard]] String StringConstraint() const;
    //! Get validation data
    [[nodiscard]] String::List StringList() const;
    //! Get validation data
    [[nodiscard]] std::vector<int> IntList() const;

    //! Has display info?
    [[nodiscard]] bool HasDisplay() const { return !mDisplay.empty(); }
    //! Get display info
    [[nodiscard]] String::List DisplayList() const;

    //! Get int lower range value
    [[nodiscard]] int Lower() const { return mLower; }
    //! Get int lower range value
    [[nodiscard]] int Higher() const { return mHigher; }
};

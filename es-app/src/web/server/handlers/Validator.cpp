//
// Created by bkg2k on 30/04/2020.
// Last modification by Maksthorr on 21/04/2023
//

#include "Validator.h"
#include <utils/Log.h>

bool Validator::Validate(String& value) const
{
  switch (mType)
  {
    case Types::StringFree: return true;
    case Types::StringConstrained:
    {
      for(int i = (int)value.size(); --i >= 0; )
        if (mList.find(value[i]) == String::npos)
          return false;
      return true;
    }
    case Types::StringPicker:
    {
        // FIX : Changing searching method v2.
        size_t pos = 0;
        while ((pos = mList.find(value, pos)) != String::npos)
        {
            // Check if v not begin in the middle of a word
            if (pos != 0 && mList[pos-1] != '|')
            {
                pos++;
                continue;
            }
            // Check if v not end in the middle of a word
            if (pos + value.length() != mList.length() && mList[pos+value.length()] != '|')
            {
                ++pos;
                continue;
            }
            return true;
        }
        return false;
    }
    case Types::StringMultiPicker:
    {
      for(const String& v : value.Split(','))
      {
          // FIX : Changing searching method v2.
          bool ok = false;
          size_t pos = 0;
          while ((pos = mList.find(v, pos)) != String::npos)
          {
              // Check if v not begin in the middle of a word
              if (pos != 0 && mList[pos-1] != '|')
              {
                  pos++;
                  continue;
              }

              // Check if v not end in the middle of a word
              if (pos + v.length() != mList.length() && mList[pos+v.length()] != '|')
              {
                  ++pos;
                  continue;
              }
              ok = true;
              break;
          }
          if(!ok) return false;

      }
      return true;

    }
    case Types::IntRange:
    {
      int intValue = 0;
      if (value.TryAsInt(intValue))
        return (intValue >= mLower && intValue <= mHigher);
      return false;
    }
    case Types::Bool:
    {
      if (value.length() == 1)
        return (value[0] == '0' || value[0] == '1');
      value.LowerCase();
      if (value == "true") value = "1";
      else if (value == "false") value = "0";
      return (value[0] == '0' || value[0] == '1');
    }
    default: break;
  }

  LOG(LogError) << "Unknown type";
  return false;
}

const char* Validator::TypeAsString() const
{
  switch(mType)
  {
    case Types::StringFree: return "string";
    case Types::StringConstrained: return "constrainedString";
    case Types::StringPicker: return "stringList";
    case Types::StringMultiPicker: return "stringListMulti";
    case Types::IntRange: return "intRange";
    case Types::Bool: return "boolean";
    default: break;
  }
  return "Unknown";
}

String::List Validator::StringList() const
{
  if (mType == Types::StringPicker ||
      mType == Types::StringMultiPicker)
    return mList.ToTrim('|').Split('|');
  return String::List();
}

String::List Validator::DisplayList() const
{
  if (mType == Types::StringPicker ||
      mType == Types::StringMultiPicker)
    return mDisplay.ToTrim('|').Split('|');
  return String::List();
}

std::vector<int> Validator::IntList() const
{
  std::vector<int> result;
  if (mType == Types::StringConstrained)
  {
    result.resize((int)(mList.size() / sizeof(int)));
    int* list = (int*) mList.data();
    for (int i = (int)(mList.size() / sizeof(int)); --i >= 0;)
      result[i] = list[i];

  }
  return result;
}

String Validator::StringConstraint() const
{
  if (mType == Types::StringConstrained) return mList;
  return String();
}


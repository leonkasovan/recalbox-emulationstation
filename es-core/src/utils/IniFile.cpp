//
// Created by thierry.imbert on 18/02/2020.
//

#include "IniFile.h"

#include <utils/Files.h>
#include "utils/Log.h"

IniFile::IniFile(const Path& path, const Path& fallbackpath, bool extraSpace)
  : mFilePath(path)
  , mFallbackFilePath(fallbackpath)
  , mExtraSpace(extraSpace)
  , mValid(Load())
{
}

IniFile::IniFile(const Path& path, bool extraSpace)
  : mFilePath(path)
  , mFallbackFilePath()
  , mExtraSpace(extraSpace)
  , mValid(Load())
{
}

bool IniFile::IsValidKeyValue(const String& line, String& key, String& value, bool& isCommented)
{
  static String _allowedCharacters = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_.-";
  if (!line.empty()) // Ignore empty line
  {
    bool comment = (line[0] == '#');
    if (!comment)
    {
      int separatorPos = line.Find('=');
      if (separatorPos >= 0) // Expect a key=value line
      {
        key = line.SubString(0, separatorPos).Trim();
        isCommented = (!key.empty() && key[0] == ';');
        if (isCommented) key.erase(0, 1);
        value = line.SubString(separatorPos + 1).Trim();
        if (key.find_first_not_of(_allowedCharacters) == String::npos) return true;
        { LOG(LogWarning) << "[IniFile] Invalid key: `" << key << '`'; }
      }
      else { LOG(LogError) << "[IniFile] Invalid line: `" << line << '`'; }
    }
  }
  return false;
}

bool IniFile::Load()
{
  // Load file
  String content;
  if (!mFilePath.IsEmpty() && mFilePath.Exists()) content = Files::LoadFile(mFilePath);
  else if (!mFallbackFilePath.IsEmpty() && mFallbackFilePath.Exists()) content = Files::LoadFile(mFallbackFilePath);
  else return false;

  // Split lines
  content.Replace("\r", "");
  String::List lines = content.Split('\n');

  // Get key/value
  String key, value;
  bool comment = false;
  for (String& line : lines)
    if (IsValidKeyValue(line.Trim(" \t\r\n"), key, value, comment))
      if (!comment)
        mConfiguration[key] = value;

  OnLoad();
  return !mConfiguration.empty();
}

static bool MakeBootReadOnly()
{
  return system("mount -o remount,ro /boot") == 0;
}

static bool MakeBootReadWrite()
{
  return system("mount -o remount,rw /boot") == 0;
}

bool IniFile::Save()
{
  // No change?
  if (mPendingWrites.empty() && mPendingDelete.empty()) return true;

  // Load file
  String content = Files::LoadFile(mFilePath);

  // Split lines
  content.Replace("\r", "");
  String::List lines = content.Split('\n');

  // Save new value if exists
  String lineKey;
  String lineVal;
  String equal(mExtraSpace ? " = " : "=");
  for (auto& it : mPendingWrites)
  {
    // Write new kay/value
    String key = it.first;
    String val = it.second;
    bool lineFound = false;
    bool commented = false;
    for (auto& line : lines)
      if (IsValidKeyValue(line.Trim(), lineKey, lineVal, commented))
        if (lineKey == key)
        {
          line = key.Append(equal).Append(val);
          lineFound = true;
          break;
        }
    if (!lineFound)
      lines.push_back(key.Append(equal).Append(val));

    // Move from Pendings to regular Configuration
    mConfiguration[key] = val;
    mPendingWrites.erase(key);
  }

  // Delete (comment) keys
  for (auto& deletedKey : mPendingDelete)
  {
    bool commented = false;
    for (auto& line : lines)
      if (IsValidKeyValue(line.Trim(" \t\r\n"), lineKey, lineVal, commented))
        if (lineKey == deletedKey)
          if (!commented)
            line = String(';').Append(deletedKey).Append(equal).Append(lineVal);
  }
  mPendingDelete.clear();

  // Save new
  bool boot = mFilePath.StartWidth("/boot/");
  if (boot && MakeBootReadWrite()) { LOG(LogError) <<"[IniFile] Error remounting boot partition (RW)"; }
  Files::SaveFile(mFilePath, String::Join(lines, '\n').Append('\n'));
  if (boot && MakeBootReadOnly()) { LOG(LogError) << "[IniFile] Error remounting boot partition (RO)"; }

  OnSave();
  return true;
}

String IniFile::AsString(const String& name) const
{
  return ExtractValue(name);
}

String IniFile::AsString(const String& name, const String& defaultValue) const
{
  String item = ExtractValue(name);
  return (!item.empty()) ? item : defaultValue;
}

bool IniFile::AsBool(const String& name, bool defaultValue) const
{
  String item = ExtractValue(name);
  return (!item.empty()) ? (item.size() == 1 && item[0] == '1') : defaultValue;
}

unsigned int IniFile::AsUInt(const String& name, unsigned int defaultValue) const
{
  String item = ExtractValue(name);
  if (!item.empty())
  {
    long long int value = 0;
    if (item.TryAsInt64(value))
      return (unsigned int)value;
  }

  return defaultValue;
}

int IniFile::AsInt(const String& name, int defaultValue) const
{
  String item = ExtractValue(name);
  if (!item.empty())
  {
    int value = 0;
    if (item.TryAsInt(value))
      return value;
  }

  return defaultValue;
}

void IniFile::Delete(const String& name)
{
  mPendingDelete.insert(name);
}

void IniFile::SetString(const String& name, const String& value)
{
  mPendingDelete.erase(name);
  mPendingWrites[name] = value;
}

void IniFile::SetBool(const String& name, bool value)
{
  mPendingDelete.erase(name);
  mPendingWrites[name] = value ? "1" : "0";
}

void IniFile::SetUInt(const String& name, unsigned int value)
{
  mPendingDelete.erase(name);
  mPendingWrites[name] = String((long long)value);
}

void IniFile::SetInt(const String& name, int value)
{
  mPendingDelete.erase(name);
  mPendingWrites[name] = String(value);
}

void IniFile::SetList(const String& name, const String::List& values)
{
  mPendingDelete.erase(name);
  mPendingWrites[name] = String::Join(values, ',');
}

bool IniFile::isInList(const String& name, const String& value) const
{
  if (!value.empty())
    if (mConfiguration.contains(name))
    {
      String s = AsString(name);
      for (int p = s.Find(value); p >= 0; p = s.Find(value, p + value.Count()))
        if (p == 0 || s[p - 1] == ',')
          if (p + value.Count() >= s.Count() || s[p + value.Count()] ==  ',')
            return true;
    }
  return false;
}

const String& IniFile::ExtractValue(const String& key) const
{
  String* item = mPendingWrites.try_get(key);
  if (item == nullptr) item = mConfiguration.try_get(key);
  return (item != nullptr) ? *item : String::Empty;
}

bool IniFile::HasKeyStartingWith(const String& startWidth) const
{
  for (const auto& it : mPendingWrites)
    if (it.first.StartsWith(startWidth))
      return true;

  for (const auto& it : mConfiguration)
    if (it.first.StartsWith(startWidth))
      return true;

  return false;
}

bool IniFile::HasKey(const String& key) const
{
  for (const auto& it : mPendingWrites)
    if (it.first == key)
      return true;

  for (const auto& it : mConfiguration)
    if (it.first == key)
      return true;

  return false;
}

String::List IniFile::GetKeyEndingWith(const String& endWidth)
{
  String::List result;
  for (auto& it : mPendingWrites)
    if (it.first.EndsWith(endWidth))
      result.push_back(it.first);

  for (auto& it : mConfiguration)
    if (it.first.EndsWith(endWidth))
      result.push_back(it.first);

  return result;
}
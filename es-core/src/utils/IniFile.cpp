//
// Created by thierry.imbert on 18/02/2020.
// Last modification by Maksthorr on 28/04/2023
//

#include "IniFile.h"

#include <utils/Files.h>
#include "utils/Log.h"

IniFile::IniFile(const Path& path, const Path& fallbackpath, bool extraSpace, bool autoBackup)
  : mFilePath(path)
  , mFallbackFilePath(fallbackpath)
  , mExtraSpace(extraSpace)
  , mAutoBackup(autoBackup)
  , mValid(Load())
{
}

IniFile::IniFile(const Path& path, bool extraSpace, bool autoBackup)
  : mFilePath(path)
  , mFallbackFilePath()
  , mExtraSpace(extraSpace)
  , mAutoBackup(autoBackup)
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
        if (isCommented = (!key.empty() && key[0] == ';'); isCommented) key.erase(0, 1);
        value = line.SubString(separatorPos + 1).Trim();
        if (key.find_first_not_of(_allowedCharacters) == String::npos) return true;
        { LOG(LogWarning) << "[IniFile] Invalid key: `" << key << '`'; }
      }
      else { LOG(LogError) << "[IniFile] Invalid line: `" << line << '`'; }
    }
  }
  return false;
}

bool IniFile::LoadContent(String& content)
{
  // Regular file
  if (!mFilePath.IsEmpty() && mFilePath.Exists())
  {
    content = Files::LoadFile(mFilePath);
    { LOG(LogDebug) << "[IniFile] Load: Loading default file " << mFilePath << " of " << content.size() << " bytes."; }
    if (!content.empty()) return true;
  }

  // Backup if required
  if (mAutoBackup)
  {
    Path backup(mFilePath.ToString() + ".backup");
    if (!backup.IsEmpty() && backup.Exists())
    {
      content = Files::LoadFile(backup);
      { LOG(LogDebug) << "[IniFile] Load: Loading backup file " << backup << " of " << content.size() << " bytes."; }
      if (!content.empty()) return true;
    }
  }

  // Fallback file
  if (!mFallbackFilePath.IsEmpty() && mFallbackFilePath.Exists())
  {
    content = Files::LoadFile(mFallbackFilePath);
    { LOG(LogDebug) << "[IniFile] Load: Loading fallback filepath " << mFallbackFilePath << " of " << content.size() << " bytes."; }
    if (!content.empty()) return true;
  }

  return false;
}

bool IniFile::Load()
{
  // Load file
  String content;
  if (!LoadContent(content)) return false;

  // Split lines
  content.Remove('\r');
  String::List lines = content.Split('\n');
  { LOG(LogDebug) << "[IniFile] Load: " << lines.size() << " lines loaded."; }

  // Get key/value
  String key, value;
  bool comment = false;
  for (String& line : lines)
    if (IsValidKeyValue(line.Trim(), key, value, comment))
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
  Mutex::AutoLock locker(mLocker);

  // No change?
  if (mPendingWrites.empty() && mPendingDelete.empty()) return true;

  // Load file
  String content;
  if (!LoadContent(content))
  {
    { LOG(LogError) << "[IniFile] Save: Error loading base faile. Save aborted."; }
    return false;
  }

  // Split lines
  content.Remove('\r');
  String::List lines = content.Split('\n');
  { LOG(LogDebug) << "[IniFile] Save: " << lines.size() << " lines loaded."; }

  // Save new value if exists
  int replacedLines = 0;
  int addedLines = 0;
  int deletedLines = 0;
  String lineKey;
  String lineVal;
  String equal(mExtraSpace ? " = " : "=");
  for (auto& it : mPendingWrites)
  {
    // Write new kay/value
    String key(it.first);
    String orgKey(key);
    String val(it.second);
    bool lineFound = false;
    bool commented = false;
    for (auto& line : lines)
      if (IsValidKeyValue(line.Trim(), lineKey, lineVal, commented))
        if (lineKey == key)
        {
          line = key.Append(equal).Append(val);
          lineFound = true;
          replacedLines++;
          break;
        }
    if (!lineFound)
    {
      lines.push_back(key.Append(equal).Append(val));
      addedLines++;
    }

    // Move from Pendings to regular Configuration
    mConfiguration[orgKey] = val;
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
          {
            line = String(';').Append(deletedKey).Append(equal).Append(lineVal);
            deletedLines++;
          }
  }
  mPendingDelete.clear();

  { LOG(LogDebug) << "[IniFile] Save: " << replacedLines << " values replaced. " << addedLines << " keys/values added. " << deletedLines << " keys commented."; }

  // Save new
  bool result = true;
  bool boot = mFilePath.StartWidth("/boot/");
  if (boot && MakeBootReadWrite()) { LOG(LogError) <<"[IniFile] Error remounting boot partition (RW)"; }
  if (mAutoBackup)
  {
    Path backup(mFilePath.ToString() + ".backup");
    if (!backup.Delete()) { LOG(LogError) << "[IniFile] Save: Error deleting backup file " << backup; }
    if (!Path::Rename(mFilePath, backup)) { LOG(LogError) << "[IniFile] Save: Error moving file " << mFilePath << " to backup file " << backup; }
  }
  if (!Files::SaveFile(mFilePath, String::Join(lines, '\n')))
  {
    { result = false; LOG(LogError) << "[IniFile] Save: Error saving file " << mFilePath; }
    if (mAutoBackup)
    {
      { result = false; LOG(LogError) << "[IniFile] Save: Trying emergency rescue from backup file"; }
      Path backup(mFilePath.ToString() + ".backup");
      if (backup.Exists())
      {
        if (!mFilePath.Delete()) { LOG(LogWarning) << "[IniFile] Save: Error deleting unsafe " << mFilePath; }
        if (!Path::Rename(backup, mFilePath)) { LOG(LogError) << "[IniFile] Save: Error moving file " << backup << " to backup file " << mFilePath; }
      }
      else { LOG(LogError) << "[IniFile] Save: Backup file unavailable!"; }
    }
  }
  if (boot && MakeBootReadOnly()) { LOG(LogError) << "[IniFile] Error remounting boot partition (RO)"; }

  OnSave();
  return result;
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
    if (mConfiguration.contains(name) || mPendingWrites.contains(name))
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

bool IniFile::ResetWithFallback() {
  if (!mFallbackFilePath.IsEmpty() && mFallbackFilePath.Exists())
  {
    if(!Files::CopyFile(mFallbackFilePath, mFilePath))
      return false;
  }
  this->Cancel();
  mConfiguration.clear();
  return this->Load();
}

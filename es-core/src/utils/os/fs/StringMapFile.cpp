//
// Created by bkg2k on 17/10/2019.
//

#include "StringMapFile.h"

void StringMapFile::Load()
{
  mMap.clear();
  FILE* f = fopen(mPath.c_str(), "r");
  if (f != nullptr)
  {
    char buffer[2048];
    while (fgets(buffer, sizeof(buffer), f) != nullptr)
    {
      String keyvalue = buffer;
      if (int pos = keyvalue.Find('='); pos >= 0)
      {
        String key = keyvalue.SubString(0, pos).Trim();
        String value = keyvalue.SubString(pos + 1).Trim();
        mMap[key] = value;
      }
    }
    (void)fclose(f);
  }
}

void StringMapFile::Save()
{
  FILE* f = fopen(mPath.c_str(), "w+");
  if (f != nullptr)
  {
    for(auto& holder : mMap)
    {
      String keyvalue = holder.first;
      keyvalue += '=';
      keyvalue += holder.second;
      keyvalue += '\n';
      (void)fputs(keyvalue.c_str(), f);
    }
    (void)fclose(f);
  }
}

String StringMapFile::GetString(const String& key, const String& defaultvalue)
{
  if (String* found = mMap.try_get(key); found != nullptr)
    return *found;
  return defaultvalue;
}

int StringMapFile::GetInt(const String& key, int defaultvalue)
{
  if (String* found = mMap.try_get(key); found != nullptr)
    if (int value = defaultvalue; found->TryAsInt(value))
      return value;
  return defaultvalue;
}

bool StringMapFile::GetBool(const String& key, bool defaultvalue)
{
  if (String* found = mMap.try_get(key); found != nullptr)
    if (bool value = defaultvalue; found->TryAsBool(value))
      return value;
  return defaultvalue;
}

void StringMapFile::SetString(const String& key, const String& value)
{
  mMap[key] = value;
}

void StringMapFile::SetInt(const String& key, int value)
{
  mMap[key] = String(value);
}

void StringMapFile::SetBool(const String& key, bool value)
{
  mMap[key] = value ? "1" : "0";
}



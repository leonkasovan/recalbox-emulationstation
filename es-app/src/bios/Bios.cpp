//
// Created by bkg2k on 28/12/2019.
//

#include <utils/Log.h>
#include <utils/hash/Md5.h>
#include <utils/Files.h>
#include <RootFolders.h>
#include <utils/Zip.h>
#include "Bios.h"

Bios::Md5Hash::Md5Hash(const String& source)
  : Md5Hash()
{
  if (source.length() != sizeof(mBytes) * 2)
  {
    { LOG(LogError) << "[Bios] Invalid MD5: " << source; }
    mValid = false;
    return;
  }

  // Deserialize
  for(int i = (int)sizeof(mBytes); --i >= 0;)
  {
    unsigned char b = 0;
    unsigned char cl = (source[i * 2 + 0]) | 0x20 /* force lowercase - do not disturb digits */;
    unsigned char ch = (source[i * 2 + 1]) | 0x20 /* force lowercase - do not disturb digits */;
    b |= ((cl - 0x30) <= 9 ? (cl - 0x30) : ((cl - 0x61) <= 5) ? (cl - 0x61 + 10) : 0) << 4;
    b |= ((ch - 0x30) <= 9 ? (ch - 0x30) : ((ch - 0x61) <= 5) ? (ch - 0x61 + 10) : 0) << 0;
    mBytes[i] = b;
  }
  mValid = true;
}

String Bios::Md5Hash::ToString() const
{
  char hashStr[sizeof(mBytes) * 2];

  // Serialize
  for(int i = (int)sizeof(mBytes); --i >= 0;)
  {
    hashStr[i * 2 + 0] = "0123456789ABCDEF"[mBytes[i] >> 4];
    hashStr[i * 2 + 1] = "0123456789ABCDEF"[mBytes[i] & 15];
  }

  return String(hashStr, sizeof(hashStr));
}

Bios::Bios()
  : mMandatory(false),
    mHashMatchMandatory(false),
    mStatus(Status::Unknown),
    mReportStatus(ReportStatus::Unknown)
{
}

void Bios::Scan()
{
  // Scan
  bool found = false;
  for(int i = sMaxBiosPath; --i >= 0;)
  {
    if (!mPath[i].Exists()) continue;
    if (mPath[i].Extension().LowerCase() == ".zip")
    {
      // Get composite hash from the zip file
      String md5string = Zip(mPath[i]).Md5Composite();
      mRealFileHash = Md5Hash(md5string);
    }
    else
    {
      // Load bios
      String biosContent = Files::LoadFile(mPath[i]);

      // Compute md5
      MD5 md5;
      md5.update(biosContent.data(), biosContent.length());
      md5.finalize();
      mRealFileHash = Md5Hash(md5);
    }
    found = true;
  }

  // Not found
  if (!found)
  {
    mStatus = Status::FileNotFound;
    mReportStatus = mMandatory ? ReportStatus::Red : ReportStatus::Yellow;
    return;
  }

  // Matching?
  bool matching = false;
  for(Md5Hash& m : mHashes)
    if (matching = mRealFileHash.IsMatching(m); matching)
      break;

  // Get result
  if (matching)
  {
    mStatus = Status::HashMatching;
    mReportStatus = ReportStatus::Green;
  }
  else
  {
    mStatus = Status::HashNotMatching;
    mReportStatus = mHashMatchMandatory && mMandatory ? ReportStatus::Red : ReportStatus ::Yellow;
  }
}

bool Bios::IsForCore(const String& core) const
{
  // Contains?
  unsigned long long pos = mCores.find(core);
  if (pos == String::npos) return false;

  // Start?
  if (pos != 0)
    if (mCores[pos - 1] != ',')
      return false;

  // End?
  if (pos + core.length() < mCores.length())
    if (mCores[pos + core.length()] != ',')
      return false;

  return true;
}

Bios::Bios(const XmlNode& biosNode)
  : mStatus(Status::Unknown),
    mReportStatus(ReportStatus::Unknown)
{
  // Load mandatory fields
  pugi::xml_attribute path = biosNode.attribute("path");
  if (!path) { LOG(LogError) << "[Bios] Bios file's bios node is missing path!"; return; }
  pugi::xml_attribute hashes = biosNode.attribute("md5");
  if (!hashes) { LOG(LogError) << "[Bios] Bios file's bios node is missing md5!"; return; }
  pugi::xml_attribute cores = biosNode.attribute("core");
  if (!cores) { LOG(LogError) << "[Bios] Bios file's bios node is missing cores!"; return; }

  // Set mandatory fields
  String::List list = String(path.value()).Split('|');
  for(int i = sMaxBiosPath; --i >= 0; )
    if ((int)list.size() > i)
    {
      mPath[i] = Path(list[i]);
      if (!mPath[i].IsAbsolute())
        mPath[i] = RootFolders::DataRootFolder / "bios" / path.value();
    }

  mCores = cores.value();
  String::List md5list = String(hashes.value()).Split(',');
  for(String& md5string : md5list)
  {
    Md5Hash md5(md5string.Trim());
    if (md5.IsValid())
      mHashes.push_back(md5);
  }

  // Optionnal fields
  mMandatory = strcmp(biosNode.attribute("mandatory").value(), "false") != 0;
  mHashMatchMandatory = strcmp(biosNode.attribute("hashMatchMandatory").value(), "false") != 0;
  mNotes = biosNode.attribute("note").value();
}

String::List Bios::MD5List() const
{
  String::List result;
  for(const Md5Hash& hash : mHashes)
    if (hash.IsValid())
      result.push_back(hash.ToString());
  return result;
}

String Bios::Filename(bool shorten) const
{
  bool ok = false;

  // Try to make relative to the bios folder
  Path rootPath(RootFolders::DataRootFolder);
  String result = mPath[0].MakeRelative(rootPath, ok).ToString();
  // Too long?
  if (shorten)
    if (result.Count('/') > 2)
      result = String(".../").Append(mPath[0].Filename());

  return result;
}

String Bios::GenerateReport() const
{
  String report;

  switch(mStatus)
  {
    case Status::FileNotFound:
    case Status::HashNotMatching:
    {
      // Ignore No matching case when it's not required
      if (mStatus == Status::HashNotMatching && !mHashMatchMandatory) break;

      // BIOS
      report.Append("  ").Append(mStatus == Status::FileNotFound ? "MISSING " : "INCORRECT ")
            .Append(mMandatory ? "REQUIRED " : "OPTIONAL ")
            .Append("BIOS: ").Append(mPath[0].Filename()).Append(String::CRLF);

      // Information
      report.Append("    Path: ").Append(mPath[0].ToString());
      for(int i = sMaxBiosPath; --i >= 1; )
        if (!mPath[i].IsEmpty()) report.Append(" or ").Append(mPath[i].ToString());
      report.Append(String::CRLF);
      if (!mNotes.empty()) report.Append("    Notes: ").Append(mNotes).Append(String::CRLF);
      if (!mCores.empty()) report.Append("    For: ").Append(mCores).Append(String::CRLF);

      // MD5
      if (mStatus == Status::HashNotMatching)
        report.Append("    Current MD5: ").Append(mRealFileHash.ToString()).Append(String::CRLF);
      report.Append("    Possible MD5 List:\r\n");
      for(const Md5Hash& hash : mHashes)
        report.Append("      ").Append(hash.ToString()).Append(String::CRLF);
      break;
    }
    case Status::Unknown:
    case Status::HashMatching:
    default: break;
  }

  return report;
}


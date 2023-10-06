//
// Created by bkg2k on 28/12/2019.
//

#include "BiosList.h"
#include <utils/Log.h>

BiosList::BiosList(XmlNode& systemNode)
{
  // Extract system
  mSystemFullName = systemNode.attribute("fullname").value();
  mSystemName = systemNode.attribute("platform").value();
  if (mSystemFullName.empty() || mSystemName.empty())
  { LOG(LogWarning) << "[Bios] Bios file's system node is missing fullname and/or platform attributes!"; }

  // Extract bios
  for(XmlNode& biosNode : systemNode.children("bios"))
  {
    Bios bios(biosNode);
    if (bios.IsValid())
      mBiosList.push_back(bios);
  }
}

int BiosList::TotalBiosOk() const
{
  int result = 0;
  for(const Bios& bios : mBiosList)
    if (bios.LightStatus() == Bios::ReportStatus::Green)
      result++;
  return result;
}

int BiosList::TotalBiosKo() const
{
  int result = 0;
  for(const Bios& bios : mBiosList)
    if (bios.LightStatus() == Bios::ReportStatus::Red || bios.LightStatus() == Bios::ReportStatus::Unknown)
      result++;
  return result;
}

int BiosList::TotalBiosUnsafe() const
{
  int result = 0;
  for(const Bios& bios : mBiosList)
    if (bios.LightStatus() == Bios::ReportStatus::Yellow)
      result++;
  return result;
}

int BiosList::TotalFileNotFound() const
{
  int result = 0;
  for(const Bios& bios : mBiosList)
    if (bios.BiosStatus() == Bios::Status::FileNotFound)
      result++;
  return result;
}

int BiosList::TotalHashMatching() const
{
  int result = 0;
  for(const Bios& bios : mBiosList)
    if (bios.BiosStatus() == Bios::Status::HashMatching)
      result++;
  return result;
}

int BiosList::TotalHashNotMatching() const
{
  int result = 0;
  for(const Bios& bios : mBiosList)
    if (bios.BiosStatus() == Bios::Status::HashNotMatching)
      result++;
  return result;
}

Bios::ReportStatus BiosList::ReportStatus() const
{
  int Red = 0;
  int Yellow = 0;
  int Green = 0;

  for(const Bios& bios : mBiosList)
    switch(bios.LightStatus())
    {
      case Bios::ReportStatus::Unknown:           break;
      case Bios::ReportStatus::Green  : Green++;  break;
      case Bios::ReportStatus::Yellow : Yellow++; break;
      case Bios::ReportStatus::Red    : Red++;    break;
    }

  if (Red    != 0) return Bios::ReportStatus::Red;
  if (Yellow != 0) return Bios::ReportStatus::Yellow;
  if (Green  != 0) return Bios::ReportStatus::Green;
  return Bios::ReportStatus::Unknown;
}

String::List BiosList::GetMissingBiosFileList() const
{
  String::List result;
  for(const Bios& bios : mBiosList)
    result.push_back(bios.Filename(false));

  return result;
}

String BiosList::GenerateReport() const
{
  String header("SYSTEM: ");
  header.Append(mSystemFullName).Append(String::CRLF)
        .Append("---------------------------------------------\r\n\r\n");
  String report;

  for(const Bios& bios : mBiosList)
  {
    String subReport = bios.GenerateReport();
    if (!subReport.empty())
      report.Append(subReport).Append(String::CRLF);
  }

  return report.empty() ? report : header + report;
}


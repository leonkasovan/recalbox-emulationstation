//
// Created by bkg2k on 06/04/2020.
//

#include <utils/Files.h>
#include <utils/Log.h>
#include <utils/datetime/DateTime.h>
#include <web/server/handlers/MqttTopics.h>
#include <cstring>
#include "SysInfos.h"

SysInfos::SysInfos()
  : mMqttClient("recalbox-api-server-broadcaster", nullptr),
    mPlatform(Platform::Unknown),
    mData {}
{
  mData.Reset();
  ReadCPUNames();
  LOG(LogInfo) << "[SysInfo] CPU Count: " << mData.CpuCount;
  Start("SysInfo");
}

void SysInfos::Run()
{
  while(IsRunning())
  {
    DateTime start;
    RefreshAll();
    SendMqttObjects(start);
    DateTime stop;

    if (IsRunning())
    {
      TimeSpan elapsed(stop - start);
      int msLeft = 1000 - (int)elapsed.TotalMilliseconds();
      if (msLeft > 0)
        usleep(msLeft * 1000);
    }
  }
}

void SysInfos::ReadCPUNames()
{
  String::List lines = Files::LoadFile(Path("/proc/cpuinfo")).Split('\n');
  mData.CpuCount = 0;
  int processorNumber = 0;
  String hardware;
  for(const String& line : lines)
  {
    int p = line.Find(':');
    if (p >= 0)
    {
      String key = line.SubString(0, p).Trim();
      String value = line.SubString(p + 1).Trim();
      if (key == "processor")
      {
        if (value.TryAsInt(processorNumber))
          mData.CpuCount = (processorNumber + 1 > mData.CpuCount) ? processorNumber + 1 : mData.CpuCount;
        else
          LOG(LogError) << "Unreadable line: " << line;
      }
      if (key == "model name" && (unsigned int)processorNumber < sMaxCPU)
          mCpuNames[processorNumber] = value;
      if (key == "Hardware")
        hardware = value;
    }
  }
  // Get platform
  mPlatform = GetPlatformFrom(hardware);
}

void SysInfos::GetCpu()
{
  // Get raw values
  String::List lines = Files::LoadFile(Path("/proc/stat")).Split('\n');
  long long cpuData[sMaxCPU][CpuDataType::__Count];
  int cpuIndex = 0;
  for(const String& line : lines)
    if (line.length() > 3)
      if (line[0] == 'c')
        if (line[1] == 'p')
          if (line[2] == 'u')
            if ((unsigned int)line[3] - 0x30u <= 9)
            {
              if (line.TryAsInt(3, ' ', cpuIndex))
              {
                int start = 3;
                for(int i = 0; i < CpuDataType::__Count; ++i)
                {
                  start = line.Find(' ', start + 1);
                  if (!line.TryAsInt64(start + 1, ' ', cpuData[cpuIndex][i]))
                    (void)line.TryAsInt64(start + 1, 0, cpuData[cpuIndex][i]);
                }
              }
            }

  // Get percent
  for (int i = mData.CpuCount; --i >= 0;)
  {
    // Get total time for the current cpu
    long long total = 0;
    long long diffs[CpuDataType::__Count];
    for (int j = CpuDataType::__Count; --j >= 0;)
      total += (diffs[j] = cpuData[i][j] - mData.CpuData[i][j]);

    // Get IDLE time
    long long idle = diffs[CpuDataType::Idle] + diffs[CpuDataType::IOWait];

    // Get percent
    double percent = (double) (total - idle) / (double) total;
    mData.Cpu[i][mData.StorageIndex] = (float) (percent * 100.0);
  }

  // Store values
  memcpy(mData.CpuData, cpuData, sizeof(mData.CpuData));
}

void SysInfos::GetTemperature()
{
  switch(mPlatform)
  {
    case Platform::Unknown: break;
    case Platform::PC:
    case Platform::RaspberryPi:
    case Platform::Anbernic:
    {
      String temp = Files::LoadFile(Path("/sys/class/thermal/thermal_zone0/temp"));
      int temperature = 0;
      if (temp.Trim().TryAsInt(temperature))
        mData.Temperature[mData.StorageIndex] = (float)temperature / 1000.0f;
      break;
    }
    case Platform::Odroid:
    {
      String temp = Files::LoadFile(Path("/sys/devices/virtual/thermal/thermal_zone0/temp"));
      int temperature = 0;
      if (temp.TryAsInt(temperature))
        mData.Temperature[mData.StorageIndex] = (float)temperature / 1000.0f;
      break;
    }
  }
}

void SysInfos::GetMemory()
{
  #define __STR(x) x, (int)sizeof(x) - 1
  // Get raw values
  String::List lines = Files::LoadFile(Path("/proc/meminfo")).Split('\n');
  int dataToGet = 3; // Total, Free & Available
  long long total = 0, free = 0, available = 0;
  for(const String& line : lines)
  {
    // Collect target data
    long long* data = nullptr;
    if (line.StartsWith(__STR("MemTotal:"))) data = &total;
    else if (line.StartsWith(__STR("MemFree:"))) data = &free;
    else if (line.StartsWith(__STR("MemAvailable:"))) data = &available;
    // Get data
    if (data != nullptr)
    {
      int pos = (int)line.find_first_not_of(' ', line.find(':') + 1);
      (void)line.TryAsInt64(pos, ' ', *data);
      // Completed?
      if (--dataToGet == 0)
      {
        mData.TotalMemory = total << 10;
        mData.FreeMemory[mData.StorageIndex] = free << 10;
        mData.AvailableMemory[mData.StorageIndex] = available << 10;
        break; // All data collected
      }
    }
  }
  #undef __STR
}

void SysInfos::RefreshAll()
{
  // Get all data
  GetCpu();
  GetMemory();
  GetTemperature();

  // Next storage
  mData.StorageIndex = (mData.StorageIndex + 1) & sStorageMask;
}

void SysInfos::CpuConsumption(int cpu, FloatStorage& percents) const
{
  GetData<float>(percents, mData.Cpu[cpu]);
}

SysInfos::Platform SysInfos::GetPlatformFrom(const String& platformName)
{
  if (platformName.StartsWith("BCM2835")) return Platform::RaspberryPi;
  if (platformName.StartsWith("ODROID")) return Platform::Odroid;
  if (platformName.StartsWith("Anbernic")) return Platform::Anbernic;
  return Platform::PC;
}

const char* SysInfos::GetPlatformFrom(SysInfos::Platform platform)
{
  switch(platform)
  {
    case Platform::Unknown: break;
    case Platform::RaspberryPi: return "Raspberry Pi";
    case Platform::Odroid: return "Odroid";
    case Platform::PC: return "Compatible PC";
    case Platform::Anbernic: return "Anbernic";
  }
  return "Unknown";
}

void SysInfos::Temperature(FloatStorage& temperature) const
{
  GetData<float>(temperature, mData.Temperature);
}

void SysInfos::Memory(LongStorage& free, LongStorage& available) const
{
  GetData<long long>(free, mData.FreeMemory);
  GetData<long long>(available, mData.AvailableMemory);
}

JSONBuilder SysInfos::BuildCpuObject(bool all)
{
  SysInfos::FloatStorage cpusData;
  int length = all ? SysInfos::MeasureCount() : 1;

  JSONBuilder cpus;
  cpus.Open();
  for(int i = mData.CpuCount; --i >= 0;)
  {
    CpuConsumption(i, cpusData);
    cpus.OpenObject(String(i).c_str())
        .Field(all ? "model" : nullptr, mCpuNames[i])
        .Field("consumption", cpusData + SysInfos::MeasureCount() - length, length)
        .CloseObject();
  }
  cpus.Close();

  return cpus;
}

JSONBuilder SysInfos::BuildMemoryObject(bool all) const
{
  // Build memory object
  SysInfos::LongStorage freeData;
  SysInfos::LongStorage availData;
  Memory(freeData, availData);
  int length = all ? SysInfos::MeasureCount() : 1;

  JSONBuilder memory;
  memory.Open()
        .Field("total", mData.TotalMemory)
        .Field("free", freeData + SysInfos::MeasureCount() - length, length)
        .Field("available", availData + SysInfos::MeasureCount() - length, length)
        .Close();

  return memory;
}

JSONBuilder SysInfos::BuildPlatformObject()
{
  JSONBuilder platform;
  platform.Open()
          .Field("platform", GetPlatformFrom(mPlatform))
          .Close();

  return platform;
}

JSONBuilder SysInfos::BuildTemperatureObject(bool all) const
{
  SysInfos::FloatStorage temperatures;
  Temperature(temperatures);
  int length = all ? SysInfos::MeasureCount() : 1;

  JSONBuilder temp;
  temp.Open()
      .Field("unit", "°C")
      .Field("temperatures", temperatures + SysInfos::MeasureCount() - length, length)
      .Close();

  return temp;
}

void SysInfos::SendMqttObjects(const DateTime& timestamp)
{
  JSONBuilder sysInfo;
  sysInfo.Open()
         .Field("timestamp", timestamp.ToISO8601())
         .Field("cpus", BuildCpuObject(false))
         .Field("memory", BuildMemoryObject(false))
         .Field("temperature", BuildTemperatureObject(false))
         .Close();

  mMqttClient.Send(MqttTopics::sSystemInfoTopic, sysInfo);
}

void SysInfos::DataBag::Reset()
{
  memset(this, 0, sizeof(SysInfos::DataBag));
}

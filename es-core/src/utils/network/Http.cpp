//
// Created by bkg2k on 05/12/2019.
//

#include "HttpClient.h"
#include "utils/Files.h"
#include "utils/Log.h"

HttpClient::DownloadInfo HttpClient::sDownloadStorage[sMaxBandwidthInfo];
int HttpClient::sDownloadCount = 0;
int HttpClient::sDownloadIndex = 0;
Mutex HttpClient::sDownloadLocker;

HttpClient::HttpClient() noexcept
  : mHandle(curl_easy_init())
  , mStringList(nullptr)
  , mIDownload(nullptr)
  , mContentSize(0)
  , mContentLength(0)
  , mContentFlushed(0)
  , mLastReturnCode(0)
  , mCancel(false)
{
  if (mHandle != nullptr)
  {
    curl_easy_setopt(mHandle, CURLOPT_REDIR_PROTOCOLS,CURLPROTO_HTTP | CURLPROTO_HTTPS);
    curl_easy_setopt(mHandle, CURLOPT_TCP_KEEPALIVE, 1L);
    curl_easy_setopt(mHandle, CURLOPT_TCP_KEEPIDLE, 240L);
    curl_easy_setopt(mHandle, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(mHandle, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(mHandle, CURLOPT_WRITEDATA, this);
  }
}

HttpClient::~HttpClient()
{
  if (mStringList != nullptr)
    curl_slist_free_all(mStringList);
  if (mHandle != nullptr)
    curl_easy_cleanup(mHandle);
}

bool HttpClient::Execute(const String& url, String& output)
{
  if (mHandle != nullptr)
  {
    DateTime start;
    mContentSize = 0;
    mLastReturnCode = 0;
    mResultHolder.clear();
    mResultFile = Path::Empty;
    DataStart();
    curl_easy_setopt(mHandle, CURLOPT_URL, url.c_str());
    CURLcode res = curl_easy_perform(mHandle);
    curl_easy_getinfo(mHandle, CURLINFO_RESPONSE_CODE, &mLastReturnCode);
    StoreDownloadInfo(start, DateTime(), mContentSize);
    output = std::move(mResultHolder);
    bool ok = (res == CURLcode::CURLE_OK);
    if (ok) DataEnd();
    return ok;
  }
  return false;
}

bool HttpClient::Execute(const String& url, const Path& output)
{
  if (mHandle != nullptr)
  {
    if (!output.Directory().Exists())
      (void)output.Directory().CreatePath();
    DateTime start;
    mContentSize = 0;
    mContentFlushed = 0;
    mLastReturnCode = 0;
    mResultHolder.clear();
    mResultFile = output;
    (void)mResultFile.Delete();
    DataStart();
    curl_easy_setopt(mHandle, CURLOPT_URL, url.c_str());
    CURLcode res = curl_easy_perform(mHandle);
    WriteCallback(nullptr, 0, 0, this);
    curl_easy_getinfo(mHandle, CURLINFO_RESPONSE_CODE, &mLastReturnCode);
    StoreDownloadInfo(start, DateTime(), mContentSize);
    bool ok = (res == CURLcode::CURLE_OK);
    if (!ok) (void)output.Delete();
    else DataEnd();
    return ok;
  }
  return false;
}

bool HttpClient::Execute(const String& url, const Path& output, HttpClient::IDownload* interface)
{
  mIDownload = interface;
  return Execute(url, output);
}

size_t HttpClient::WriteCallback(char* ptr, size_t size, size_t nmemb, void* userdata)
{
  HttpClient& This = *((HttpClient*)userdata);

  return This.DoDataReceived(ptr, (int)(size * nmemb));
}

size_t HttpClient::DoDataReceived(const char* data, int length)
{
  // Always store into the string
  mResultHolder.Append(data, length);
  mContentSize += (long long)(length);

  // Get content length
  if (mContentLength == 0)
  {
    double contentLength = 0;
    curl_easy_getinfo(mHandle, CURLINFO_CONTENT_LENGTH_DOWNLOAD, &contentLength);
    mContentLength = (long long)contentLength;
  }

  // Callback?
  if (mIDownload != nullptr)
    mIDownload->DownloadProgress(*this, mContentSize, mContentLength);

  // Inherited processing
  DataReceived(data, length);

  // Should flush?
  if (!mResultFile.IsEmpty())
    if (mResultHolder.length() > sMaxDataKeptInRam || length == 0)
    {
      // Try flushing to disk
      if (!Files::AppendToFile(mResultFile, mResultHolder.c_str(), (int)mResultHolder.length()))
        return 0; // Error flushing to disk
      mContentFlushed += (int)mResultHolder.length();
      // Clear the string
      mResultHolder.clear();
    }

  return mCancel ? 0 : length;
}

void HttpClient::Cancel()
{
  //if (mHandle != nullptr)
  //  curl_easy_reset(mHandle);
  mCancel = true;
}

void HttpClient::SetBasicAuth(const String& login, const String& password)
{
  if (mHandle != nullptr)
  {
    // Hold strings
    mLogin = login;
    mPassword = password;
    // Set basic auth
    curl_easy_setopt(mHandle, CURLOPT_HTTPAUTH, (long)CURLAUTH_BASIC);
    curl_easy_setopt(mHandle, CURLOPT_USERNAME, mLogin.data());
    curl_easy_setopt(mHandle, CURLOPT_PASSWORD, mPassword.data());
  }
}

void HttpClient::SetBearer(const String& bearer)
{
  if (mHandle != nullptr)
  {
    // Hold strings
    mBearer = bearer;
    // Set bearer auth
    if (mStringList != nullptr) curl_slist_free_all(mStringList);
    mStringList = nullptr;
    const String header = String("Authorization: Bearer ").Append(bearer);
    mStringList = curl_slist_append(mStringList, header.c_str());
    curl_easy_setopt(mHandle, CURLOPT_HTTPHEADER, mStringList);
  }
}

void HttpClient::CancelBasicAuth()
{
  if (mHandle != nullptr)
  {
    // Reset strings
    mLogin.clear();
    mPassword.clear();
    // Set basic auth
    curl_easy_setopt(mHandle, CURLOPT_HTTPAUTH, (long)CURLAUTH_NONE);
    curl_easy_setopt(mHandle, CURLOPT_USERNAME, mLogin.data());
    curl_easy_setopt(mHandle, CURLOPT_PASSWORD, mPassword.data());
  }
}

void HttpClient::StoreDownloadInfo(const DateTime& start, const DateTime& stop, long long size)
{
  long long elapsed = (stop - start).TotalMilliseconds();
  Mutex::AutoLock locker(sDownloadLocker);
  DownloadInfo& info = sDownloadStorage[sDownloadIndex];
  info.Time = elapsed;
  info.Size = size;
  if (++sDownloadIndex >= sMaxBandwidthInfo) sDownloadIndex = 0;
  sDownloadCount++;

  { LOG(LogDebug) << "[Http] Average Bandwidth: " << (float)GetAverageBandwidth() << " bytes/s."; }
}

double HttpClient::GetAverageBandwidth()
{
  if (sDownloadCount < 16) return -1; // Not enough data
  Mutex::AutoLock locker(sDownloadLocker);
  int count = sDownloadCount > sMaxBandwidthInfo ? sMaxBandwidthInfo : sDownloadCount;
  long long accumulatedBytes = 0;
  long long accumulatedTime = 0;
  for(int i = count; --i >= 0;)
  {
    accumulatedBytes += sDownloadStorage[i].Size;
    accumulatedTime += sDownloadStorage[i].Time;
  }
  return (double)accumulatedBytes / ((double)accumulatedTime / 1000.0);
}

//
// Created by davidb2111 on 14/04/2023
//
// As part of the RECALBOX Project
// http://www.recalbox.com
//
#include <utils/network/HttpUnxzUntar.h>
#include "utils/Log.h"

HttpUnxzUntar::HttpUnxzUntar(const Path& outputPath)
  : mOutputPath(outputPath)
  , mInbuf()
  , mOutbuf()
{
  if (xz.InitDecoder() != LZMA_OK) {
    // Decoder initialization failed. There's no point
    // to retry it so we need to exit.
    { LOG(LogError) << "[HttpUnxzUntar] Error initializing XZ decoder"; }
    return;
  }
  { LOG(LogDebug) << "[HttpUnxzUntar] Xz initialized"; }

  tar.Untar(mOutputPath.ToString());
  { LOG(LogDebug) << "[HttpUnxzUntar] Tar initialized"; }

}

void HttpUnxzUntar::DataReceived(const char* data, int length)
{
  if (!ProcessBuffer(data, length, LZMA_RUN)) {
    if (tar.Error())
    { 
      { LOG(LogError) << "[HttpUnxzUntar] Tar error: " << tar.ErrorMessage(); };
      { LOG(LogError) << "[HttpUnxzUntar] Download aborted"; };
      Cancel();
    }
    if (xz.Error() != LZMA_OK && xz.Error() != LZMA_STREAM_END )
    { 
      { LOG(LogError) << "[HttpUnxzUntar] Xz error: " << xz.ErrorMessage(); };
      { LOG(LogError) << "[HttpUnxzUntar] Download aborted"; };
      Cancel();
    }
  }
}

size_t HttpUnxzUntar::ProcessBuffer(const char* ptr, size_t available, lzma_action action=LZMA_RUN) {
  size_t unarchived = 0; 
  size_t availableout = 0;

  xz.InjectBuffer((uint8_t*)ptr, available, action);
 
  while (xz.IsStillDecompressing()) {
    availableout = xz.Decompress(mOutbuf, BUFSIZ);
    if (!availableout) {
      return 0;
    }
    tar.InjectBuffer(mOutbuf, availableout);
    unarchived+=availableout;
  
  }

  return unarchived;
}
void HttpUnxzUntar::DataStart()
{
}

void HttpUnxzUntar::DataEnd()
{
}

bool HttpUnxzUntar::SimpleExecute(const String& url, HttpClient::IDownload* interface)
{
  mIDownload = interface;
  if (mHandle != nullptr)
  {
    DateTime start;
    mContentSize = 0;
    mContentFlushed = 0;
    mLastReturnCode = 0;
    mResultFile = Path("/dev/null");
    DataStart();
    curl_easy_setopt(mHandle, CURLOPT_URL, url.c_str());
    curl_easy_setopt(mHandle, CURLOPT_FAILONERROR, 1L);
    CURLcode res = curl_easy_perform(mHandle);
    curl_easy_getinfo(mHandle, CURLINFO_RESPONSE_CODE, &mLastReturnCode);
    StoreDownloadInfo(start, DateTime(), mContentSize);
    DataEnd();
    bool ok = (res == CURLcode::CURLE_OK);
    if (ok && !mCancel && (xz.Error() == LZMA_OK || xz.Error() == LZMA_STREAM_END) && tar.Error() == TAR_NO_ERROR)
      return true;
    { LOG(LogError) << "[HttpUnxzUntar] Error downloading upgrade from " << url; }
    { LOG(LogError) << "[HttpUnxzUntar]  Curl return code: " << res << '(' << curl_easy_strerror(res) << "), HTTP response code: " << mLastReturnCode; }
    { LOG(LogError) << "[HttpUnxzUntar]  Xz last error: " << xz.Error(); }
    { LOG(LogError) << "[HttpUnxzUntar]  tar last error: " << tar.Error(); }
    { LOG(LogError) << "[HttpUnxzUntar]  cancel status: " << mCancel; }
  }
  return false;
}


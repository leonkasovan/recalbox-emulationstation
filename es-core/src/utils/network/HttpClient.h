//
// Created by bkg2k on 05/12/2019.
//
#pragma once

#include <utils/String.h>
#include "utils/os/fs/Path.h"
#include "utils/os/system/Mutex.h"
#include "utils/datetime/DateTime.h"
#include "curl/curl.h"

class HttpClient
{
  public:
    //! Download interface
    class IDownload
    {
      public:
        //! Default destructor
        virtual ~IDownload() = default;

        /*!
         * @brief Notify of download progress
         * @param http HTTP request
         * @param currentSize downloaded bytes
         * @param expectedSize total expected bytes
         */
        virtual void DownloadProgress(const HttpClient& http, long long currentSize, long long expectedSize) = 0;
    };

    /*!
     * @brief Default constructor
     */
    HttpClient() noexcept;

    /*!
     * @brief Destructor
     */
    virtual ~HttpClient();

    /*!
     * @brief Execute an HTTP Request ang get the result into a string
     * @param url Target URL
     * @param output String to write request result into
     * @return True if the request executed successfuly
     */
    bool Execute(const String& url, String& output);

    /*!
     * @brief Execute an HTTP Request ang save the result into a file
     * @param url Target URL
     * @param output File path to save request result into
     * @return True if the request executed successfuly
     */
    bool Execute(const String& url, const Path& output);

    /*!
     * @brief Execute an HTTP Request ang save the result into a file
     * @param url Target URL
     * @param output File path to save request result into
     * @param interface IDownload interface
     * @return True if the request executed successfuly
     */
    bool Execute(const String& url, const Path& output, IDownload* interface);

    //bool SimpleExecute(const String& url, IDownload* interface);
    /*!
     * @brief Asynchronously cancel a running request
     */
    void Cancel();

    /*!
     * @brief Get last http response code
     * @return Http code, or 0 if the request failed to execute
     */
    [[nodiscard]] int GetLastHttpResponseCode() const { return mLastReturnCode; }

    /*!
     * @brief Get last http content size
     * @return Content size
     */
    [[nodiscard]] long long GetLastContentSize() const { return mContentSize; }

    /*!
     * @brief When content has been downloaded, return if the output file match content size
     * @return True if the file is complete, false otherwise
     */
    [[nodiscard]] bool IsOutputFileValid() const { return mContentSize == mContentFlushed; }

    /*!
     * @brief Set basic authentication
     * @param login Login
     * @param password Password
     */
    void SetBasicAuth(const String& login, const String& password);

    /*!
     * @brief Set bearer authentication
     * @param bearer bearer
     */
    void SetBearer(const String& bearer);

    /*!
     * @brief Unset basic authentication
     */
    void CancelBasicAuth();

    /*!
     * @brief Get average bandwidth based on download storage
     * @return Actuel bandwidth or negative value if there is not enough informations
     */
    static double GetAverageBandwidth();

  protected:
    //! Maximum data kept in memory before flushing to file
    static constexpr int sMaxDataKeptInRam = (2 << 20); // 20Mb

    //! Bearer (Authorization)
    String mBearer;
    //! Login (basic auth)
    String mLogin;
    //! Password (basic auth)
    String mPassword;
    //! Request result, or temporary buffer waiting for disk flush
    String mResultHolder;
    //! Store request result into this file if it's not empty
    Path mResultFile;

    //! CURL Handle for all requests. Allocated once
    CURL* mHandle;
    //! CURL string list
    struct curl_slist* mStringList;
    //! Download intarface
    IDownload* mIDownload;
    //! Real content size
    long long mContentSize;
    //! Expected content size
    long long mContentLength;
    //! Content successfully flushed
    long long mContentFlushed;
    //! HTTP return code of last request
    int  mLastReturnCode;
    //! Cancel flag
    volatile bool mCancel;

    //! Maximum bandwidth stored information
    static constexpr int sMaxBandwidthInfo = 64;
    //! Donwload structure for bandwidth computations
    struct DownloadInfo
    {
      long long Size; //!< Downloaded bytes
      long long Time; //!< Elapsed time in millisecond
    };
    //! Download information storage
    static DownloadInfo sDownloadStorage[sMaxBandwidthInfo];
    //! Valid information count in the above array
    static int sDownloadCount;
    //! Next slot to write download info in
    static int sDownloadIndex;
    //! Storage protection
    static Mutex sDownloadLocker;

    /*!
     * @brief CURL callback when recveiving data
     * @param ptr Data pointer
     * @param size Data size multiplier (always 1)
     * @param nmemb Data size (real)
     * @param userdata Userdata pointer (pointer to class instance)
     * @return Must return nmemb when fully processed. Any other value means an error
     */
    static size_t WriteCallback(char *ptr, size_t size, size_t nmemb, void *userdata);

    /*!
     * @brief CURL callback when receiving data, class instance compatible
     * @param data Data pointer
     * @param length Data length
     * @return Must return length when fully processed. Any other value means an error
     */
    size_t DoDataReceived(const char* data, int length);

    /*!
     * @brief Data downloaded
     * @param data Data pointer
     * @param length Data length
     * @return Must return length when fully processed. Any other value means an error
     */
    virtual void DataReceived(const char* data, int length) { (void)data; (void)length; }

    /*!
     * @brief Called when data start to download
     */
    virtual void DataStart() {}

    /*!
     * @brief Called when all data are dowloaded
     */
    virtual void DataEnd() {}

    /*!
     * @brief Store download information
     * @param start Start time
     * @param stop Stop time
     * @param size Downloaded size
     */
    static void StoreDownloadInfo(const DateTime& start, const DateTime& stop, long long size);
};

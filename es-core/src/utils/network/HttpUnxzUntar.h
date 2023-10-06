//
// Created by davidb2111 on 14/04/2023
//
// As part of the RECALBOX Project
// http://www.recalbox.com
//
#pragma once

#include <utils/network/Http.h>
#include "utils/tar/Tar.h"
#include "utils/xz/Xz.h"

class HttpUnxzUntar : public Http
{
  public:
    HttpUnxzUntar(Path);
    bool SimpleExecute(const String& url, Http::IDownload* interface);
//    bool Execute(const String& url, const Path& output, IDownload* interface);

  private:
    //! Xz decompression facility
    Xz xz;

    //! Tar unarchiving facility
    Tar tar;

    /*!
     * @brief CURL callback when receiving data, class instance compatible
     * @param data Data pointer
     * @param length Data length
     * @return Must return length when fully processed. Any other value means an error
     */
    void DataReceived(const char* data, int length) final;

    /*!
     * @brief Called when data start to download
     */
    void DataStart() final;

    /*!
     * @brief Called when all data are dowloaded
     */
    void DataEnd() final;

    const Path *mOutputPath;
    /* !
     * @brief Process download buffer
     * first unxz and then untar
     */
    size_t ProcessBuffer(const char* ptr, size_t available, lzma_action actio);
    //! Buffers
    uint8_t mInbuf[BUFSIZ];
    size_t mAvailablein = 0;

    uint8_t mOutbuf[BUFSIZ];
};

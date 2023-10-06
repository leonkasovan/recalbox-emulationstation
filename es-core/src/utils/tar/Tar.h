//
// Created by davidb2111 on 14/04/2023
//
// As part of the RECALBOX Project
// http://www.recalbox.com
//
#pragma once

#include <iostream>
#include <utils/String.h>
#include <stdlib.h>

typedef struct TARFileHeader
{
  char filename[100];
  char mode[8];
  char uid[8];
  char gid[8];
  char fileSize[12];
  char lastModification[12];
  char checksum[8];
  char typeFlag;
  char linkedFileName[100];
  char ustarIndicator[6];
  char ustarVersion[2];
  char ownerUserName[32];
  char ownerGroupName[32];
  char deviceMajorNumber[8];
  char deviceMinorNumber[8];
  char filenamePrefix[155];
  char padding[12];
} tar_file_header;

typedef enum
{
  BUFFER_START,
  BUFFER_END,
  BEGIN_FILE,
  IN_FILE,
  BEGIN_DIRECTORY,
  BEGIN_PADDING,
  END_PADDING,
} tar_status;

typedef enum
{
  TAR_NO_ERROR,
  TAR_CANT_WRITE,
  TAR_CANT_MKDIR,
} tar_error;

class Tar
{
  public:
    Tar();
    ~Tar();
    size_t InjectBuffer(uint8_t *bufferin, int buffersize);
    void Untar(String path);
    void Close();
    tar_error Error();

    /*!
     * @brieg Returns last error message
     */
    String ErrorMessage();

  private:
    /*!
     * @brief Process input buffer and prepare tar header
     */
    uint64_t ProcessHeader(uint8_t *buffer, int buffersize);

    /*!
     * @brief Process input buffer and prepare file data
     */
    uint64_t ProcessFile(uint8_t *buffer, int buffersize);

    /*!
     * @brief Process input buffer and prepare directory
     */
    bool ProcessDirectory();

    /*!
     * @brief Process tar padding
     */
    uint64_t ProcessPadding(uint8_t *buffer, int buffersize);

    /*!
     * @brief Returns current processing file size
     */
    uint64_t CurrentFileSize();

    /*!
     * @brief Convert tar octal data into decimal
     */
    static uint64_t DecodeTarOctal(char* data, size_t size = 12);

    /*!
     * @brief Check for end of buffer
     */
    bool IsTarEnd();

    /*!
     * @brief Change current processing state
     */
    void SetState(tar_status new_status);

    /*!
     * @brief Returns padding size
     */
    uint64_t GetPaddingSize();

    /*!
     * @brief Destination dir
     */
    String m_outputpath;

    /*!
     * @brief Current processing file
     */
    String m_currentfile;

    /*!
     * @brief Processing status
     */
    tar_status currentstatus;

    /*!
     * @brief Current tar header
     */
    tar_file_header currentheader;

    /*!
     * @brief Current offset in buffer
     */
    uint32_t offsetinbuffer;

    /*!
     * @brief Report error
     */
    tar_error m_currenterror;

    /*!
     * @brief Specify the offset of the current object (file, header, padding)
     * used in the case it a splitted on two buffers
     */
    uint32_t offsetincurrentobject;
    FILE *currentfile;
};

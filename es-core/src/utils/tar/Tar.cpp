// Created by davidb2111 on 14/04/2023
//
// As part of the RECALBOX Project
// http://www.recalbox.com
//

#include "Tar.h"
#include "utils/Log.h"
#include <errno.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

#define min64(a, b) ((long long int)(a) < (long long int)(b) ? (a) : (b))
#define ASCII_TO_NUMBER(num) ((num)-48)

Tar::Tar() {
  SetState(BUFFER_START);
  m_currenterror = TAR_NO_ERROR;
  { LOG(LogDebug) << "[Tar] Tar initialized"; }
}

Tar::~Tar() {
  { LOG(LogDebug) << "[Tar] Tar terminated"; }
}

void Tar::Untar(String path) {
  struct stat sb;
  m_outputpath = path;
  if (stat(path.c_str(), &sb) != 0) {
    mkdir(path.c_str(), 0755);
  }
}

bool Tar::IsTarEnd() {
  uint8_t *pointer = (uint8_t *)&currentheader;
  for (int i = 0; i < 512; i++)
    if (*pointer != 0)
      return false;
  return true;
}

size_t Tar::InjectBuffer(uint8_t *bufferin, int buffersize) {
  size_t dataunarchived = 0;
  offsetinbuffer = 0;
  while (offsetinbuffer != (uint32_t )buffersize) {
    switch (currentstatus) {
    case BUFFER_START:
      offsetincurrentobject = 0;
      [[fallthrough]];
    case BUFFER_END: {
      offsetinbuffer +=
          ProcessHeader(bufferin + offsetinbuffer, buffersize - offsetinbuffer);
      if (offsetincurrentobject == sizeof(tar_file_header)) {
        if (IsTarEnd()) {
          SetState(BUFFER_START);
        } else {
          if (currentheader.typeFlag == '5') {
            SetState(BEGIN_DIRECTORY);
          } else if (currentheader.typeFlag == '0' ||
                     currentheader.typeFlag == 0) {
            SetState(BEGIN_FILE);
          }
        }
      } else {
        SetState(BUFFER_END);
      }
      break;
    }
    case BEGIN_FILE:
      offsetincurrentobject = 0;
      [[fallthrough]];
    case IN_FILE: {
      offsetinbuffer +=
          ProcessFile(bufferin + offsetinbuffer, buffersize - offsetinbuffer);
      if (offsetincurrentobject == CurrentFileSize()) {
        { LOG(LogDebug) << "[Tar] " << currentheader.filename; }
        SetState(BEGIN_PADDING);
      } else {
        SetState(IN_FILE);
      }
      dataunarchived = offsetinbuffer;
      break;
    }
    case BEGIN_PADDING:
      //  advance in buffer, for padding, continu on new buffer
      offsetincurrentobject = 0;
      [[fallthrough]];
    case END_PADDING: {
      offsetinbuffer += ProcessPadding(bufferin + offsetinbuffer,
                                       buffersize - offsetinbuffer);
      if (offsetincurrentobject == GetPaddingSize()) {
        SetState(BUFFER_START);
      } else {
        SetState(END_PADDING);
      }
      break;
    }
    case BEGIN_DIRECTORY: {
      {
        LOG(LogDebug) << "[Tar] " << currentheader.filename << '/';
      }
      ProcessDirectory();
      SetState(BUFFER_START);
      break;
    }
    }
  }
  /* buffer exhausted
   */
  return buffersize - dataunarchived;
}

uint64_t Tar::ProcessHeader(uint8_t *buffer, int buffersize) {
  uint64_t remainingbytesforheader =
      sizeof(tar_file_header) - offsetincurrentobject;
  uint64_t bytesforheader = min64(remainingbytesforheader, buffersize);
  memcpy((uint8_t *)&currentheader + offsetincurrentobject, buffer,
         bytesforheader);
  offsetincurrentobject += bytesforheader;

  return bytesforheader;
}

uint64_t Tar::ProcessFile(uint8_t *buffer, int buffersize) {
  uint64_t remainingbytestowrite = CurrentFileSize() - offsetincurrentobject;
  m_currentfile = (String)currentheader.filename;
  int bytestowrite = min64(remainingbytestowrite, buffersize);
  currentfile = fopen((m_outputpath + '/' + m_currentfile).c_str(),
                      currentstatus == BEGIN_FILE ? "w" : "a");
  if (currentfile) {
    fwrite(buffer, 1, bytestowrite, currentfile);
    fclose(currentfile);
    m_currenterror = TAR_NO_ERROR;
  } else {
    { LOG(LogError) << "Can't write to " << m_outputpath + '/' + m_currentfile << ": " << strerror(errno); }
    m_currenterror = TAR_CANT_WRITE;
  }
  offsetincurrentobject += bytestowrite;

  return bytestowrite;
}

bool Tar::ProcessDirectory() {
  String dirpath =
      m_outputpath + '/' + String(currentheader.filename);
  bool status = mkdir(dirpath.c_str(), 0755);
  m_currenterror = TAR_NO_ERROR;
  if (!status) {
    m_currenterror = TAR_CANT_MKDIR;
  }
  return status;
}

uint64_t Tar::ProcessPadding(uint8_t *buffer, int buffersize) {
  (void)buffer;
  int padding = GetPaddingSize();
  uint64_t remainingbytesforpadding = padding - offsetincurrentobject;
  uint64_t bytesforpadding = min64(remainingbytesforpadding, buffersize);
  offsetincurrentobject += bytesforpadding;

  return bytesforpadding;
}

uint64_t Tar::GetPaddingSize() {
  return (512 - (CurrentFileSize() % 512)) % 512;
}

uint64_t Tar::CurrentFileSize() {
  return DecodeTarOctal((char *)&currentheader.fileSize, 12);
}

uint64_t Tar::DecodeTarOctal(char *data, size_t size) {
  unsigned char *currentPtr = (unsigned char *)data + size;
  uint64_t sum = 0;
  uint64_t currentMultiplier = 1;
  // Skip everything after the last NUL/space character
  // In some TAR archives the size field has non-trailing NULs/spaces, so this
  // is neccessary
  unsigned char *checkPtr =
      currentPtr; // This is used to check where the last NUL/space char is
  for (; checkPtr >= (unsigned char *)data; checkPtr--) {
    if ((*checkPtr) == 0 || (*checkPtr) == ' ') {
      currentPtr = checkPtr - 1;
    }
  }
  for (; currentPtr >= (unsigned char *)data; currentPtr--) {
    sum += ASCII_TO_NUMBER(*currentPtr) * currentMultiplier;
    currentMultiplier *= 8;
  }
  return sum;
}

void Tar::SetState(tar_status new_status) { currentstatus = new_status; }

void Tar::Close() { delete this; }

tar_error Tar::Error() {
  return m_currenterror;
}

String Tar::ErrorMessage() {
  switch (m_currenterror) {
    case TAR_NO_ERROR:
      return String("no error");
      break;
    case TAR_CANT_WRITE:
      return String("can't write file");
      break;
    case TAR_CANT_MKDIR:
      return String("can't create directory");
      break;
    default:
      return String("unknown error");
      break;
  }
}

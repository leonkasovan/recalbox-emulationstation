#include "UDev.h"

UDev::UDev()
  : mContext(udev_new())
{
}

UDev::~UDev()
{
  if (mContext != nullptr)
    udev_unref(mContext);
}

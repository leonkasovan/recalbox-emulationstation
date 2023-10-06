//
// Created by pre2010-02 on 13/01/23.
//

#include <sys/stat.h>
#include "SaveState.h"

SaveState::SaveState(const Path& path)
  : mPath(path),
  mThumbnailPath(path.ChangeExtension(path.Extension() + ".png")),
  mIsAuto(false)
{
  struct stat attr{};
  stat(path.ToChars(), &attr);
  mDateTime = DateTime((long long)attr.st_mtime);

  String ext = mPath.Extension();
  if (!path.Exists()) mSlotNumber = -1;
  else if ( ext == ".state") mSlotNumber = 0;
  else if ( ext == ".auto") mIsAuto = true;
  else mSlotNumber = ext.Remove(".state").AsInt();
}

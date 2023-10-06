//
// Created by bkg2k on 16/01/2022.
//
// As part of the RECALBOX Project
// http://www.recalbox.com
//

#include <systems/SystemSorting.h>
#include <systems/SystemData.h>

int SortingName(SystemData* const& a, SystemData* const& b)
{
  return strcmp(a->FullName().data(), b->FullName().data());
}

int SortingReleaseDate(SystemData* const& a, SystemData* const& b)
{
  return (a->Descriptor().ReleaseDate() - b->Descriptor().ReleaseDate());
}

int Sorting1Type2Name(SystemData* const& a, SystemData* const& b)
{
  if (a->Descriptor().Type() == b->Descriptor().Type())
    return strcmp(a->FullName().data(), b->FullName().data());

  return ((int)a->Descriptor().Type() - (int)b->Descriptor().Type());
}

int Sorting1Type2ReleaseDate(SystemData* const& a, SystemData* const& b)
{
  if (a->Descriptor().Type() == b->Descriptor().Type())
    return a->Descriptor().ReleaseDate() - b->Descriptor().ReleaseDate();

  return ((int)a->Descriptor().Type() - (int)b->Descriptor().Type());
}

int Sorting1Manufacturer2Name(SystemData* const& a, SystemData* const& b)
{
  if (a->Descriptor().Manufacturer() == b->Descriptor().Manufacturer())
    return strcmp(a->FullName().data(), b->FullName().data());

  return strcmp(a->Descriptor().Manufacturer().data(), b->Descriptor().Manufacturer().data());
}

int Sorting1Manufacturer2ReleaseDate(SystemData* const& a, SystemData* const& b)
{
  if (a->Descriptor().Manufacturer() == b->Descriptor().Manufacturer())
  {
    return a->Descriptor().ReleaseDate() - b->Descriptor().ReleaseDate();
  }
  return strcmp(a->Descriptor().Manufacturer().data(), b->Descriptor().Manufacturer().data());
}

int Sorting1Type2Manufacturer3Name(SystemData* const& a, SystemData* const& b)
{
  if (a->Descriptor().Type() == b->Descriptor().Type())
  {
    if (a->Descriptor().Manufacturer() == a->Descriptor().Manufacturer())
      return strcmp(a->FullName().data(), b->FullName().data());
    return strcmp(a->Descriptor().Manufacturer().data(), b->Descriptor().Manufacturer().data());
  }
  return ((int)a->Descriptor().Type() - (int)b->Descriptor().Type());
}

int Sorting1Type2Manufacturer3ReleaseDate(SystemData* const& a, SystemData* const& b)
{
  if (a->Descriptor().Type() == b->Descriptor().Type())
  {
    if (a->Descriptor().Manufacturer() == b->Descriptor().Manufacturer())
      return a->Descriptor().ReleaseDate() - b->Descriptor().ReleaseDate();
    return strcmp(a->Descriptor().Manufacturer().data(), b->Descriptor().Manufacturer().data());
  }
  return ((int)a->Descriptor().Type() - (int)b->Descriptor().Type());
}

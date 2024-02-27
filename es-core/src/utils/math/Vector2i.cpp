#include "Vector2i.h"
#include "Vector2f.h"
#include "Misc.h"

Vector2f Vector2i::toFloat() const
{
  return { (float)mX, (float)mY };
}

Vector2i Vector2i::toInt() const
{
  return { Math::roundi(mX), Math::roundi(mY) };
}

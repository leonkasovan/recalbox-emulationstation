#include "DateTime.h"
#include "TimeSpan.h"

TimeSpan& TimeSpan::Build(long long ms)
{
  Units._Millis = (short)(ms % 1000LL); ms /= 1000LL;
  Units._Seconds = (char)(ms % 60LL); ms /= 60LL;
  Units._Minutes = (char)(ms % 60LL); ms /= 60LL;
  Units._Hours = (int)ms;
  return *this;
}

TimeSpan::TimeSpan(const DateTime& start, const DateTime& stop)
{
  long long epochStop = (stop.ToEpochTime()) * 1000LL + (long long)stop.Millisecond();
  long long epochStart = (start.ToEpochTime()) * 1000LL + (long long)start.Millisecond();
  Build(epochStop - epochStart);
}

String TimeSpan::ToStringFormat(const char* format) const
{
  String result;
  bool Escaped = false;

  for (int index = 0; format[index] != 0;)
  {
    char c = format[index];
    if (c == '\\')
    {
      Escaped = true;
      index++;
      continue;
    }
    if ((c != '%') || Escaped)
    {
      result += c;
      Escaped = false;
      index++;
      continue;
    }
    // c is '%', get first format char
    c = format[++index];
    int repeat = 0;
    if (format[++index] == c)
    {
      repeat++;
      if (format[++index] == c)
      {
        repeat++;
        if (format[++index] == c)
        {
          repeat++;
          index++;
        }
      }
    }
    switch (c)
    {
      case 'd':
      {
        if (repeat == 0) result.Append(Units._Hours / 24);
        break;
      }
      case 'H':
      {
        if (repeat == 0) result.Append(Units._Hours);
        else if (repeat == 1) { result += ((char) ('0' + (((Units._Hours % 24) / 10) % 10))); result += ((char) ('0' + ((Units._Hours % 24) % 10))); }
        break;
      }
      case 'm':
      {
        if (repeat == 0) result.Append(Units._Minutes);
        else if (repeat == 1) { result += ((char) ('0' + ((Units._Minutes / 10) % 10))); result += ((char) ('0' + (Units._Minutes % 10))); }
        else if (repeat == 3) result.Append(TotalMinutes());
        break;
      }
      case 's':
      {
        if (repeat == 0) result.Append(Units._Seconds);
        else if (repeat == 1) { result += ((char) ('0' + ((Units._Seconds / 10) % 10))); result += ((char) ('0' + (Units._Seconds % 10))); }
        else if (repeat == 3) result.Append(TotalSeconds());
        break;
      }
      case 'f':
      {
        if (repeat == 0) result.Append(Units._Millis);
        else if (repeat == 2) { result += ((char) ('0' + ((Units._Millis / 100) % 10))); result += ((char) ('0' + ((Units._Millis / 10) % 10))); result += ((char) ('0' + (Units._Millis % 10))); }
        else if (repeat == 3) result.Append(TotalMilliseconds());
        break;
      }
      case '%':
      {
        result.Append('%', repeat);
        break;
      }
      default:
      {
        result += ("<Unk:");
        result.Append(c, repeat + 1);
        result += ('>');
        break;
      }
    }
  }

  return result;
}


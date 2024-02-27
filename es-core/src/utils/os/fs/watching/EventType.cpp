#include "EventType.h"

String& operator <<(String& stream, const EventType& event)
{
  if (hasFlag(event, EventType::Access))       stream.Append("access ");
  if (hasFlag(event, EventType::Attrib))       stream.Append("attrib ");
  if (hasFlag(event, EventType::CloseWrite))   stream.Append("close_write ");
  if (hasFlag(event, EventType::CloseNowrite)) stream.Append("close_nowrite ");
  if (hasFlag(event, EventType::Create))       stream.Append("create ");
  if (hasFlag(event, EventType::Remove))       stream.Append("remove ");
  if (hasFlag(event, EventType::RemoveSelf))   stream.Append("remove_self ");
  if (hasFlag(event, EventType::Modify))       stream.Append("modify ");
  if (hasFlag(event, EventType::MoveSelf))     stream.Append("move_self ");
  if (hasFlag(event, EventType::MovedFrom))    stream.Append("moved_from ");
  if (hasFlag(event, EventType::MovedTo))      stream.Append("moved_to ");
  if (hasFlag(event, EventType::Open))         stream.Append("open ");
  if (hasFlag(event, EventType::IsDir))        stream.Append("is_dir ");
  if (hasFlag(event, EventType::Unmount))      stream.Append("unmount ");
  if (hasFlag(event, EventType::QOverflow))    stream.Append("q_overflow ");
  if (hasFlag(event, EventType::Close))        stream.Append("close ");
  if (hasFlag(event, EventType::Ignored))      stream.Append("ignored ");
  if (hasFlag(event, EventType::Oneshot))      stream.Append("oneshot ");

  return stream;
}

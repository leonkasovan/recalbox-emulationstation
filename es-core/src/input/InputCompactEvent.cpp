//
// Created by bkg2k on 28/10/2019.
//

#include "InputCompactEvent.h"
#include "InputDevice.h"
#include "SDL2/SDL.h"

bool InputCompactEvent::KeyDown() const
{
  return mInputDevice.IsKeyboard() && mInputEvent.Value() != 0;
}

bool InputCompactEvent::KeyUp() const
{
  return mInputDevice.IsKeyboard() && mInputEvent.Value() == 0;
}

bool InputCompactEvent::IsKeyboard() const
{
  return mInputDevice.IsKeyboard();
}

int InputCompactEvent::KeyCode() const
{
  return mInputDevice.IsKeyboard() ? mInputEvent.Id() : SDLK_UNKNOWN;
}

void InputCompactEvent::swap(Entry& entries, Entry left, Entry right)
{
  if ((((EntryType)entries & (EntryType)left) ^ ((EntryType)entries & (EntryType)right)) != 0)
  {
    entries ^= left;
    entries ^= right;
  }
}

void InputCompactEvent::Rotate()
{
  { LOG(LogDebug) << "[InputCompactEvent] Rotate CompactEvent"; }

  swap(mActivatedEntryFlags,   Entry::X,       Entry::Y);
  swap(mActivatedEntryFlags,   Entry::Y,       Entry::A);
  swap(mActivatedEntryFlags,   Entry::B,       Entry::Y);
  swap(mActivatedEntryFlags,   Entry::J2Left,  Entry::J2Down); // Down on Left
  swap(mActivatedEntryFlags,   Entry::J2Right, Entry::J2Up); // Up on Right
  swap(mActivatedEntryFlags,   Entry::J2Up,    Entry::J2Down);
  swap(mDeactivatedEntryFlags, Entry::X,       Entry::Y);
  swap(mDeactivatedEntryFlags, Entry::Y,       Entry::A);
  swap(mDeactivatedEntryFlags, Entry::B,       Entry::Y);
  swap(mDeactivatedEntryFlags, Entry::J2Left,  Entry::J2Down);
  swap(mDeactivatedEntryFlags, Entry::J2Right, Entry::J2Up);
  swap(mDeactivatedEntryFlags, Entry::J2Up,    Entry::J2Down);

  swap(mActivatedEntryFlags,   Entry::HotkeyX,       Entry::HotkeyY);
  swap(mActivatedEntryFlags,   Entry::HotkeyY,       Entry::HotkeyA);
  swap(mActivatedEntryFlags,   Entry::HotkeyB,       Entry::HotkeyY);
  swap(mActivatedEntryFlags,   Entry::HotkeyJ2Left,  Entry::HotkeyJ2Down); // Down on Left
  swap(mActivatedEntryFlags,   Entry::HotkeyJ2Right, Entry::HotkeyJ2Up); // Up on Right
  swap(mActivatedEntryFlags,   Entry::HotkeyJ2Up,    Entry::HotkeyJ2Down);
  swap(mDeactivatedEntryFlags, Entry::HotkeyX,       Entry::HotkeyY);
  swap(mDeactivatedEntryFlags, Entry::HotkeyY,       Entry::HotkeyA);
  swap(mDeactivatedEntryFlags, Entry::HotkeyB,       Entry::HotkeyY);
  swap(mDeactivatedEntryFlags, Entry::HotkeyJ2Left,  Entry::HotkeyJ2Down);
  swap(mDeactivatedEntryFlags, Entry::HotkeyJ2Right, Entry::HotkeyJ2Up);
  swap(mDeactivatedEntryFlags, Entry::HotkeyJ2Up,    Entry::HotkeyJ2Down);
}

String InputCompactEvent::ToString() const
{
  String pressed;
  String released;

  for(int i = 2; --i >= 0; )
  {
    Entry flags = (i != 0) ? mActivatedEntryFlags : mDeactivatedEntryFlags;
    String& result = (i != 0) ? pressed : released;
    if ((flags & Entry::Start            ) != 0) result.Append(',').Append("Start");
    if ((flags & Entry::Select           ) != 0) result.Append(',').Append("Select");
    if ((flags & Entry::Hotkey           ) != 0) result.Append(',').Append("Hotkey");
    if ((flags & Entry::A                ) != 0) result.Append(',').Append('A');
    if ((flags & Entry::B                ) != 0) result.Append(',').Append('B');
    if ((flags & Entry::X                ) != 0) result.Append(',').Append('X');
    if ((flags & Entry::Y                ) != 0) result.Append(',').Append('Y');
    if ((flags & Entry::L1               ) != 0) result.Append(',').Append("L1");
    if ((flags & Entry::R1               ) != 0) result.Append(',').Append("R1");
    if ((flags & Entry::L2               ) != 0) result.Append(',').Append("L2");
    if ((flags & Entry::R2               ) != 0) result.Append(',').Append("R2");
    if ((flags & Entry::L3               ) != 0) result.Append(',').Append("L3");
    if ((flags & Entry::R3               ) != 0) result.Append(',').Append("R3");
    if ((flags & Entry::Up               ) != 0) result.Append(',').Append("Up");
    if ((flags & Entry::Right            ) != 0) result.Append(',').Append("Right");
    if ((flags & Entry::Down             ) != 0) result.Append(',').Append("Down");
    if ((flags & Entry::Left             ) != 0) result.Append(',').Append("Left");
    if ((flags & Entry::J1Up             ) != 0) result.Append(',').Append("J1Up");
    if ((flags & Entry::J1Right          ) != 0) result.Append(',').Append("J1Right");
    if ((flags & Entry::J1Down           ) != 0) result.Append(',').Append("J1Down");
    if ((flags & Entry::J1Left           ) != 0) result.Append(',').Append("J1Left");
    if ((flags & Entry::J2Up             ) != 0) result.Append(',').Append("J2Up");
    if ((flags & Entry::J2Right          ) != 0) result.Append(',').Append("J2Right");
    if ((flags & Entry::J2Down           ) != 0) result.Append(',').Append("J2Down");
    if ((flags & Entry::J2Left           ) != 0) result.Append(',').Append("J2Left");
    if ((flags & Entry::VolUp            ) != 0) result.Append(',').Append("VolUp");
    if ((flags & Entry::VolDown          ) != 0) result.Append(',').Append("VolDown");
    if ((flags & Entry::LumUp            ) != 0) result.Append(',').Append("LumUp");
    if ((flags & Entry::LumDown          ) != 0) result.Append(',').Append("LumDown");
    if ((flags & Entry::NeedConfiguration) != 0) result.Append(',').Append("*NEED CONF*");
    if ((flags & Entry::HotkeyStart      ) != 0) result.Append(',').Append("Hotkey+Start");
    if ((flags & Entry::HotkeyA          ) != 0) result.Append(',').Append("Hotkey+A");
    if ((flags & Entry::HotkeyB          ) != 0) result.Append(',').Append("Hotkey+B");
    if ((flags & Entry::HotkeyX          ) != 0) result.Append(',').Append("Hotkey+X");
    if ((flags & Entry::HotkeyY          ) != 0) result.Append(',').Append("Hotkey+Y");
    if ((flags & Entry::HotkeyL1         ) != 0) result.Append(',').Append("Hotkey+L1");
    if ((flags & Entry::HotkeyR1         ) != 0) result.Append(',').Append("Hotkey+R1");
    if ((flags & Entry::HotkeyL2         ) != 0) result.Append(',').Append("Hotkey+L2");
    if ((flags & Entry::HotkeyR2         ) != 0) result.Append(',').Append("Hotkey+R2");
    if ((flags & Entry::HotkeyL3         ) != 0) result.Append(',').Append("Hotkey+L3");
    if ((flags & Entry::HotkeyR3         ) != 0) result.Append(',').Append("Hotkey+R3");
    if ((flags & Entry::HotkeyUp         ) != 0) result.Append(',').Append("Hotkey+Up");
    if ((flags & Entry::HotkeyRight      ) != 0) result.Append(',').Append("Hotkey+Right");
    if ((flags & Entry::HotkeyDown       ) != 0) result.Append(',').Append("Hotkey+Down");
    if ((flags & Entry::HotkeyLeft       ) != 0) result.Append(',').Append("Hotkey+Left");
    if ((flags & Entry::HotkeyJ1Up       ) != 0) result.Append(',').Append("Hotkey+J1Up");
    if ((flags & Entry::HotkeyJ1Right    ) != 0) result.Append(',').Append("Hotkey+J1Right");
    if ((flags & Entry::HotkeyJ1Down     ) != 0) result.Append(',').Append("Hotkey+J1Down");
    if ((flags & Entry::HotkeyJ1Left     ) != 0) result.Append(',').Append("Hotkey+J1Left");
    if ((flags & Entry::HotkeyJ2Up       ) != 0) result.Append(',').Append("Hotkey+J2Up");
    if ((flags & Entry::HotkeyJ2Right    ) != 0) result.Append(',').Append("Hotkey+J2Right");
    if ((flags & Entry::HotkeyJ2Down     ) != 0) result.Append(',').Append("Hotkey+J2Down");
    if ((flags & Entry::HotkeyJ2Left     ) != 0) result.Append(',').Append("Hotkey+J2Left");
    if ((flags & Entry::Reserved         ) != 0) result.Append(',').Append("*NEED CONF*");
  }

  return String("Pressed: ").Append(pressed).Append(" - Released: ").Append(released);
}

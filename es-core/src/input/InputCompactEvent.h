//
// Created by bkg2k on 28/10/2019.
//
#pragma once

#include <utils/cplusplus/Bitflags.h>
#include <input/InputEvent.h>
#include "RecalboxConf.h"

class InputDevice;

class InputCompactEvent
{
  public:
    typedef unsigned long long int EntryType;
    enum class Entry: EntryType
    {
      Nothing           = 0x0000000000000000,
      Start             = 0x0000000000000001,
      Select            = 0x0000000000000002,
      Hotkey            = 0x0000000000000004,
      A                 = 0x0000000000000008,
      B                 = 0x0000000000000010,
      X                 = 0x0000000000000020,
      Y                 = 0x0000000000000040,
      L1                = 0x0000000000000080,
      R1                = 0x0000000000000100,
      L2                = 0x0000000000000200,
      R2                = 0x0000000000000400,
      L3                = 0x0000000000000800,
      R3                = 0x0000000000001000,
      Up                = 0x0000000000002000,
      Right             = 0x0000000000004000,
      Down              = 0x0000000000008000,
      Left              = 0x0000000000010000,
      J1Left            = 0x0000000000020000,
      J1Right           = 0x0000000000040000,
      J1Up              = 0x0000000000080000,
      J1Down            = 0x0000000000100000,
      J2Left            = 0x0000000000200000,
      J2Right           = 0x0000000000400000,
      J2Up              = 0x0000000000800000,
      J2Down            = 0x0000000001000000,
      VolUp             = 0x0000000002000000,
      VolDown           = 0x0000000004000000,
      LumUp             = 0x0000000008000000,
      LumDown           = 0x0000000010000000,
      NeedConfiguration = 0x0000000080000000, //!< Special case: When activated, this entry means the source device is not configured yet!
      // Extended keys must have the same value then the original keys, shifted left by 32
      HotkeyStart       = 0x0000000100000000,
      InvalidSelect     = 0x0000000200000000,
      InvalidHotkey     = 0x0000000400000000,
      HotkeyA           = 0x0000000800000000,
      HotkeyB           = 0x0000001000000000,
      HotkeyX           = 0x0000002000000000,
      HotkeyY           = 0x0000004000000000,
      HotkeyL1          = 0x0000008000000000,
      HotkeyR1          = 0x0000010000000000,
      HotkeyL2          = 0x0000020000000000,
      HotkeyR2          = 0x0000040000000000,
      HotkeyL3          = 0x0000080000000000,
      HotkeyR3          = 0x0000100000000000,
      HotkeyUp          = 0x0000200000000000,
      HotkeyRight       = 0x0000400000000000,
      HotkeyDown        = 0x0000800000000000,
      HotkeyLeft        = 0x0001000000000000,
      HotkeyJ1Left      = 0x0002000000000000,
      HotkeyJ1Right     = 0x0004000000000000,
      HotkeyJ1Up        = 0x0008000000000000,
      HotkeyJ1Down      = 0x0010000000000000,
      HotkeyJ2Left      = 0x0020000000000000,
      HotkeyJ2Right     = 0x0040000000000000,
      HotkeyJ2Up        = 0x0080000000000000,
      HotkeyJ2Down      = 0x0100000000000000,
      Reserved          = 0x8000000000000000,

      AllSpecials       = (EntryType)Hotkey | (EntryType)Select | (EntryType)NeedConfiguration,
      HotkeyAllSpecials = (EntryType)InvalidHotkey | (EntryType)InvalidSelect | (EntryType)Reserved,
      HotkeyAll         = 0xFFFFFFFF00000000 ^ HotkeyAllSpecials,
    };

  private:
    Entry mActivatedEntryFlags;   //!< Entries activated by this event, as bitflag (button pressed, joy moved, ...)
    Entry mDeactivatedEntryFlags; //!< Entries deactivated by this event, as bitflag (button released, joy centerted, ...)
    int mElapsedTime;                    //!< Elapsed time for release events. 0 for press event
    InputDevice& mInputDevice;           //!< Source Device
    InputEvent   mInputEvent;            //!< Original source event

    static void swap(Entry& n, Entry p, Entry q);

  public:
    /*!
     * @brief Constructor
     * @param on Activated entries bitflag
     * @param off Deactivated entries bitflag
     * @param device Device instance from which the event is related to
     * @param originalEvent Original raw event
     */
    InputCompactEvent(Entry on, Entry off, int elapsed, InputDevice& device, const InputEvent& originalEvent)
      : mActivatedEntryFlags(on)
      , mDeactivatedEntryFlags(off)
      , mElapsedTime(elapsed)
      , mInputDevice(device)
      , mInputEvent(originalEvent)
    {
    }

    explicit InputCompactEvent(InputDevice& device)
      : mActivatedEntryFlags(Entry::Nothing)
      , mDeactivatedEntryFlags(Entry::Nothing)
      , mElapsedTime(0)
      , mInputDevice(device)
      , mInputEvent(0, InputEvent::EventType::Unknown, 0, 0, 0)
    {
    }

    /*
     * Source accessors
     */

          [[nodiscard]] InputDevice& Device()    const { return mInputDevice; }
    [[nodiscard]] const InputEvent&  RawEvent()  const { return mInputEvent; }

    /*
     * Special accessors & keyboard
     */

    [[nodiscard]] bool AskForConfiguration() const { return ((EntryType)mActivatedEntryFlags & (EntryType)Entry::NeedConfiguration) != 0; }
    [[nodiscard]] bool IsKeyboard()          const;
    [[nodiscard]] bool KeyDown()             const;
    [[nodiscard]] int  KeyCode()             const;
    [[nodiscard]] bool KeyUp()               const;

    /*
     * Accessors w/o state
     */

    [[nodiscard]] bool Up()                   const { return (((EntryType)mActivatedEntryFlags | (EntryType)mDeactivatedEntryFlags) & (EntryType)Entry::Up           ) != 0; }
    [[nodiscard]] bool Down()                 const { return (((EntryType)mActivatedEntryFlags | (EntryType)mDeactivatedEntryFlags) & (EntryType)Entry::Down         ) != 0; }
    [[nodiscard]] bool Left()                 const { return (((EntryType)mActivatedEntryFlags | (EntryType)mDeactivatedEntryFlags) & (EntryType)Entry::Left         ) != 0; }
    [[nodiscard]] bool Right()                const { return (((EntryType)mActivatedEntryFlags | (EntryType)mDeactivatedEntryFlags) & (EntryType)Entry::Right        ) != 0; }
    [[nodiscard]] bool A()                    const { return (((EntryType)mActivatedEntryFlags | (EntryType)mDeactivatedEntryFlags) & (EntryType)Entry::A            ) != 0; }
    [[nodiscard]] bool B()                    const { return (((EntryType)mActivatedEntryFlags | (EntryType)mDeactivatedEntryFlags) & (EntryType)Entry::B            ) != 0; }
    [[nodiscard]] bool X()                    const { return (((EntryType)mActivatedEntryFlags | (EntryType)mDeactivatedEntryFlags) & (EntryType)Entry::X            ) != 0; }
    [[nodiscard]] bool Y()                    const { return (((EntryType)mActivatedEntryFlags | (EntryType)mDeactivatedEntryFlags) & (EntryType)Entry::Y            ) != 0; }
    [[nodiscard]] bool Start()                const { return (((EntryType)mActivatedEntryFlags | (EntryType)mDeactivatedEntryFlags) & (EntryType)Entry::Start        ) != 0; }
    [[nodiscard]] bool Select()               const { return (((EntryType)mActivatedEntryFlags | (EntryType)mDeactivatedEntryFlags) & (EntryType)Entry::Select       ) != 0; }
    [[nodiscard]] bool Hotkey()               const { return (((EntryType)mActivatedEntryFlags | (EntryType)mDeactivatedEntryFlags) & (EntryType)Entry::Hotkey       ) != 0; }
    [[nodiscard]] bool L1()                   const { return (((EntryType)mActivatedEntryFlags | (EntryType)mDeactivatedEntryFlags) & (EntryType)Entry::L1           ) != 0; }
    [[nodiscard]] bool R1()                   const { return (((EntryType)mActivatedEntryFlags | (EntryType)mDeactivatedEntryFlags) & (EntryType)Entry::R1           ) != 0; }
    [[nodiscard]] bool L2()                   const { return (((EntryType)mActivatedEntryFlags | (EntryType)mDeactivatedEntryFlags) & (EntryType)Entry::L2           ) != 0; }
    [[nodiscard]] bool R2()                   const { return (((EntryType)mActivatedEntryFlags | (EntryType)mDeactivatedEntryFlags) & (EntryType)Entry::R2           ) != 0; }
    [[nodiscard]] bool L3()                   const { return (((EntryType)mActivatedEntryFlags | (EntryType)mDeactivatedEntryFlags) & (EntryType)Entry::L3           ) != 0; }
    [[nodiscard]] bool R3()                   const { return (((EntryType)mActivatedEntryFlags | (EntryType)mDeactivatedEntryFlags) & (EntryType)Entry::R3           ) != 0; }
    [[nodiscard]] bool J1Up()                 const { return (((EntryType)mActivatedEntryFlags | (EntryType)mDeactivatedEntryFlags) & (EntryType)Entry::J1Up         ) != 0; }
    [[nodiscard]] bool J1Down()               const { return (((EntryType)mActivatedEntryFlags | (EntryType)mDeactivatedEntryFlags) & (EntryType)Entry::J1Down       ) != 0; }
    [[nodiscard]] bool J1Left()               const { return (((EntryType)mActivatedEntryFlags | (EntryType)mDeactivatedEntryFlags) & (EntryType)Entry::J1Left       ) != 0; }
    [[nodiscard]] bool J1Right()              const { return (((EntryType)mActivatedEntryFlags | (EntryType)mDeactivatedEntryFlags) & (EntryType)Entry::J1Right      ) != 0; }
    [[nodiscard]] bool J2Up()                 const { return (((EntryType)mActivatedEntryFlags | (EntryType)mDeactivatedEntryFlags) & (EntryType)Entry::J2Up         ) != 0; }
    [[nodiscard]] bool J2Down()               const { return (((EntryType)mActivatedEntryFlags | (EntryType)mDeactivatedEntryFlags) & (EntryType)Entry::J2Down       ) != 0; }
    [[nodiscard]] bool J2Left()               const { return (((EntryType)mActivatedEntryFlags | (EntryType)mDeactivatedEntryFlags) & (EntryType)Entry::J2Left       ) != 0; }
    [[nodiscard]] bool J2Right()              const { return (((EntryType)mActivatedEntryFlags | (EntryType)mDeactivatedEntryFlags) & (EntryType)Entry::J2Right      ) != 0; }
    [[nodiscard]] bool VolumeUp()             const { return (((EntryType)mActivatedEntryFlags | (EntryType)mDeactivatedEntryFlags) & (EntryType)Entry::VolUp        ) != 0; }
    [[nodiscard]] bool VolumeDown()           const { return (((EntryType)mActivatedEntryFlags | (EntryType)mDeactivatedEntryFlags) & (EntryType)Entry::VolDown      ) != 0; }
    [[nodiscard]] bool BrightnessUp()         const { return (((EntryType)mActivatedEntryFlags | (EntryType)mDeactivatedEntryFlags) & (EntryType)Entry::LumUp        ) != 0; }
    [[nodiscard]] bool BrightnessDown()       const { return (((EntryType)mActivatedEntryFlags | (EntryType)mDeactivatedEntryFlags) & (EntryType)Entry::LumDown      ) != 0; }
    [[nodiscard]] bool HotkeyUp()             const { return (((EntryType)mActivatedEntryFlags | (EntryType)mDeactivatedEntryFlags) & (EntryType)Entry::HotkeyUp     ) != 0; }
    [[nodiscard]] bool HotkeyDown()           const { return (((EntryType)mActivatedEntryFlags | (EntryType)mDeactivatedEntryFlags) & (EntryType)Entry::HotkeyDown   ) != 0; }
    [[nodiscard]] bool HotkeyLeft()           const { return (((EntryType)mActivatedEntryFlags | (EntryType)mDeactivatedEntryFlags) & (EntryType)Entry::HotkeyLeft   ) != 0; }
    [[nodiscard]] bool HotkeyRight()          const { return (((EntryType)mActivatedEntryFlags | (EntryType)mDeactivatedEntryFlags) & (EntryType)Entry::HotkeyRight  ) != 0; }
    [[nodiscard]] bool HotkeyA()              const { return (((EntryType)mActivatedEntryFlags | (EntryType)mDeactivatedEntryFlags) & (EntryType)Entry::HotkeyA      ) != 0; }
    [[nodiscard]] bool HotkeyB()              const { return (((EntryType)mActivatedEntryFlags | (EntryType)mDeactivatedEntryFlags) & (EntryType)Entry::HotkeyB      ) != 0; }
    [[nodiscard]] bool HotkeyX()              const { return (((EntryType)mActivatedEntryFlags | (EntryType)mDeactivatedEntryFlags) & (EntryType)Entry::HotkeyX      ) != 0; }
    [[nodiscard]] bool HotkeyY()              const { return (((EntryType)mActivatedEntryFlags | (EntryType)mDeactivatedEntryFlags) & (EntryType)Entry::HotkeyY      ) != 0; }
    [[nodiscard]] bool HotkeyStart()          const { return (((EntryType)mActivatedEntryFlags | (EntryType)mDeactivatedEntryFlags) & (EntryType)Entry::HotkeyStart  ) != 0; }
    [[nodiscard]] bool HotkeyL1()             const { return (((EntryType)mActivatedEntryFlags | (EntryType)mDeactivatedEntryFlags) & (EntryType)Entry::HotkeyL1     ) != 0; }
    [[nodiscard]] bool HotkeyR1()             const { return (((EntryType)mActivatedEntryFlags | (EntryType)mDeactivatedEntryFlags) & (EntryType)Entry::HotkeyR1     ) != 0; }
    [[nodiscard]] bool HotkeyL2()             const { return (((EntryType)mActivatedEntryFlags | (EntryType)mDeactivatedEntryFlags) & (EntryType)Entry::HotkeyL2     ) != 0; }
    [[nodiscard]] bool HotkeyR2()             const { return (((EntryType)mActivatedEntryFlags | (EntryType)mDeactivatedEntryFlags) & (EntryType)Entry::HotkeyR2     ) != 0; }
    [[nodiscard]] bool HotkeyL3()             const { return (((EntryType)mActivatedEntryFlags | (EntryType)mDeactivatedEntryFlags) & (EntryType)Entry::HotkeyL3     ) != 0; }
    [[nodiscard]] bool HotkeyR3()             const { return (((EntryType)mActivatedEntryFlags | (EntryType)mDeactivatedEntryFlags) & (EntryType)Entry::HotkeyR3     ) != 0; }
    [[nodiscard]] bool HotkeyJ1Up()           const { return (((EntryType)mActivatedEntryFlags | (EntryType)mDeactivatedEntryFlags) & (EntryType)Entry::HotkeyJ1Up   ) != 0; }
    [[nodiscard]] bool HotkeyJ1Down()         const { return (((EntryType)mActivatedEntryFlags | (EntryType)mDeactivatedEntryFlags) & (EntryType)Entry::HotkeyJ1Down ) != 0; }
    [[nodiscard]] bool HotkeyJ1Left()         const { return (((EntryType)mActivatedEntryFlags | (EntryType)mDeactivatedEntryFlags) & (EntryType)Entry::HotkeyJ1Left ) != 0; }
    [[nodiscard]] bool HotkeyJ1Right()        const { return (((EntryType)mActivatedEntryFlags | (EntryType)mDeactivatedEntryFlags) & (EntryType)Entry::HotkeyJ1Right) != 0; }
    [[nodiscard]] bool HotkeyJ2Up()           const { return (((EntryType)mActivatedEntryFlags | (EntryType)mDeactivatedEntryFlags) & (EntryType)Entry::HotkeyJ2Up   ) != 0; }
    [[nodiscard]] bool HotkeyJ2Down()         const { return (((EntryType)mActivatedEntryFlags | (EntryType)mDeactivatedEntryFlags) & (EntryType)Entry::HotkeyJ2Down ) != 0; }
    [[nodiscard]] bool HotkeyJ2Left()         const { return (((EntryType)mActivatedEntryFlags | (EntryType)mDeactivatedEntryFlags) & (EntryType)Entry::HotkeyJ2Left ) != 0; }
    [[nodiscard]] bool HotkeyJ2Right()        const { return (((EntryType)mActivatedEntryFlags | (EntryType)mDeactivatedEntryFlags) & (EntryType)Entry::HotkeyJ2Right) != 0; }

    /*
     * Simple accessors w/ state
     */

    [[nodiscard]] bool UpPressed()                    const { return ((EntryType)mActivatedEntryFlags   & (EntryType)Entry::Up           ) != 0; }
    [[nodiscard]] bool UpReleased()                   const { return ((EntryType)mDeactivatedEntryFlags & (EntryType)Entry::Up           ) != 0; }
    [[nodiscard]] bool DownPressed()                  const { return ((EntryType)mActivatedEntryFlags   & (EntryType)Entry::Down         ) != 0; }
    [[nodiscard]] bool DownReleased()                 const { return ((EntryType)mDeactivatedEntryFlags & (EntryType)Entry::Down         ) != 0; }
    [[nodiscard]] bool LeftPressed()                  const { return ((EntryType)mActivatedEntryFlags   & (EntryType)Entry::Left         ) != 0; }
    [[nodiscard]] bool LeftReleased()                 const { return ((EntryType)mDeactivatedEntryFlags & (EntryType)Entry::Left         ) != 0; }
    [[nodiscard]] bool RightPressed()                 const { return ((EntryType)mActivatedEntryFlags   & (EntryType)Entry::Right        ) != 0; }
    [[nodiscard]] bool RightReleased()                const { return ((EntryType)mDeactivatedEntryFlags & (EntryType)Entry::Right        ) != 0; }
    [[nodiscard]] bool APressed()                     const { return ((EntryType)mActivatedEntryFlags   & (EntryType)Entry::A            ) != 0; }
    [[nodiscard]] bool AReleased()                    const { return ((EntryType)mDeactivatedEntryFlags & (EntryType)Entry::A            ) != 0; }
    [[nodiscard]] bool BPressed()                     const { return ((EntryType)mActivatedEntryFlags   & (EntryType)Entry::B            ) != 0; }
    [[nodiscard]] bool BReleased()                    const { return ((EntryType)mDeactivatedEntryFlags & (EntryType)Entry::B            ) != 0; }
    [[nodiscard]] bool XPressed()                     const { return ((EntryType)mActivatedEntryFlags   & (EntryType)Entry::X            ) != 0; }
    [[nodiscard]] bool XReleased()                    const { return ((EntryType)mDeactivatedEntryFlags & (EntryType)Entry::X            ) != 0; }
    [[nodiscard]] bool YPressed()                     const { return ((EntryType)mActivatedEntryFlags   & (EntryType)Entry::Y            ) != 0; }
    [[nodiscard]] bool YReleased()                    const { return ((EntryType)mDeactivatedEntryFlags & (EntryType)Entry::Y            ) != 0; }
    [[nodiscard]] bool StartPressed()                 const { return ((EntryType)mActivatedEntryFlags   & (EntryType)Entry::Start        ) != 0; }
    [[nodiscard]] bool StartReleased()                const { return ((EntryType)mDeactivatedEntryFlags & (EntryType)Entry::Start        ) != 0; }
    [[nodiscard]] bool SelectPressed()                const { return ((EntryType)mActivatedEntryFlags   & (EntryType)Entry::Select       ) != 0; }
    [[nodiscard]] bool SelectReleased()               const { return ((EntryType)mDeactivatedEntryFlags & (EntryType)Entry::Select       ) != 0; }
    [[nodiscard]] bool HotkeyPressed()                const { return ((EntryType)mActivatedEntryFlags   & (EntryType)Entry::Hotkey       ) != 0; }
    [[nodiscard]] bool HotkeyReleased()               const { return ((EntryType)mDeactivatedEntryFlags & (EntryType)Entry::Hotkey       ) != 0; }
    [[nodiscard]] bool L1Pressed()                    const { return ((EntryType)mActivatedEntryFlags   & (EntryType)Entry::L1           ) != 0; }
    [[nodiscard]] bool L1Released()                   const { return ((EntryType)mDeactivatedEntryFlags & (EntryType)Entry::L1           ) != 0; }
    [[nodiscard]] bool R1Pressed()                    const { return ((EntryType)mActivatedEntryFlags   & (EntryType)Entry::R1           ) != 0; }
    [[nodiscard]] bool R1Released()                   const { return ((EntryType)mDeactivatedEntryFlags & (EntryType)Entry::R1           ) != 0; }
    [[nodiscard]] bool L2Pressed()                    const { return ((EntryType)mActivatedEntryFlags   & (EntryType)Entry::L2           ) != 0; }
    [[nodiscard]] bool L2Released()                   const { return ((EntryType)mDeactivatedEntryFlags & (EntryType)Entry::L2           ) != 0; }
    [[nodiscard]] bool R2Pressed()                    const { return ((EntryType)mActivatedEntryFlags   & (EntryType)Entry::R2           ) != 0; }
    [[nodiscard]] bool R2Released()                   const { return ((EntryType)mDeactivatedEntryFlags & (EntryType)Entry::R2           ) != 0; }
    [[nodiscard]] bool L3Pressed()                    const { return ((EntryType)mActivatedEntryFlags   & (EntryType)Entry::L3           ) != 0; }
    [[nodiscard]] bool L3Released()                   const { return ((EntryType)mDeactivatedEntryFlags & (EntryType)Entry::L3           ) != 0; }
    [[nodiscard]] bool R3Pressed()                    const { return ((EntryType)mActivatedEntryFlags   & (EntryType)Entry::R3           ) != 0; }
    [[nodiscard]] bool R3Released()                   const { return ((EntryType)mDeactivatedEntryFlags & (EntryType)Entry::R3           ) != 0; }
    [[nodiscard]] bool J1UpPressed()                  const { return ((EntryType)mActivatedEntryFlags   & (EntryType)Entry::J1Up         ) != 0; }
    [[nodiscard]] bool J1UpReleased()                 const { return ((EntryType)mDeactivatedEntryFlags & (EntryType)Entry::J1Up         ) != 0; }
    [[nodiscard]] bool J1DownPressed()                const { return ((EntryType)mActivatedEntryFlags   & (EntryType)Entry::J1Down       ) != 0; }
    [[nodiscard]] bool J1DownReleased()               const { return ((EntryType)mDeactivatedEntryFlags & (EntryType)Entry::J1Down       ) != 0; }
    [[nodiscard]] bool J1LeftPressed()                const { return ((EntryType)mActivatedEntryFlags   & (EntryType)Entry::J1Left       ) != 0; }
    [[nodiscard]] bool J1LeftReleased()               const { return ((EntryType)mDeactivatedEntryFlags & (EntryType)Entry::J1Left       ) != 0; }
    [[nodiscard]] bool J1RightPressed()               const { return ((EntryType)mActivatedEntryFlags   & (EntryType)Entry::J1Right      ) != 0; }
    [[nodiscard]] bool J1RightReleased()              const { return ((EntryType)mDeactivatedEntryFlags & (EntryType)Entry::J1Right      ) != 0; }
    [[nodiscard]] bool J2UpPressed()                  const { return ((EntryType)mActivatedEntryFlags   & (EntryType)Entry::J2Up         ) != 0; }
    [[nodiscard]] bool J2UpReleased()                 const { return ((EntryType)mDeactivatedEntryFlags & (EntryType)Entry::J2Up         ) != 0; }
    [[nodiscard]] bool J2DownPressed()                const { return ((EntryType)mActivatedEntryFlags   & (EntryType)Entry::J2Down       ) != 0; }
    [[nodiscard]] bool J2DownReleased()               const { return ((EntryType)mDeactivatedEntryFlags & (EntryType)Entry::J2Down       ) != 0; }
    [[nodiscard]] bool J2LeftPressed()                const { return ((EntryType)mActivatedEntryFlags   & (EntryType)Entry::J2Left       ) != 0; }
    [[nodiscard]] bool J2LeftReleased()               const { return ((EntryType)mDeactivatedEntryFlags & (EntryType)Entry::J2Left       ) != 0; }
    [[nodiscard]] bool J2RightPressed()               const { return ((EntryType)mActivatedEntryFlags   & (EntryType)Entry::J2Right      ) != 0; }
    [[nodiscard]] bool J2RightReleased()              const { return ((EntryType)mDeactivatedEntryFlags & (EntryType)Entry::J2Right      ) != 0; }
    [[nodiscard]] bool VolumeUpPressed()              const { return ((EntryType)mActivatedEntryFlags   & (EntryType)Entry::VolUp        ) != 0; }
    [[nodiscard]] bool VolumeUpReleased()             const { return ((EntryType)mDeactivatedEntryFlags & (EntryType)Entry::VolUp        ) != 0; }
    [[nodiscard]] bool VolumeDownPressed()            const { return ((EntryType)mActivatedEntryFlags   & (EntryType)Entry::VolDown      ) != 0; }
    [[nodiscard]] bool VolumeDownReleased()           const { return ((EntryType)mDeactivatedEntryFlags & (EntryType)Entry::VolDown      ) != 0; }
    [[nodiscard]] bool BrightnessUpPressed()          const { return ((EntryType)mActivatedEntryFlags   & (EntryType)Entry::LumUp        ) != 0; }
    [[nodiscard]] bool BrightnessUpReleased()         const { return ((EntryType)mDeactivatedEntryFlags & (EntryType)Entry::LumUp        ) != 0; }
    [[nodiscard]] bool BrightnessDownPressed()        const { return ((EntryType)mActivatedEntryFlags   & (EntryType)Entry::LumDown      ) != 0; }
    [[nodiscard]] bool BrightnessDownReleased()       const { return ((EntryType)mDeactivatedEntryFlags & (EntryType)Entry::LumDown      ) != 0; }
    [[nodiscard]] bool HotkeyUpPressed()              const { return ((EntryType)mActivatedEntryFlags   & (EntryType)Entry::HotkeyUp     ) != 0; }
    [[nodiscard]] bool HotkeyUpReleased()             const { return ((EntryType)mDeactivatedEntryFlags & (EntryType)Entry::HotkeyUp     ) != 0; }
    [[nodiscard]] bool HotkeyDownPressed()            const { return ((EntryType)mActivatedEntryFlags   & (EntryType)Entry::HotkeyDown   ) != 0; }
    [[nodiscard]] bool HotkeyDownReleased()           const { return ((EntryType)mDeactivatedEntryFlags & (EntryType)Entry::HotkeyDown   ) != 0; }
    [[nodiscard]] bool HotkeyLeftPressed()            const { return ((EntryType)mActivatedEntryFlags   & (EntryType)Entry::HotkeyLeft   ) != 0; }
    [[nodiscard]] bool HotkeyLeftReleased()           const { return ((EntryType)mDeactivatedEntryFlags & (EntryType)Entry::HotkeyLeft   ) != 0; }
    [[nodiscard]] bool HotkeyRightPressed()           const { return ((EntryType)mActivatedEntryFlags   & (EntryType)Entry::HotkeyRight  ) != 0; }
    [[nodiscard]] bool HotkeyRightReleased()          const { return ((EntryType)mDeactivatedEntryFlags & (EntryType)Entry::HotkeyRight  ) != 0; }
    [[nodiscard]] bool HotkeyAPressed()               const { return ((EntryType)mActivatedEntryFlags   & (EntryType)Entry::HotkeyA      ) != 0; }
    [[nodiscard]] bool HotkeyAReleased()              const { return ((EntryType)mDeactivatedEntryFlags & (EntryType)Entry::HotkeyA      ) != 0; }
    [[nodiscard]] bool HotkeyBPressed()               const { return ((EntryType)mActivatedEntryFlags   & (EntryType)Entry::HotkeyB      ) != 0; }
    [[nodiscard]] bool HotkeyBReleased()              const { return ((EntryType)mDeactivatedEntryFlags & (EntryType)Entry::HotkeyB      ) != 0; }
    [[nodiscard]] bool HotkeyXPressed()               const { return ((EntryType)mActivatedEntryFlags   & (EntryType)Entry::HotkeyX      ) != 0; }
    [[nodiscard]] bool HotkeyXReleased()              const { return ((EntryType)mDeactivatedEntryFlags & (EntryType)Entry::HotkeyX      ) != 0; }
    [[nodiscard]] bool HotkeyYPressed()               const { return ((EntryType)mActivatedEntryFlags   & (EntryType)Entry::HotkeyY      ) != 0; }
    [[nodiscard]] bool HotkeyYReleased()              const { return ((EntryType)mDeactivatedEntryFlags & (EntryType)Entry::HotkeyY      ) != 0; }
    [[nodiscard]] bool HotkeyStartPressed()           const { return ((EntryType)mActivatedEntryFlags   & (EntryType)Entry::HotkeyStart  ) != 0; }
    [[nodiscard]] bool HotkeyStartReleased()          const { return ((EntryType)mDeactivatedEntryFlags & (EntryType)Entry::HotkeyStart  ) != 0; }
    [[nodiscard]] bool HotkeyL1Pressed()              const { return ((EntryType)mActivatedEntryFlags   & (EntryType)Entry::HotkeyL1     ) != 0; }
    [[nodiscard]] bool HotkeyL1Released()             const { return ((EntryType)mDeactivatedEntryFlags & (EntryType)Entry::HotkeyL1     ) != 0; }
    [[nodiscard]] bool HotkeyR1Pressed()              const { return ((EntryType)mActivatedEntryFlags   & (EntryType)Entry::HotkeyR1     ) != 0; }
    [[nodiscard]] bool HotkeyR1Released()             const { return ((EntryType)mDeactivatedEntryFlags & (EntryType)Entry::HotkeyR1     ) != 0; }
    [[nodiscard]] bool HotkeyL2Pressed()              const { return ((EntryType)mActivatedEntryFlags   & (EntryType)Entry::HotkeyL2     ) != 0; }
    [[nodiscard]] bool HotkeyL2Released()             const { return ((EntryType)mDeactivatedEntryFlags & (EntryType)Entry::HotkeyL2     ) != 0; }
    [[nodiscard]] bool HotkeyR2Pressed()              const { return ((EntryType)mActivatedEntryFlags   & (EntryType)Entry::HotkeyR2     ) != 0; }
    [[nodiscard]] bool HotkeyR2Released()             const { return ((EntryType)mDeactivatedEntryFlags & (EntryType)Entry::HotkeyR2     ) != 0; }
    [[nodiscard]] bool HotkeyL3Pressed()              const { return ((EntryType)mActivatedEntryFlags   & (EntryType)Entry::HotkeyL3     ) != 0; }
    [[nodiscard]] bool HotkeyL3Released()             const { return ((EntryType)mDeactivatedEntryFlags & (EntryType)Entry::HotkeyL3     ) != 0; }
    [[nodiscard]] bool HotkeyR3Pressed()              const { return ((EntryType)mActivatedEntryFlags   & (EntryType)Entry::HotkeyR3     ) != 0; }
    [[nodiscard]] bool HotkeyR3Released()             const { return ((EntryType)mDeactivatedEntryFlags & (EntryType)Entry::HotkeyR3     ) != 0; }
    [[nodiscard]] bool HotkeyJ1UpPressed()            const { return ((EntryType)mActivatedEntryFlags   & (EntryType)Entry::HotkeyJ1Up   ) != 0; }
    [[nodiscard]] bool HotkeyJ1UpReleased()           const { return ((EntryType)mDeactivatedEntryFlags & (EntryType)Entry::HotkeyJ1Up   ) != 0; }
    [[nodiscard]] bool HotkeyJ1DownPressed()          const { return ((EntryType)mActivatedEntryFlags   & (EntryType)Entry::HotkeyJ1Down ) != 0; }
    [[nodiscard]] bool HotkeyJ1DownReleased()         const { return ((EntryType)mDeactivatedEntryFlags & (EntryType)Entry::HotkeyJ1Down ) != 0; }
    [[nodiscard]] bool HotkeyJ1LeftPressed()          const { return ((EntryType)mActivatedEntryFlags   & (EntryType)Entry::HotkeyJ1Left ) != 0; }
    [[nodiscard]] bool HotkeyJ1LeftReleased()         const { return ((EntryType)mDeactivatedEntryFlags & (EntryType)Entry::HotkeyJ1Left ) != 0; }
    [[nodiscard]] bool HotkeyJ1RightPressed()         const { return ((EntryType)mActivatedEntryFlags   & (EntryType)Entry::HotkeyJ1Right) != 0; }
    [[nodiscard]] bool HotkeyJ1RightReleased()        const { return ((EntryType)mDeactivatedEntryFlags & (EntryType)Entry::HotkeyJ1Right) != 0; }
    [[nodiscard]] bool HotkeyJ2UpPressed()            const { return ((EntryType)mActivatedEntryFlags   & (EntryType)Entry::HotkeyJ2Up   ) != 0; }
    [[nodiscard]] bool HotkeyJ2UpReleased()           const { return ((EntryType)mDeactivatedEntryFlags & (EntryType)Entry::HotkeyJ2Up   ) != 0; }
    [[nodiscard]] bool HotkeyJ2DownPressed()          const { return ((EntryType)mActivatedEntryFlags   & (EntryType)Entry::HotkeyJ2Down ) != 0; }
    [[nodiscard]] bool HotkeyJ2DownReleased()         const { return ((EntryType)mDeactivatedEntryFlags & (EntryType)Entry::HotkeyJ2Down ) != 0; }
    [[nodiscard]] bool HotkeyJ2LeftPressed()          const { return ((EntryType)mActivatedEntryFlags   & (EntryType)Entry::HotkeyJ2Left ) != 0; }
    [[nodiscard]] bool HotkeyJ2LeftReleased()         const { return ((EntryType)mDeactivatedEntryFlags & (EntryType)Entry::HotkeyJ2Left ) != 0; }
    [[nodiscard]] bool HotkeyJ2RightPressed()         const { return ((EntryType)mActivatedEntryFlags   & (EntryType)Entry::HotkeyJ2Right) != 0; }
    [[nodiscard]] bool HotkeyJ2RightReleased()        const { return ((EntryType)mDeactivatedEntryFlags & (EntryType)Entry::HotkeyJ2Right) != 0; }

    /*
     * Meta accessors
     */

    [[nodiscard]] bool AnyHotkeyCombination()    const { return (((EntryType)mActivatedEntryFlags | (EntryType)mDeactivatedEntryFlags) & (EntryType)Entry::HotkeyAll) != 0; }

    [[nodiscard]] bool AnyUp()    const { return (((EntryType)mActivatedEntryFlags | (EntryType)mDeactivatedEntryFlags) & ((EntryType)Entry::Up    | (EntryType)Entry::J1Up    | (EntryType)Entry::J2Up   )) != 0; }
    [[nodiscard]] bool AnyDown()  const { return (((EntryType)mActivatedEntryFlags | (EntryType)mDeactivatedEntryFlags) & ((EntryType)Entry::Down  | (EntryType)Entry::J1Down  | (EntryType)Entry::J2Down )) != 0; }
    [[nodiscard]] bool AnyLeft()  const { return (((EntryType)mActivatedEntryFlags | (EntryType)mDeactivatedEntryFlags) & ((EntryType)Entry::Left  | (EntryType)Entry::J1Left  | (EntryType)Entry::J2Left )) != 0; }
    [[nodiscard]] bool AnyRight() const { return (((EntryType)mActivatedEntryFlags | (EntryType)mDeactivatedEntryFlags) & ((EntryType)Entry::Right | (EntryType)Entry::J1Right | (EntryType)Entry::J2Right)) != 0; }

    [[nodiscard]] bool AnyPrimaryUp()    const { return (((EntryType)mActivatedEntryFlags | (EntryType)mDeactivatedEntryFlags) & ((EntryType)Entry::Up    | (EntryType)Entry::J1Up   )) != 0; }
    [[nodiscard]] bool AnyPrimaryDown()  const { return (((EntryType)mActivatedEntryFlags | (EntryType)mDeactivatedEntryFlags) & ((EntryType)Entry::Down  | (EntryType)Entry::J1Down )) != 0; }
    [[nodiscard]] bool AnyPrimaryLeft()  const { return (((EntryType)mActivatedEntryFlags | (EntryType)mDeactivatedEntryFlags) & ((EntryType)Entry::Left  | (EntryType)Entry::J1Left )) != 0; }
    [[nodiscard]] bool AnyPrimaryRight() const { return (((EntryType)mActivatedEntryFlags | (EntryType)mDeactivatedEntryFlags) & ((EntryType)Entry::Right | (EntryType)Entry::J1Right)) != 0; }

    [[nodiscard]] bool AnyUpPressed()     const { return ((EntryType)mActivatedEntryFlags   & ((EntryType)Entry::Up    | (EntryType)Entry::J1Up    | (EntryType)Entry::J2Up   )) != 0; }
    [[nodiscard]] bool AnyUpReleased()    const { return ((EntryType)mDeactivatedEntryFlags & ((EntryType)Entry::Up    | (EntryType)Entry::J1Up    | (EntryType)Entry::J2Up   )) != 0; }
    [[nodiscard]] bool AnyDownPressed()   const { return ((EntryType)mActivatedEntryFlags   & ((EntryType)Entry::Down  | (EntryType)Entry::J1Down  | (EntryType)Entry::J2Down )) != 0; }
    [[nodiscard]] bool AnyDownReleased()  const { return ((EntryType)mDeactivatedEntryFlags & ((EntryType)Entry::Down  | (EntryType)Entry::J1Down  | (EntryType)Entry::J2Down )) != 0; }
    [[nodiscard]] bool AnyLeftPressed()   const { return ((EntryType)mActivatedEntryFlags   & ((EntryType)Entry::Left  | (EntryType)Entry::J1Left  | (EntryType)Entry::J2Left )) != 0; }
    [[nodiscard]] bool AnyLeftReleased()  const { return ((EntryType)mDeactivatedEntryFlags & ((EntryType)Entry::Left  | (EntryType)Entry::J1Left  | (EntryType)Entry::J2Left )) != 0; }
    [[nodiscard]] bool AnyRightPressed()  const { return ((EntryType)mActivatedEntryFlags   & ((EntryType)Entry::Right | (EntryType)Entry::J1Right | (EntryType)Entry::J2Right)) != 0; }
    [[nodiscard]] bool AnyRightReleased() const { return ((EntryType)mDeactivatedEntryFlags & ((EntryType)Entry::Right | (EntryType)Entry::J1Right | (EntryType)Entry::J2Right)) != 0; }

    [[nodiscard]] bool AnyPrimaryUpPressed()     const { return ((EntryType)mActivatedEntryFlags   & ((EntryType)Entry::Up    | (EntryType)Entry::J1Up   )) != 0; }
    [[nodiscard]] bool AnyPrimaryUpReleased()    const { return ((EntryType)mDeactivatedEntryFlags & ((EntryType)Entry::Up    | (EntryType)Entry::J1Up   )) != 0; }
    [[nodiscard]] bool AnyPrimaryDownPressed()   const { return ((EntryType)mActivatedEntryFlags   & ((EntryType)Entry::Down  | (EntryType)Entry::J1Down )) != 0; }
    [[nodiscard]] bool AnyPrimaryDownReleased()  const { return ((EntryType)mDeactivatedEntryFlags & ((EntryType)Entry::Down  | (EntryType)Entry::J1Down )) != 0; }
    [[nodiscard]] bool AnyPrimaryLeftPressed()   const { return ((EntryType)mActivatedEntryFlags   & ((EntryType)Entry::Left  | (EntryType)Entry::J1Left )) != 0; }
    [[nodiscard]] bool AnyPrimaryLeftReleased()  const { return ((EntryType)mDeactivatedEntryFlags & ((EntryType)Entry::Left  | (EntryType)Entry::J1Left )) != 0; }
    [[nodiscard]] bool AnyPrimaryRightPressed()  const { return ((EntryType)mActivatedEntryFlags   & ((EntryType)Entry::Right | (EntryType)Entry::J1Right)) != 0; }
    [[nodiscard]] bool AnyPrimaryRightReleased() const { return ((EntryType)mDeactivatedEntryFlags & ((EntryType)Entry::Right | (EntryType)Entry::J1Right)) != 0; }

    [[nodiscard]] bool AnyJ1()            const { return (((EntryType)mActivatedEntryFlags | (EntryType)mDeactivatedEntryFlags) & ((EntryType)Entry::J1Up | (EntryType)Entry::J1Down | (EntryType)Entry::J1Left | (EntryType)Entry::J1Right)) != 0; }
    [[nodiscard]] bool AnyJ1Pressed()     const { return ((EntryType)mActivatedEntryFlags                                       & ((EntryType)Entry::J1Up | (EntryType)Entry::J1Down | (EntryType)Entry::J1Left | (EntryType)Entry::J1Right)) != 0; }
    [[nodiscard]] bool AnyJ1Released()    const { return (                                   (EntryType)mDeactivatedEntryFlags  & ((EntryType)Entry::J1Up | (EntryType)Entry::J1Down | (EntryType)Entry::J1Left | (EntryType)Entry::J1Right)) != 0; }
    [[nodiscard]] bool AnyJ2()            const { return (((EntryType)mActivatedEntryFlags | (EntryType)mDeactivatedEntryFlags) & ((EntryType)Entry::J2Up | (EntryType)Entry::J2Down | (EntryType)Entry::J2Left | (EntryType)Entry::J2Right)) != 0; }
    [[nodiscard]] bool AnyJ2Pressed()     const { return ((EntryType)mActivatedEntryFlags                                       & ((EntryType)Entry::J2Up | (EntryType)Entry::J2Down | (EntryType)Entry::J2Left | (EntryType)Entry::J2Right)) != 0; }
    [[nodiscard]] bool AnyJ2Released()    const { return (                                   (EntryType)mDeactivatedEntryFlags  & ((EntryType)Entry::J2Up | (EntryType)Entry::J2Down | (EntryType)Entry::J2Left | (EntryType)Entry::J2Right)) != 0; }

    [[nodiscard]] bool AnythingPressed()  const { return (EntryType)mActivatedEntryFlags   != 0; }
    [[nodiscard]] bool AnythingReleased() const { return (EntryType)mDeactivatedEntryFlags != 0; }

    [[nodiscard]] bool AnyButtonPressed()  const { return ((EntryType)mActivatedEntryFlags   & ((EntryType)Entry::A | (EntryType)Entry::B | (EntryType)Entry::X | (EntryType)Entry::Y | (EntryType)Entry::R1 | (EntryType)Entry::L1 | (EntryType)Entry::R2 | (EntryType)Entry::L2 | (EntryType)Entry::Start | (EntryType)Entry::Select | (EntryType)Entry::VolUp | (EntryType)Entry::VolDown | (EntryType)Entry::LumDown | (EntryType)Entry::LumUp)) != 0; }
    [[nodiscard]] bool AnyButtonReleased() const { return ((EntryType)mDeactivatedEntryFlags & ((EntryType)Entry::A | (EntryType)Entry::B | (EntryType)Entry::X | (EntryType)Entry::Y | (EntryType)Entry::R1 | (EntryType)Entry::L1 | (EntryType)Entry::R2 | (EntryType)Entry::L2 | (EntryType)Entry::Start | (EntryType)Entry::Select | (EntryType)Entry::VolUp | (EntryType)Entry::VolDown | (EntryType)Entry::LumDown | (EntryType)Entry::LumUp)) != 0; }

    [[nodiscard]] bool Empty() const { return (((EntryType)mDeactivatedEntryFlags | (EntryType)mActivatedEntryFlags) == 0) && (!KeyDown() && !KeyUp()); }

    [[nodiscard]] bool ValidPressed() const { return RecalboxConf::Instance().GetSwapValidateAndCancel() ? APressed() : BPressed(); }
    [[nodiscard]] bool ValidReleased() const { return RecalboxConf::Instance().GetSwapValidateAndCancel() ? AReleased() : BReleased(); }
    [[nodiscard]] bool CancelPressed() const { return RecalboxConf::Instance().GetSwapValidateAndCancel() ? BPressed() : APressed(); }
    [[nodiscard]] bool CancelReleased() const { return RecalboxConf::Instance().GetSwapValidateAndCancel() ? BReleased() : AReleased(); }

    /*
     * Elapsed time
     */

    [[nodiscard]] int ElapsedMs() const { return mElapsedTime; }

    /*
     * Debug
     */

    [[nodiscard]] String ToString() const;

    /*!
     * @brief Rotate the current event
     */
    void Rotate();
};

DEFINE_BITFLAG_ENUM(InputCompactEvent::Entry, InputCompactEvent::EntryType)

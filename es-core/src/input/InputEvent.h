//
// Created by bkg2k on 28/10/2019.
//
#pragma once

#include <utils/String.h>
#include "utils/String.h"

class InputEvent
{
  public:
    //! Event type
    enum class EventType: int
    {
      Unknown,     //!< Unknown/Not initialized event
      Axis,        //!< Analog axis with values from -32768 to +32767
      Button,      //!< Button with binary state
      Hat,         //!< DPAD bitflag
      Key,         //!< Keyboard key
      MouseButton, //!< Mouse button
      MouseWheel,  //!< Mouse wheel
    };

  private:
    int       mDeviceIdentifier; //!< SDL Device Identifier or -1 for keyboard
    EventType mType;             //!< Event type
    int       mId;               //!< Event identifier
    int       mValue;            //!< Event value - type dependent
    int       mCode;             //!< SDL Raw code

  public:
    //! Define virtual default mousse device
    static constexpr int sMouseDevice = -1;
    //! Define virtual default keyboard device
    static constexpr int sKeyboardDevice = -2;
    //! Define empty device device
    static constexpr int sEmptyDevice = -3;

    /*!
     * @brief Default constructor
     */
    InputEvent()
      : mDeviceIdentifier(sKeyboardDevice),
        mType(EventType::Unknown),
        mId(-1),
        mValue(-999),
        mCode(-1)
    {
    }

    /*!
     * @brief Full constructor. Build an immutable Inpout event object
     * @param dev Device index
     * @param t Type
     * @param i Identifier
     * @param val Value
     * @param conf True if the input event is configured
     */
    InputEvent(int dev, EventType type, int id, int val)
      : mDeviceIdentifier(dev),
        mType(type),
        mId(id),
        mValue(val),
        mCode(-1)
    {
    }

    /*!
     * @brief Full constructor. Build an immutable Inpout event object
     * @param dev Device index
     * @param t Type
     * @param i Identifier
     * @param val Value
     * @param conf True if the input event is configured
     */
    InputEvent(int dev, EventType type, int id, int val, int code)
      : mDeviceIdentifier(dev),
        mType(type),
        mId(id),
        mValue(val),
        mCode(code)
    {
    }

    /*
     * Accessors
     */

    [[nodiscard]] int Device()     const { return mDeviceIdentifier; }
    [[nodiscard]] EventType Type() const { return mType; }
    [[nodiscard]] int Id()         const { return mId; }
    [[nodiscard]] int Value()      const { return mValue; }
    [[nodiscard]] int Code()       const { return mCode; }

    /*
     * Special accessor for configuration convenience
     */

    [[nodiscard]] bool AnyButtonPressed() const { return (mType == EventType::Button) && (mValue != 0); }
    [[nodiscard]] bool AnyButtonReleased() const { return (mType == EventType::Button) && (mValue == 0); }
    [[nodiscard]] bool AnythingPressed() const { return (mValue != 0); }
    [[nodiscard]] bool AnythingReleased() const { return (mValue == 0); }

    /*!
     * @brief Convert current event to a string representation
     * @return String representation of the c urrent event
     */
    [[nodiscard]] String ToString() const;

    /*!
     * @brief Get the raw SDL2 code for the current event.
     * Some emulators require raw code in their configurations
     */
    void StoreSDLCode(int deviceIndex);

    [[nodiscard]] bool EqualsTo(const InputEvent& to) const
    {
      return mDeviceIdentifier == to.mDeviceIdentifier &&
             mType == to.mType &&
             mId == to.mId &&
             mValue == to.mValue;
    }

    /*
     * Helpers
     */

    static String TypeToString(InputEvent::EventType type);
    static EventType StringToType(const String& type);
    static String HatDir(int val);
};

//
// Created by bkg2k on 15/01/2021.
//
// As part of the RECALBOX Project
// http://www.recalbox.com
//
#pragma once

#include <vector>
#include <utils/String.h>
#include <input/Input.h>
#include <input/IInputChange.h>
#include <utils/math/Misc.h>
#include <utils/os/fs/Path.h>
#include "utils/cplusplus/INoCopy.h"

// Forward declaration
class InputManager;

class InputMapper : public IInputChange
                  , public INoCopy
{
  public:
    //! Pad structure
    struct Pad
    {
      String mName;     //!< Real pad name
      String mUUID;     //!< Pad uuid
      Path   mPath;     //! /dev/input/eventX
      int    mIndex;    //!< Pad index in InputManager list
      int    mPosition; //!< Position in the mapper array

      Pad() : mIndex(-1), mPosition(-1) {}

      Pad(const String& name, const String& uuid, const Path& path, int inputIndex)
        : mName(name)
        , mUUID(uuid)
        , mPath(path)
        , mIndex(inputIndex)
        , mPosition(-1)
      {
      }

      void Set(const String& name, const String& uuid, const Path& path, int inputIndex)
      {
        mName = name;
        mUUID = uuid;
        mPath = path;
        mIndex = inputIndex;
        mPosition = -1;
      }

      void Reset()
      {
        mName.clear();
        mUUID.clear();
        mPath = Path();
        mIndex = -1;
        mPosition = -1;
      }

      [[nodiscard]] String LookupPowerLevel() const;

      [[nodiscard]] bool IsValid() const { return !mName.empty() && !mUUID.empty(); }

      [[nodiscard]] bool IsConnected() const { return !mName.empty() && !mUUID.empty() && mIndex >= 0; }

      [[nodiscard]] bool Equals(const Pad& to) const
      {
        return mName == to.mName &&
               mUUID == to.mUUID &&
               mIndex == to.mIndex;
      }

      [[nodiscard]] bool Same(const Pad& than) const
      {
        return mName == than.mName &&
               mUUID == than.mUUID;
      }

      [[nodiscard]] String AsString() const { return String(mName).Append('.').Append(mUUID).Append('.').Append(mIndex); }

      void Copy(const Pad& source)
      {
        mName = source.mName;
        mUUID = source.mUUID;
        mPath = source.mPath;
        mIndex = source.mIndex;
        mPosition = source.mPosition;
      }
    };

    //! Pad array
    typedef Pad PadArray[Input::sMaxInputDevices];
    //! Pad list
    typedef std::vector<Pad> PadList;

    //! Constructor
    InputMapper() = default;

    //! Destructor
    virtual ~InputMapper();

    /*!
     * @brief Compose numbered name at the given index, by counting same pads' name/uuid with lower indexes
     * @param padArray Pad array
     * @param index Index of the pad to get name from
     * @return Name or numbered name
     */
    String GetDecoratedName(int index);

    /*!
     * @brief Get all connected pads in a compact list (no unconnected pads)
     * @return Connected pad list
     */
    [[nodiscard]] PadList GetPads() const;

    /*!
     * @brief Swap pads at the given positions
     * @param index1 First pad index
     * @param index2 Second pad index
     */
    void Swap(int index1, int index2);

    //! Get connected pad count
    [[nodiscard]] int ConnectedPadCount() const;

    /*!
     * @brief Get real pad index (not included disconnected pads) from its identifier
     * @param identifier Device identifier
     * @return Index from 0 to X, or -1 if he device is unknown
     */
    int PadIndexFromDeviceIdentifier(int identifier);

  private:
    //! Pad array
    PadArray mPads;

    //! Rebuid the pad array, ready to be used
    void Build();

    /*!
     * @brief Load configuration from recalbox.conf
     */
    void LoadConfiguration();

    /*!
     * @brief Save configuration into recalbox.conf
     */
    void SaveConfiguration();

    //! Get available pad list
    static PadList AvailablePads();

    // Assign each pad a position in the mapper list
    void AssignPositions();

    /*
     * IInputChange implementation
     */

    //! Refresh pad list
    void PadsAddedOrRemoved(bool removed) override;
};




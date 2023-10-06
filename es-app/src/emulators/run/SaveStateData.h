//
// Created by gugue_u on 04/01/2023.
//

#pragma once

#include <utils/String.h>

class SaveStateData
{
  public:
    SaveStateData() = default;

    [[nodiscard]] const String& SlotNumber() const { return mSlotNumber; }

    void SetSlotNumber(int slotNumber)
    {
      if (slotNumber < 0) mSlotNumber = String::Empty;
      mSlotNumber = String(slotNumber);
    }

  private:
    String mSlotNumber;
};

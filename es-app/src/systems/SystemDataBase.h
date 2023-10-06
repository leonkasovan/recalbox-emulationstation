//
// Created by bkg2k on 25/06/23.
//
#pragma once

class SystemDataBase
{
  public:
    SystemDataBase() : mInitialized(false) {}

    //! Public getter
    bool IsInitialized() const { return mInitialized; }

  private:
    //! System status - True = Games are loaded, False = games are not loaded (for virtual systems only)
    bool mInitialized;

    //! Git access to the setter to SystemManager
    friend class SystemManager;

    //! One-time setter
    void SetInitialized() { mInitialized = true; }
};

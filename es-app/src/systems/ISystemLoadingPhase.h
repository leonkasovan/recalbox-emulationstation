//
// Created by bkg2k on 25/06/23.
//
#pragma once

class ISystemLoadingPhase
{
  public:
    //! Loading Phase
    enum class Phase
    {
      RegularSystems, //!< Start loading regular systems
      VirtualSystems, //!< Start loading virtual systems
      Completed,      //!< Loading complete
    };

    //! Default constructor
    virtual ~ISystemLoadingPhase() = default;

    /*!
     * @brief Called on start of evert loading phase
     * @param phase Phase
     */
    virtual void SystemLoadingPhase(Phase phase) = 0;
};

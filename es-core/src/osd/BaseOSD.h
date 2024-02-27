//
// Created by bkg2k on 02/10/23.
//
#pragma once

#include <guis/Gui.h>

class BaseOSD : public Gui
{
  public:
    //! Screen side OSD are located
    enum class Side
    {
      Left,  //!< Left screen side
      Right, //!< Right screen side
    };

    /*!
     * @brief Constructor
     * @param window Window manager reference
     * @param side Screen side the OSD is located on
     */
    BaseOSD(WindowManager& window, Side side, bool inputAware)
      : Gui(window)
      , mSide(side)
      , mInputAware(inputAware)
    {
    }

    /*!
     * @brief Get OSD Area width
     * @return Area width
     */
    [[nodiscard]] virtual int OSDAreaWidth() const = 0;

    /*!
     * @brief Get OSD Area height
     * @return Area width
     */
    [[nodiscard]] virtual int OSDAreaHeight() const = 0;

    /*!
     * @brief Implementation must reply true if the OSD is active, false if not
     * @return True if the OSD is active/visible, false otherwise
     */
    [[nodiscard]] virtual bool IsActive() const = 0;

    //! Get size
    [[nodiscard]] Side WhichSide() const { return mSide; }

    //! Get input awarness
    [[nodiscard]] bool IsInputAware() const { return mInputAware; }

  private:
    //! Screen side where the OSD is located
    Side mSide;
    //! This OSD is input aware
    bool mInputAware;
};

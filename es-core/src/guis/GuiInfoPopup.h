//
// Created by bkg2k on 26/03/2022.
//
// As part of the RECALBOX Project
// http://www.recalbox.com
//
#pragma once

#include <components/ComponentGrid.h>
#include <WindowManager.h>
#include <guis/GuiInfoPopupBase.h>
#include <utils/Log.h>

class TextComponent;

class GuiInfoPopup : public GuiInfoPopupBase
{
  public:
    GuiInfoPopup(WindowManager& window, const String& message, int duration, PopupType icon);

  protected:
    float AddComponents(WindowManager& window, ComponentGrid& grid, float maxWidth, float maxHeight, int paddingX, int paddingY) override;

  private:
    //! Text message
    String mMessage;
    //! Icon
    PopupType mIcon;

    //! MEssage component
    std::shared_ptr<TextComponent> mMsgText;
    //! Icon component
    std::shared_ptr<TextComponent> mMsgIcon;
};




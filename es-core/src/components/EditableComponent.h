//
// Created by bkg2k on 21/01/2021.
//
// As part of the RECALBOX Project
// http://www.recalbox.com
//
#pragma once

#include <guis/IGuiArcadeVirtualKeyboardInterface.h>
#include "components/base/Component.h"
#include "resources/Font.h"
#include "NinePatchComponent.h"
#include "IEditableComponent.h"

class ThemeData;

class EditableComponent : public Component, private IGuiArcadeVirtualKeyboardInterface
{
  public:
    explicit EditableComponent(WindowManager&window);
    EditableComponent(WindowManager&window, const String& editTitle, const String& text, const std::shared_ptr<Font>& font, unsigned int color, int id, IEditableComponent* interface, bool masked);
    EditableComponent(WindowManager&window, const String& editTitle, const String& text, const std::shared_ptr<Font>& font, unsigned int color, const std::function<void(const String&)>& callback);
    EditableComponent(WindowManager&window, const String& editTitle, const String& text, const std::shared_ptr<Font>& font, unsigned int color, TextAlignment align, const std::function<void(const String&)>& callback);

    void setFont(const std::shared_ptr<Font>& font);
    void setUppercase(bool uppercase);
    void onSizeChanged() override;
    void setText(const String& text);
    void setColor(unsigned int color) override;
    inline void setOriginColor(unsigned int color) { mOriginColor = color; }
    unsigned int getOriginColor() override { return mOriginColor; }

    void setHorizontalAlignment(TextAlignment align);
    void setVerticalAlignment(TextAlignment align) { mVerticalAlignment = align; }

    void setLineSpacing(float spacing);

    void Render(const Transform4x4f& parentTrans) override;

    String getValue() const override { return mText; }
    void setValue(const String& value) override { setText(value); }

    unsigned char getOpacity() const override {	return (unsigned char)(mColor & 0xFF); }

    void setOpacity(unsigned char opacity) override;

    inline std::shared_ptr<Font> getFont() const { return mFont; }

    void applyTheme(const ThemeData& theme, const String& view, const String& element, ThemeProperties properties) override;

    //! Start the Virtual Keyboard to edit this component text
    void StartEditing();

  private:
    void calculateExtent();

    void onTextChanged();
    void onColorChanged();

    NinePatchComponent mBackground;
    std::shared_ptr<Font> mFont;
    std::shared_ptr<TextCache> mTextCache;
    String mText;
    String mTextBackup;
    String mEditTitle;
    std::function<void(const String&)> mTextChanged;
    IEditableComponent* mInterface;
    int mIndentifier;
    unsigned int mColor;
    unsigned int mOriginColor;
    float mMargin;
    float mLineSpacing;
    TextAlignment mHorizontalAlignment;
    TextAlignment mVerticalAlignment;
    unsigned char mColorOpacity;
    bool mUppercase;
    bool mAutoCalcExtentX;
    bool mAutoCalcExtentY;
    bool mMasked;

    /*
     * IGuiArcadeVirtualKeyboardInterface implementation
     */

    /*!
     * @brief Called when the edited text change.
     * Current text is available from the Text() method.
     */
    void ArcadeVirtualKeyboardTextChange(GuiArcadeVirtualKeyboard& vk, const String& text) final;

    /*!
     * @brief Called when the edited text is validated (Enter or Start)
     * Current text is available from the Text() method.
     */
    void ArcadeVirtualKeyboardValidated(GuiArcadeVirtualKeyboard& vk, const String& text) final;

    /*!
     * @brief Called when the edited text is cancelled.
     */
    void ArcadeVirtualKeyboardCanceled(GuiArcadeVirtualKeyboard& vk) final;
};




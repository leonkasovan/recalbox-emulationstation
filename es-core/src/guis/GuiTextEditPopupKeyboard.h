#include <guis/Gui.h>
#include <components/NinePatchComponent.h>
#include <components/ButtonComponent.h>
#include <components/ComponentGrid.h>
#include <components/TextEditComponent.h>
#include <components/TextComponent.h>

class GuiTextEditPopupKeyboard : public Gui
{
public:
	GuiTextEditPopupKeyboard(WindowManager& window, const String& title, const String& initValue,
                           const std::function<void(const String&)>& okCallback, bool multiLine,
                           const String& acceptBtnText);
  GuiTextEditPopupKeyboard(WindowManager& window, const String& title, const String& initValue,
                           const std::function<void(const String&)>& okCallback, bool multiLine)
    : GuiTextEditPopupKeyboard(window, title, initValue, okCallback, multiLine, "OK")
  {
  }

	bool ProcessInput(const InputCompactEvent& event) override;
	void Update(int) override {}
	void onSizeChanged() override;
	bool getHelpPrompts(Help& help) override;

private:

    class KeyboardButton
    {
    public:
        std::shared_ptr<ButtonComponent> button;
		    const String key;
		    const String shiftedKey;
        KeyboardButton(const std::shared_ptr<ButtonComponent>& b, const String& k, const String& sk)
          : button(b),
            key(k),
            shiftedKey(sk)
        {
        };
    };

	void switchShift();
    std::shared_ptr<ButtonComponent> makeButton(const String& key, const String& shiftedKey);

    NinePatchComponent mBackground;
	ComponentGrid mGrid;

	std::vector< KeyboardButton > keyboardButtons;

	std::shared_ptr<TextComponent> mTitle;
	std::shared_ptr<TextEditComponent> mText;
	std::shared_ptr<ComponentGrid> mKeyboardGrid;
	std::shared_ptr<ComponentGrid> mButtonGrid;
	std::shared_ptr<ComponentGrid> mButtons;

	bool mMultiLine;
	bool mShift = false;
	std::shared_ptr<ButtonComponent> mShiftButton;
};


#pragma once

#include <guis/Gui.h>
#include <components/NinePatchComponent.h>
#include <components/ComponentGrid.h>
#include <resources/Font.h>

class TextComponent;
class ButtonComponent;

class GuiMsgBox : public Gui
{
  public:
    GuiMsgBox(WindowManager& window, const String& text,
              const String& name1, const std::function<void()>& func1,
              const String& name2, const std::function<void()>& func2,
              const String& name3, const std::function<void()>& func3,
              TextAlignment align = TextAlignment::Center);
    GuiMsgBox(WindowManager& window, const String& text,
              const String& name1, const std::function<void()>& func1,
              const String& name2, const std::function<void()>& func2);
    GuiMsgBox(WindowManager& window, const String& text,
              const String& name1, const std::function<void()>& func1);
    GuiMsgBox(WindowManager& window, const String& text,
              const String& name1,
              TextAlignment align);
    GuiMsgBox(WindowManager& window, const String& text,
              const String& name1);
    GuiMsgBox(WindowManager& window, const String& text);

    GuiMsgBox* SetDefaultButton(int index);

  protected:
    bool ProcessInput(const InputCompactEvent& event) override;
    void onSizeChanged() override;
    bool getHelpPrompts(Help& help) override { return mGrid.getHelpPrompts(help); }

  private:
    explicit GuiMsgBox(WindowManager& window);

    void CloseAndCall(const std::function<void()>& func);

    void build(const String& text, TextAlignment align,
               const String& name1, const std::function<void()>& func1,
               const String& name2, const std::function<void()>& func2,
               const String& name3, const std::function<void()>& func3);

    NinePatchComponent mBackground;
    ComponentGrid mGrid;

    std::shared_ptr<TextComponent> mMsg;
    std::vector< std::shared_ptr<ButtonComponent> > mButtons;
    std::shared_ptr<ComponentGrid> mButtonGrid;
    std::function<void()> mAcceleratorFunc;

    int mSpace;
};

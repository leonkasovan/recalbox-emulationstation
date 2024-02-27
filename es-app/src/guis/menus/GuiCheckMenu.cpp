//
// Created by gugue_u on 17/01/2022.
//

#include <utils/locale/LocaleHelper.h>
#include <WindowManager.h>

#include "GuiCheckMenu.h"

GuiCheckMenu::GuiCheckMenu(WindowManager& window,
                           const String& title,
                           const String& footer,
                           int lastChoice,
                           const String& name1,
                           const String& help1,
                           const std::function<void()>& func1,
                           const String& name2,
                           const String& help2,
                           const std::function<void()>& func2)

                           : GuiMenuBase(window, title, nullptr)
{
    build(name1, help1, func1,
          name2, help2, func2,
          String(), String(), nullptr,
          String(), String(), nullptr,
          lastChoice, footer);

}

GuiCheckMenu::GuiCheckMenu(WindowManager& window,
                           const String& title,
                           const String& footer,
                           int lastChoice,
                           const String& name1,
                           const String& help1,
                           const std::function<void()>& func1,
                           const String& name2,
                           const String& help2,
                           const std::function<void()>& func2,
                           const String& name3,
                           const String& help3,
                           const std::function<void()>& func3)

                           : GuiMenuBase(window, title, nullptr)
{
    build(name1, help1, func1,
          name2, help2, func2,
          name3, help3, func3,
          String(), String(), nullptr,
          lastChoice, footer);
}

GuiCheckMenu::GuiCheckMenu(WindowManager& window,
                           const String& title,
                           const String& footer,
                           int lastChoice,
                           const String& name1,
                           const String& help1,
                           const std::function<void()>& func1,
                           const String& name2,
                           const String& help2,
                           const std::function<void()>& func2,
                           const String& name3,
                           const String& help3,
                           const std::function<void()>& func3,
                           const String& name4,
                           const String& help4,
                           const std::function<void()>& func4)

                           : GuiMenuBase(window, title, nullptr)
{
    build(name1, help1, func1,
          name2, help2, func2,
          name3, help3, func3,
          name4, help4, func4,
          lastChoice, footer);
}

bool GuiCheckMenu::ProcessInput(const InputCompactEvent& event)
{
    if (event.AnyDownPressed() || event.AnyUpPressed())
    {
        return true;
    }

    if (event.CancelPressed() || event.StartPressed())
    {
        Close();
        return true;
    }

    return GuiMenuBase::ProcessInput(event);
}

void GuiCheckMenu::CloseAndCall(const std::function<void()>& func)
{
    Close();
    if (func) func();
}

void GuiCheckMenu::build(const String &name1, const String &help1, const std::function<void()> &func1,
                         const String &name2, const String &help2, const std::function<void()> &func2,
                         const String &name3, const String &help3, const std::function<void()> &func3,
                         const String &name4, const String &help4, const std::function<void()> &func4,
                         int lastChoice, const String& footer) {

    mMenu.addButton(name1, help1, std::bind(&GuiCheckMenu::CloseAndCall, this, func1));
    mMenu.addButton(name2, help2, std::bind(&GuiCheckMenu::CloseAndCall, this, func2));

    if(!name3.empty())
        mMenu.addButton(name3, help3, std::bind(&GuiCheckMenu::CloseAndCall, this, func3));

    if(!name4.empty())
        mMenu.addButton(name4, help4, std::bind(&GuiCheckMenu::CloseAndCall, this, func4));

    mMenu.setCursorToButtons();
    mMenu.SetDefaultButton(lastChoice);
    GuiMenuBase::SetFooter(footer);
}

//
// Created by gugue_u on 17/01/2022.
//

#pragma once

#include "guis/menus/GuiMenuBase.h"

class WindowManager;
class SystemManager;

class GuiCheckMenu : public GuiMenuBase
{
  public:
    //! Constructor
    GuiCheckMenu(WindowManager& window,
                 const String& title,
                 const String& footer,
                 int lastChoice,
                 const String& name1,
                 const String& help1,
                 const std::function<void()>& func1,
                 const String& name2,
                 const String& help2,
                 const std::function<void()>& func2);

    GuiCheckMenu(WindowManager& window,
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
                 const std::function<void()>& func3);

    GuiCheckMenu(WindowManager& window,
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
                 const std::function<void()>& func4);

private:

    bool ProcessInput(const InputCompactEvent& event) override;
    void CloseAndCall(const std::function<void()>& func);

    void build(const String& name1, const String& help1, const std::function<void()>& func1,
               const String& name2, const String& help2, const std::function<void()>& func2,
               const String& name3, const String& help3, const std::function<void()>& func3,
               const String& name4, const String& help4, const std::function<void()>& func4,
               int lastChoice, const String& footer);
};

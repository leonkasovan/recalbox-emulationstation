//
// Created by bkg2k on 31/10/2020.
//

#include "GuiMenuBase.h"
#include <WindowManager.h>
#include <EmulationStation.h>
#include <MainRunner.h>
#include <input/InputCompactEvent.h>
#include <utils/locale/LocaleHelper.h>
#include <guis/GuiMsgBox.h>
#include <help/Help.h>
#include <components/SwitchComponent.h>
#include <components/SliderComponent.h>
#include <components/EditableComponent.h>
#include <patreon/PatronInfo.h>

GuiMenuBase::GuiMenuBase(WindowManager& window, const String& title, IGuiMenuBase* interface)
  : Gui(window)
  , mMenu(window, title)
  , mTheme(*MenuThemeData::getInstance()->getCurrentTheme())
  , mInterface(interface)
  , mMenuInitialized(false)
{
  addChild(&mMenu);
}

bool GuiMenuBase::ProcessInput(const InputCompactEvent& event)
{
  if (event.CancelPressed())
  {
    Close();
    return true;
  }

  if (event.StartPressed())
  {
    // Close everything
    mWindow.CloseAll();
    return true;
  }
  return Gui::ProcessInput(event);
}

bool GuiMenuBase::getHelpPrompts(Help& help)
{
  mMenu.getHelpPrompts(help);

  help.Set(HelpType::UpDown, _("CHOOSE"))
      .Set(Help::Cancel(), _("BACK"))
      .Set(HelpType::Start, _("CLOSE"));

  return true;
}

void GuiMenuBase::Update(int deltaTime)
{
  if (!mMenuInitialized)
  {
    // footer

    mMenu.setFooter(!mFooter.empty() ? mFooter : "RECALBOX VERSION " + String(PROGRAM_VERSION_STRING).UpperCase() + "  " + String(PROGRAM_BUILT_STRING).UpperCase());
    // Position
    mMenu.setPosition((Renderer::Instance().DisplayWidthAsFloat() - mMenu.getSize().x()) / 2, (Renderer::Instance().DisplayHeightAsFloat() - mMenu.getSize().y()) / 2);

    setSize(Renderer::Instance().DisplayWidthAsFloat(), Renderer::Instance().DisplayHeightAsFloat());

    mMenuInitialized = true;
  }

  Component::Update(deltaTime);
}

std::shared_ptr<TextComponent> GuiMenuBase::AddSubMenu(const String& label, int id, const String& help)
{
  ComponentListRow row;
  row.SetCallbackInterface(id, this);

  if (!help.empty())
    row.makeHelpInputHandler([this, help, label] { mWindow.pushGui(new GuiMsgBoxScroll(mWindow, label, help, _("OK"), []{}, "", nullptr, "", nullptr)); return true; });

  auto entryMenu = std::make_shared<TextComponent>(mWindow, label, mTheme.menuText.font, mTheme.menuText.color);
  row.addElement(entryMenu, true);
  row.addElement(makeArrow(mWindow), false);
  mMenu.addRowWithHelp(row, label, help);
  return entryMenu;
}

std::shared_ptr<TextComponent> GuiMenuBase::AddSubMenu(const String& label, const Path& iconPath, int id, const String& help)
{
  ComponentListRow row;
  row.SetCallbackInterface(id, this);

  if (!iconPath.IsEmpty())
  {
    // icon
    auto icon = std::make_shared<ImageComponent>(mWindow);
    icon->setImage(iconPath);
    icon->setColorShift(mTheme.menuText.color);
    icon->setResize(0, mTheme.menuText.font->getLetterHeight() * 1.25f);
    row.addElement(icon, false, true);

    // spacer between icon and text
    auto spacer = std::make_shared<Component>(mWindow);
    spacer->setSize(10.f + Math::round(mTheme.menuText.font->getLetterHeight() * 1.25f) - Math::round(icon->getSize().x()), 0);
    row.addElement(spacer, false, true);
  }

  if (!help.empty())
    row.makeHelpInputHandler([this, help, label] { mWindow.pushGui(new GuiMsgBoxScroll(mWindow, label, help, _("OK"), []{}, "", nullptr, "", nullptr)); return true; });

  auto entryMenu = std::make_shared<TextComponent>(mWindow, label, mTheme.menuText.font, mTheme.menuText.color);
  row.addElement(entryMenu, true);
  row.addElement(makeArrow(mWindow), false);
  mMenu.addRowWithHelp(row, label, help);
  return entryMenu;
}

std::shared_ptr<TextComponent> GuiMenuBase::AddSubMenu(const String& label, int id)
{
  return AddSubMenu(label, id, String::Empty);
}

std::shared_ptr<TextComponent> GuiMenuBase::AddSubMenu(const String& label, const Path& icon, int id)
{
  return AddSubMenu(label, icon, id, String::Empty);
}

std::shared_ptr<RatingComponent> GuiMenuBase::AddRating(const String& text, float value, int id, IRatingComponent* interface, const String& help)
{
  auto result = std::make_shared<RatingComponent>(mWindow, mTheme.menuText.color, value);
  result->setSize(0, mTheme.menuText.font->getHeight() * 0.8f);
  mMenu.addWithLabel(result, text, help);
  result->SetInterface(id, interface);
  return result;
}

std::shared_ptr<RatingComponent> GuiMenuBase::AddRating(const String& text, float value, int id, IRatingComponent* interface)
{
  return AddRating(text, value, id, interface, String::Empty);
}

std::shared_ptr<SliderComponent> GuiMenuBase::AddSlider(const String& text, float min, float max, float inc, float value, const String& suffix, int id, ISliderComponent* interface, const String& help)
{
  auto result = std::make_shared<SliderComponent>(mWindow, min, max, inc, suffix);
  result->setSlider(value);
  mMenu.addWithLabel(result, text, help);
  result->SetInterface(id, interface);
  return result;
}

std::shared_ptr<SliderComponent> GuiMenuBase::AddSlider(const String& text, float min, float max, float inc, float value, const String& suffix, int id, ISliderComponent* interface)
{
  return AddSlider(text, min, max, inc, value, suffix, id, interface, String::Empty);
}

std::shared_ptr<SwitchComponent> GuiMenuBase::AddSwitch(const Path& icon, const String& text, bool value, int id, ISwitchComponent* interface, const String& help)
{
  auto result = std::make_shared<SwitchComponent>(mWindow, value);
  mMenu.addWithLabel(result, icon, text, help);
  result->SetInterface(id, interface);
  return result;
}

std::shared_ptr<SwitchComponent> GuiMenuBase::AddSwitch(const Path& icon, const String& text, bool value, int id, ISwitchComponent* interface)
{
  return AddSwitch(icon, text, value, id, interface, String::Empty);
}

std::shared_ptr<SwitchComponent> GuiMenuBase::AddSwitch(const String& text, bool value, int id, ISwitchComponent* interface, const String& help)
{
  auto result = std::make_shared<SwitchComponent>(mWindow, value);
  mMenu.addWithLabel(result, text, help);
  result->SetInterface(id, interface);
  return result;
}

std::shared_ptr<SwitchComponent> GuiMenuBase::AddSwitch(const String& text, bool value, int id, ISwitchComponent* interface)
{
  return AddSwitch(text, value, id, interface, String::Empty);
}

std::shared_ptr<TextComponent> GuiMenuBase::AddText(const String& text, const String& value, unsigned int color, const String& help)
{
  auto result = std::make_shared<TextComponent>(mWindow, value, mTheme.menuText.font, color);
  mMenu.addWithLabel(result, text, help);
  return result;
}

std::shared_ptr<TextComponent> GuiMenuBase::AddText(const String& text, const String& value, unsigned int color)
{
  return AddText(text, value, color, String::Empty);
}

std::shared_ptr<TextComponent> GuiMenuBase::AddText(const String& text, const String& value, const String& help)
{
  auto result = std::make_shared<TextComponent>(mWindow, value, mTheme.menuText.font, mTheme.menuText.color);
  mMenu.addWithLabel(result, text, help);
  return result;
}

std::shared_ptr<TextComponent> GuiMenuBase::AddText(const String& text, const String& value)
{
  return AddText(text, value, String::Empty);
}

std::shared_ptr<EditableComponent> GuiMenuBase::AddEditable(const String& edittitle, const String& text, const String& value, int id, IEditableComponent* interface, const String& help, bool masked)
{
  auto result = std::make_shared<EditableComponent>(mWindow, edittitle, value, mTheme.menuText.font, mTheme.menuText.color, id, interface, masked);
  result->setSize(mMenu.getSize().x() / 3.f, mTheme.menuText.font->getHeight());
  result->setHorizontalAlignment(TextAlignment::Right);
  mMenu.addWithLabel(result, text, help, false, true, [result] { result->StartEditing(); });
  return result;
}

std::shared_ptr<EditableComponent> GuiMenuBase::AddEditable(const String& edittitle, const String& text, const String& value, int id, IEditableComponent* interface, bool masked)
{
  return AddEditable(edittitle, text, value, id, interface, String::Empty, masked);
}

std::shared_ptr<EditableComponent> GuiMenuBase::AddEditable(const String& text, const String& value, int id, IEditableComponent* interface,
                                                            const String& help, bool masked)
{
  return AddEditable(text, text, value, id, interface, help, masked);
}

std::shared_ptr<EditableComponent> GuiMenuBase::AddEditable(const String& text, const String& value, int id, IEditableComponent* interface, bool masked)
{
  return AddEditable(text, text, value, id, interface, String::Empty, masked);
}

void GuiMenuBase::Reboot()
{
  MainRunner::RequestQuit(MainRunner::ExitState::NormalReboot);
}

void GuiMenuBase::RequestReboot()
{
  mWindow.pushGui(new GuiMsgBox(mWindow, _("THE SYSTEM WILL NOW REBOOT"),
                                _("OK"), Reboot,
                                _("LATER"), std::bind(GuiMenuBase::RebootPending, &mWindow)));
}

void GuiMenuBase::RebootPending(WindowManager* window)
{
  static bool pending = false;
  if (!pending)
  {
    window->InfoPopupAdd(new GuiInfoPopup(*window, _("A reboot is required to apply pending changes."), 10000, PopupType::Reboot));
    pending = true;
  }
}

void GuiMenuBase::Relaunch()
{
  MainRunner::RequestQuit(MainRunner::ExitState::Relaunch, true);
}

void GuiMenuBase::RequestRelaunch()
{
  mWindow.pushGui(
    new GuiMsgBox(mWindow, _("EmulationStation must relaunch to apply your changes."),
                  _("OK"), Relaunch,
                  _("LATER"), std::bind(GuiMenuBase::RebootPending, &mWindow)));
}

void GuiMenuBase::ComponentListRowSelected(int id)
{
  // Pass-thru from ComponentListRow to user interface
  if (mInterface != nullptr)
    mInterface->SubMenuSelected(id);
}



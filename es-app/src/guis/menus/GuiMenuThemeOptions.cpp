//
// Created by bkg2k on 31/10/2020.
//

#include <MainRunner.h>
#include "GuiMenuThemeOptions.h"
#include "guis/MenuMessages.h"
#include "guis/GuiMsgBox.h"

GuiMenuThemeOptions::GuiMenuThemeOptions(WindowManager& window)
  : GuiMenuBase(window, _("THEME"), nullptr)
  , mRecalboxThemeIndex(0)
{
  // carousel transition option
  mCarousel = AddSwitch(_("CAROUSEL ANIMATION"), mOriginalCaroussel = RecalboxConf::Instance().GetThemeCarousel(), (int)Components::Carousel, this, _(MENUMESSAGE_UI_CAROUSEL_HELP_MSG));

  // transition style
  mTransition = AddList(_("TRANSITION STYLE"), (int)Components::Transition, this, GetTransitionEntries(), _(MENUMESSAGE_UI_TRANSITION_HELP_MSG));

  // theme set
  mTheme = AddList(_("THEME SET"), (int)Components::Theme, this, GetThemeEntries(), _(MENUMESSAGE_UI_THEME_HELP_MSG));
}

GuiMenuThemeOptions::~GuiMenuThemeOptions()
{
  if ((mCarousel->getState() != mOriginalCaroussel) ||
      (mTransition->getSelected() != mOriginalTransition) ||
      (mTheme->getSelected() != mOriginalTheme))
  {
    ThemeData::SetThemeChanged(true);
    RequestRelaunch();
  }
}

std::vector<GuiMenuBase::ListEntry<String>> GuiMenuThemeOptions::GetTransitionEntries()
{
  std::vector<ListEntry<String>> list;

  mOriginalTransition = RecalboxConf::Instance().GetThemeTransition();
  list.push_back({ _("FADE"), "fade", mOriginalTransition == "fade" });
  list.push_back({ _("SLIDE"), "slide", mOriginalTransition == "slide" });
  list.push_back({ _("INSTANT"), "instant", mOriginalTransition == "instant" });

  return list;
}

std::vector<GuiMenuBase::ListEntry<String>> GuiMenuThemeOptions::GetThemeEntries()
{
  auto themeSets = ThemeData::getThemeSets();
  auto selectedSet = themeSets.find(RecalboxConf::Instance().GetThemeFolder());
  if (selectedSet == themeSets.end()) selectedSet = themeSets.begin();
  mOriginalTheme = selectedSet->first;
  std::vector<ListEntry<String>> list;
  for (const auto& it : themeSets)
  {
    if (it.first == "recalbox-next") mRecalboxThemeIndex = (int)list.size();
    list.push_back({ it.first, it.first, it.first == mOriginalTheme });
  }

  return list;
}

void GuiMenuThemeOptions::OptionListComponentChanged(int id, int index, const String& value)
{
  (void)index;
  if ((Components)id == Components::Transition) RecalboxConf::Instance().SetThemeTransition(value).Save();
  else if ((Components)id == Components::Theme)
  {
    if (Board::Instance().CrtBoard().IsCrtAdapterAttached() && value != "recalbox-240p")
    {
      Gui* gui = new GuiMsgBox(mWindow, _("Are you sure the selected theme is compatible with CRT screens?"),
                               _("YES"), [value] { RecalboxConf::Instance().SetThemeFolder(value).Save();; },
                               _("NO"), [this, index] { mTheme->setSelectedIndex(index); });
      mWindow.pushGui(gui);

    }
    else RecalboxConf::Instance().SetThemeFolder(value).Save();
  }
}

void GuiMenuThemeOptions::SwitchComponentChanged(int id, bool& status)
{
  if ((Components)id == Components::Carousel)
    RecalboxConf::Instance().SetThemeCarousel(status).Save();
}

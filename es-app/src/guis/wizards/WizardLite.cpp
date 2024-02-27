//
// Created by bkg2k on 11/10/23.
//

#include "WizardLite.h"
#include "guis/menus/GuiMenuNetwork.h"
#include "components/PictureComponent.h"

WizardBase::Move WizardLite::OnKeyReceived(int page, const InputCompactEvent& event)
{
  if (page != 0)
    if (event.CancelPressed()) return Move::Backward;

  return Move::None;
}

bool WizardLite::OnComponentRequired(int page, int componentIndex, Rectangle& where, std::shared_ptr<Component>& component)
{
  switch((Pages)page)
  {
    case Pages::Intro:
    {
      if (componentIndex == 0)
      {
        component = std::make_shared<PictureComponent>(mWindow, true, Path(":/recalbox_x_kubii.svg"));
        where = Rectangle(0, 0, 2, 10);
        return true;
      }
      if (componentIndex == 1)
      {
        auto menuTheme = MenuThemeData::getInstance()->getCurrentTheme();
        String text = _("Recalbox et Kubii vous remercient pour votre confiance !\n"
                        "\n"
                        "Profitons de ce premier lancement pour découvrir ensemble comment utiliser votre Recalbox.");
        std::shared_ptr<TextComponent> textComponent = std::make_shared<TextComponent>(mWindow, text, menuTheme->menuText.font, menuTheme->menuText.color, TextAlignment::Left);
        textComponent->setVerticalAlignment(TextAlignment::Top);
        component = textComponent;
        where = Rectangle(3, 1, 7, 9);
        return true;
      }
      break;
    }
    case Pages::HowTo:
    {
      if (componentIndex == 0)
      {
        component = std::make_shared<PictureComponent>(mWindow, true, Path(":/pad_info_fr.svg"));
        where = Rectangle(0, 5, 10, 5);
        return true;
      }
      if (componentIndex == 1)
      {
        auto menuTheme = MenuThemeData::getInstance()->getCurrentTheme();
        String text = _("Vous pouvez naviguer dans les menus à l’aide de la croix directionnelle, sélectionner un système ou un jeu à l’aide du bouton B et en sortir à l’aide du bouton A.\n\n"
                        "Accédez au menu principal à l’aide du bouton START.\n"
                        "Pour sortir d’un jeu, appuyez simultanément sur les boutons SELECT et START");
        std::shared_ptr<TextComponent> textComponent = std::make_shared<TextComponent>(mWindow, text, menuTheme->menuText.font, menuTheme->menuText.color, TextAlignment::Left);
        textComponent->setVerticalAlignment(TextAlignment::Top);
        component = textComponent;
        where = Rectangle(0, 0, 10, 5);
        return true;
      }
      break;
    }
    case Pages::AddGames:
    {
      if (componentIndex == 0)
      {
        component = std::make_shared<PictureComponent>(mWindow, true, Path(":/win_network_share.png"));
        where = Rectangle(0, 5, 10, 5);
        return true;
      }
      if (componentIndex == 1)
      {
        auto menuTheme = MenuThemeData::getInstance()->getCurrentTheme();
        String text = _("Recalbox partage ses dossiers de roms, de bios et de sauvegardes sur le réseau local. Pour ajouter vos roms, rien de plus simple : sur votre ordinateur, recherchez votre “recalbox” dans les partages réseaux.\n"
                        "Accédez au dossier roms, et copiez ensuite votre jeu dans le dossier correspondant à son système. Par exemple, pour ajouter un jeu “NES”, copiez-le dans le dossier 'roms/nes'.");
        std::shared_ptr<TextComponent> textComponent = std::make_shared<TextComponent>(mWindow, text, menuTheme->menuText.font, menuTheme->menuText.color, TextAlignment::Left);
        textComponent->setVerticalAlignment(TextAlignment::Top);
        component = textComponent;
        where = Rectangle(0, 0, 10, 5);
        return true;
      }
      break;
    }
    case Pages::Update:
    {
      if (componentIndex == 0)
      {
        component = std::make_shared<PictureComponent>(mWindow, true, Path(":/recalbox_qrcode.svg"));
        where = Rectangle(0, 0, 3, 10);
        return true;
      }
      if (componentIndex == 1)
      {
        auto menuTheme = MenuThemeData::getInstance()->getCurrentTheme();
        String text = _("Passez gratuitement à la version complète du système en connectant votre recalbox à internet et profitez de tous les émulateurs et de toutes les fonctionnalités !\n\n"
                        "Une fois votre Recalbox connectée, une mise à jour vous sera proposée automatiquement.\n\n"
                        "Retrouvez des infos utiles et des tutoriels sur recalbox.com, ou sur le wiki recalbox en scannant le qr code.");
        std::shared_ptr<TextComponent> textComponent = std::make_shared<TextComponent>(mWindow, text, menuTheme->menuText.font, menuTheme->menuText.color, TextAlignment::Left);
        textComponent->setVerticalAlignment(TextAlignment::Top);
        component = textComponent;
        where = Rectangle(4, 0, 6, 10);
        return true;
      }
      break;
    }
    case Pages::Count: break;
  }

  return false;
}

bool WizardLite::OnButtonRequired(int page, int buttonIndex, String& buttonText)
{
  if ((Pages)page == Pages::Update && buttonIndex == 0)
  {
    buttonText = _("NETWORK SETTINGS");
    return true;
  }
  return false;
}

WizardBase::Move WizardLite::OnButtonClick(int page, int buttonIndex)
{
  if ((Pages)page == Pages::Update && buttonIndex == 0)
    mWindow.pushGui(new GuiMenuNetwork(mWindow));

  return Move::None;
}

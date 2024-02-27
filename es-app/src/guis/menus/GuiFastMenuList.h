//
// Created by bkg2k on 12/07/23.
//
#pragma once

#include <utils/String.h>
#include "IFastMenuListCallback.h"
#include "WindowManager.h"
#include "GuiMenuBase.h"

class GuiFastMenuList : public GuiMenuBase
                      , public IGuiMenuBase
{
  public:
    //! Simple entry structure
    struct EntryParam
    {
      String Name; //!< Display text
      String Help; //!< Auto help message
      EntryParam(const String& name, const String& help) : Name(name), Help(help) {}
      EntryParam(const String& name) : Name(name), Help(name) {}
    };

    //! Parameter list
    typedef std::vector<EntryParam> EntryParamList;

    /*!
     * @brief Constructor
     * @param window Window manager
     * @param callbackInterface Callback interface
     * @param title Menu title
     * @param footer Menu footer
     * @param menuIndex Global menu index
     * @param params Line params
     * @param defaultIndex Default selected line index
     */
    GuiFastMenuList(WindowManager& window, IFastMenuListCallback* callbackInterface,
                    const String& title, const String& footer, int menuIndex,
                    const EntryParamList& params, int defaultIndex)
       : GuiMenuBase(window, title, this)
       , mMenuIndex(menuIndex)
       , mInterface(callbackInterface)
    {
      // Add lines
      int entryIndex = 0;
      for(const EntryParam& param : params)
        AddSubMenu(param.Name, entryIndex++, param.Help);

      // Add can,cel button
      mMenu.addButton(_("CANCEL"), String::Empty, [this] { Close(); });

      // Set default line or button
      if (defaultIndex>= 0) mMenu.setCursorToList(defaultIndex);
      else mMenu.setCursorToButtons();

      // Set footer if any
      if (!footer.empty()) SetFooter(footer);
    }

  private:
    //! Menu index
    int mMenuIndex;
    //! Interface
    IFastMenuListCallback* mInterface;

    /*!
     * @brief Line selected, call interface back
     * @param id
     */
    void SubMenuSelected(int id) override
    {
      if (mInterface != nullptr)
        mInterface->FastMenuLineSelected(mMenuIndex, id);
      Close();
    }
};

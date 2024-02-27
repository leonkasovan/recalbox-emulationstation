//
// Created by digitalLumberjack on 20/0/2021.
//
// As part of the RECALBOX Project
// http://www.recalbox.com
//
#pragma once

#include <guis/menus/GuiMenuBase.h>
#include "hardware/crt/CRTScanlines.h"

// Forward declaration
class SystemManager;
class SwitchComponent;
template<class T> class OptionListComponent;

class GuiMenuCRT : public GuiMenuBase
                 , private IOptionListComponent<CrtAdapterType>
                 , private IOptionListComponent<String>
                 , private IOptionListComponent<CrtScanlines>
                 , private IOptionListComponent<ICrtInterface::HorizontalFrequency>
                 , private ISwitchComponent
                 , private IGuiMenuBase
{
  public:
    /*!
     * @brief Default constructor
     * @param window Global window
     */
    explicit GuiMenuCRT(WindowManager& window, const String title);

    //! Default destructor
    ~GuiMenuCRT() override;

  private:
    enum class Components
    {
      CRTDac,
      EsResolution,
      GameRegion,
      GameResolution,
      DemoIn240pOn31kHz,
      ScanlinesOn31kHz,
      Extended15kHzRange,
      UseV2,
      SuperRez,
      ReduceLatency,
      RunAhead,
      ForceJack,
      Adjustment,
      HorizontalPalOffset,
      VerticalPalOffset,
      ForceHDMI,
      JammaPanelButtons,
      JammaNeogeoLayout,
      JammaStartBtn1Credit,
      JammaHKOnStart,
      JammaExitOnStart,
      JammaMonoBoost,
      JammaScreenType,
      Jamma4Players,
      JammaAutoFire,
      JammaButtonsBtn6Gnd,
      ResetJamma
    };

    //! Dac selection
    std::shared_ptr<OptionListComponent<CrtAdapterType>> mDac;
    CrtAdapterType mOriginalDac;
    //! Es resolution
    std::shared_ptr<OptionListComponent<String>> mEsResolution;
    String mOriginalEsResolution;
    //! Force jack audio
    bool mForceJack;
    bool mOriginalForceJack;
    //! Force HDMI video
    bool mForceHDMI;
    bool mOriginalForceHDMI;
    bool mOriginalFrontendIn240pOn31kHz;
    std::string mOriginalJammaNeogeoLayout;
    ICrtInterface::HorizontalFrequency mOriginalResolution;
    std::shared_ptr<OptionListComponent<ICrtInterface::HorizontalFrequency>> mScreenTypeList;

    //! Get dacs
    static std::vector<ListEntry<CrtAdapterType>> GetDacEntries(bool onlyRgbDual);
    //! Get resolutions
    static std::vector<ListEntry<String>> GetEsResolutionEntries(bool only31kHz, bool supports120Hz, bool multisync, bool interlaced);
    //! Get super resolutions
    static std::vector<ListEntry<String>> GetSuperRezEntries();
    //! Get scanlines
    static std::vector<GuiMenuBase::ListEntry<CrtScanlines>> GetScanlinesEntries();


    /*!
     * @brief Get Horizontal frequency display test
     * @return Text
     */
    static String GetHorizontalFrequency();

    /*!
     * @brief Get 50hz display test
     * @return Texg
     */
    static String Get50hz();

    /*
     * IOptionListComponent<CrtAdapterType> implementation
     */

    void OptionListComponentChanged(int id, int index, const CrtAdapterType& value) override;

    /*
     * IOptionListComponent<std::string> implementation
     */

    void OptionListComponentChanged(int id, int index, const String & value) override;

    /*
     * IOptionListComponent<CrtScanlines> implementation
     */
    void OptionListComponentChanged(int id, int index, const CrtScanlines& value) override;

    /*
     * IOptionListComponent<ICrtInterface::HorizontalFrequency> implementation
     */
    void OptionListComponentChanged(int id, int index, const ICrtInterface::HorizontalFrequency& value) override;

    /*
     * ISwitchComponent implementation
     */
    void SwitchComponentChanged(int id, bool& status) override;

    /*
     * IGuiMenuBase implementation
     */

    void SubMenuSelected(int id) override;

};

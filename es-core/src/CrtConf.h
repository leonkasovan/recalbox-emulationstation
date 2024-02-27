//
// Created by bkg2k on 11/01/2022.
//
// As part of the RECALBOX Project
// http://www.recalbox.com
//
#pragma once

#include <RecalboxConf.h>
#include <hardware/crt/CrtAdapterType.h>
#include <views/crt/CrtResolutions.h>
#include "hardware/crt/CRTScanlines.h"

class CrtConf: public IniFile, public StaticLifeCycleControler<CrtConf>
{
  public:
    //! Constructor
    CrtConf();

    #define DefineCrtModeOffsetDeclaration(name, type, type2, key) \
      type GetCrtModeOffset##name(CrtResolution reso) const; \
      CrtConf& SetCrtModeOffset##name(CrtResolution reso, const type& value);

    #define DefineCrtModeOffsetImplementation(name, type, type2, key, defaultValue) \
      type CrtConf::GetCrtModeOffset##name(CrtResolution reso) const { return As##type2(String(sModeOffsetPrefix).Append('.').Append(CrtResolutionFromEnum(reso)).Append('.').Append(key), defaultValue); } \
      CrtConf& CrtConf::SetCrtModeOffset##name(CrtResolution reso, const type& value) { Set##type2(String(sModeOffsetPrefix).Append('.').Append(CrtResolutionFromEnum(reso)).Append('.').Append(key), value); return *this; }

    #define DefineCrtViewportDeclaration(name, type, type2, key) \
      type GetCrtViewport##name(CrtResolution reso) const; \
      CrtConf& SetCrtViewport##name(CrtResolution reso, const type& value);

    #define DefineCrtViewportImplementation(name, type, type2, key, defaultValue) \
      type CrtConf::GetCrtViewport##name(CrtResolution reso) const { return As##type2(String(sViewportPrefix).Append('.').Append(CrtResolutionFromEnum(reso)).Append('.').Append(key), defaultValue); } \
      CrtConf& CrtConf::SetCrtViewport##name(CrtResolution reso, const type& value) { Set##type2(String(sViewportPrefix).Append('.').Append(CrtResolutionFromEnum(reso)).Append('.').Append(key), value); return *this; }

    /*!
     * @brief Called when file has been saved
     */
    void OnSave() const override;

    DefineGetterSetterEnumGeneric(CrtConf, SystemCRT, CrtAdapterType, sSystemCRT, CrtAdapter)
    DefineGetterSetterGeneric(CrtConf, SystemCRTResolution, String, String, sSystemCRTResolution, "240")
    DefineGetterSetterGeneric(CrtConf, SystemCRT31kHzResolution, String, String, sSystemCRT31kHzResolution, "480")
    DefineGetterSetterGeneric(CrtConf, SystemCRTGameRegionSelect, bool, Bool, sSystemCRTGameRegionSelect, false)
    DefineGetterSetterGeneric(CrtConf, SystemCRTGameResolutionSelect, bool, Bool, sSystemCRTGameResolutionSelect, true)
    DefineGetterSetterGeneric(CrtConf, SystemCRTRunDemoIn240pOn31kHz, bool, Bool, sSystemCRTRunDemoIn240pOn31kHz, false)
    DefineGetterSetterEnumGeneric(CrtConf, SystemCRTScanlines31kHz, CrtScanlines, sSystemCRTScanlines31kHz, CrtScanlines)
    DefineGetterSetterGeneric(CrtConf, SystemCRTExtended15KhzRange, bool, Bool, sSystemCRTExtended15KhzRange, false)
    DefineGetterSetterGeneric(CrtConf, SystemCRTSuperrez, String, String, sSystemCRTSuperrez, "x6")
    DefineGetterSetterGeneric(CrtConf, SystemCRTUseV2, bool, Bool, sSystemCRTUseV2, false)
    DefineGetterSetterGeneric(CrtConf, SystemCRTForceJack, bool, Bool, sSystemCRTForceJack, false)
    DefineGetterSetterGeneric(CrtConf, SystemCRTForceHDMI, bool, Bool, sSystemCRTForceHDMI, false)
    DefineGetterSetterGeneric(CrtConf, SystemCRTJammaPanelButtons, String, String, sSystemCRTJammaPanelButtons, "6")
    DefineGetterSetterGeneric(CrtConf, SystemCRTJammaMonoAmpBoost, String, String, sSystemCRTJammaMonoAmpBoost, "0")
    DefineGetterSetterGeneric(CrtConf, SystemCRTJammaNeogeoLayout, String, String, sSystemCRTJammaNeogeoLayout, "line")
    DefineGetterSetterGeneric(CrtConf, SystemCRTJammaStartBtn1Credit, bool, Bool, sSystemCRTJammaStartBtn1Credit, true)
    DefineGetterSetterGeneric(CrtConf, SystemCRTJammaHKOnStart, bool, Bool, sSystemCRTJammaHKOnStart, true)
    DefineGetterSetterGeneric(CrtConf, SystemCRTJammaExitOnStart, bool, Bool, sSystemCRTJammaExitOnStart, true)
    DefineGetterSetterGeneric(CrtConf, SystemCRTJamma4Players, bool, Bool, sSystemCRTJamma4Players, false)
    DefineGetterSetterGeneric(CrtConf, SystemCRTJammaAutoFire, bool, Bool, sSystemCRTJammaAutoFire, true)
    DefineGetterSetterGeneric(CrtConf, SystemCRTJammaButtonsOnJamma, String, String, sSystemCRTJammaButtonsOnJamma, "6")
    DefineGetterSetterGeneric(CrtConf, SystemCRTScreen31kHz, bool, Bool, sSystemCRTScreen31kHz, false)
    DefineGetterSetterGeneric(CrtConf, SystemCRTScreenMultiSync, bool, Bool, sSystemCRTScreenMultiSync, false)


    DefineCrtModeOffsetDeclaration(VerticalOffset, int, Int, sVerticalOffset)
    DefineCrtModeOffsetDeclaration(HorizontalOffset, int, Int, sHorizontalOffset)
    DefineCrtViewportDeclaration(Width, int, Int, sWidth)

    static CrtResolution CrtResolutionFromString(const String& menu);
    static const String& CrtResolutionFromEnum(CrtResolution type);

    static CrtScanlines CrtScanlinesFromString(const String& scanlines);
    static const String& CrtScanlinesFromEnum(CrtScanlines scanlines);

  private:
    static constexpr const char* sSystemCRT                       = "adapter.type";
    static constexpr const char* sSystemCRTResolution             = "options.es.resolution";
    static constexpr const char* sSystemCRTGameRegionSelect       = "options.regionselect";
    static constexpr const char* sSystemCRTGameResolutionSelect   = "options.resolutionselect";
    static constexpr const char* sSystemCRTUseV2                  = "options.usev2";
    static constexpr const char* sSystemCRTSuperrez               = "options.superrez";
    static constexpr const char* sSystemCRT31kHzResolution        = "options.31khz.es.resolution";
    static constexpr const char* sSystemCRTRunDemoIn240pOn31kHz   = "options.31khz.demo240pOn31khz";
    static constexpr const char* sSystemCRTScanlines31kHz         = "options.31khz.scanlines";
    static constexpr const char* sSystemCRTExtended15KhzRange     = "options.15khz.extendedrange";
    static constexpr const char* sSystemCRTForceJack              = "audio.forcejack";
    static constexpr const char* sSystemCRTForceHDMI              = "video.forcehdmi";
    static constexpr const char* sSystemCRTScreen31kHz            = "options.screen.31kHz";
    static constexpr const char* sSystemCRTScreenMultiSync        = "options.screen.multisync";
    static constexpr const char* sSystemCRTJammaMonoAmpBoost      = "options.jamma.amp.boost";
    static constexpr const char* sSystemCRTJammaPanelButtons      = "options.jamma.controls.panel_buttons";
    static constexpr const char* sSystemCRTJammaNeogeoLayout      = "options.jamma.controls.neogeolayout";
    static constexpr const char* sSystemCRTJammaStartBtn1Credit   = "options.jamma.controls.credit_on_start_btn1";
    static constexpr const char* sSystemCRTJammaHKOnStart         = "options.jamma.controls.hk_on_start";
    static constexpr const char* sSystemCRTJammaExitOnStart       = "options.jamma.controls.exit_on_start";
    static constexpr const char* sSystemCRTJamma4Players          = "options.jamma.controls.4players";
    static constexpr const char* sSystemCRTJammaAutoFire          = "options.jamma.controls.autofire";
    static constexpr const char* sSystemCRTJammaButtonsOnJamma    = "options.jamma.controls.buttons_on_jamma";


    static constexpr const char* sViewportPrefix                  = "viewport";
    static constexpr const char* sModeOffsetPrefix                = "mode.offset";
    static constexpr const char* sVerticalOffset                  = "verticaloffset";
    static constexpr const char* sHorizontalOffset                = "horizontaloffset";
    static constexpr const char* sWidth                           = "width";

    static CrtAdapterType CrtAdapterFromString(const String& adapter);
    static const String& CrtAdapterFromEnum(CrtAdapterType adapter);
};




//
// Created by bkg2k on 03/10/23.
//
#pragma once

#include <memory>
#include "BaseOSD.h"
#include <utils/gl/Rectangle.h>
#include <RecalboxConf.h>

class FpsOSD : public BaseOSD
{
  public:
    /*!
     * @brief Constructor
     * @param window Window manager
     */
    explicit FpsOSD(WindowManager& window, Side side);

    /*!
     * @brief Record start of frame
     */
    void RecordStartFrame();

    /*!
     * @brief Record end of frame
     */
    void RecordStopFrame();

    /*
     * Component override
     */

    //! Draw
    void Render(const Transform4x4f& parentTrans) override;

    /*
     * BaseOSD implementation
     */

    //! Get max width
    [[nodiscard]] int OSDAreaWidth() const override { return (int)mFPSArea.Width(); }

    //! Get height
    [[nodiscard]] int OSDAreaHeight() const override { return (int)mFPSArea.Height(); }

    //! Visible?
    [[nodiscard]] bool IsActive() const override { return RecalboxConf::Instance().GetGlobalShowFPS(); };

  private:
    //! Number of recorded frame timing (in pow of 2)s
    static constexpr int sMaxFrameTimingPow2 = 6;
    //! Number of recorded frame timings (mask)
    static constexpr int sMaxFrameTimingMask = (1 << sMaxFrameTimingPow2) - 1;
    //! Number of recorded frame timings
    static constexpr int sMaxFrameTiming = 1 << sMaxFrameTimingPow2;

    //! FPS Font
    std::shared_ptr<Font> mFPSFont;
    //! FPS rectangle
    Rectangle mFPSArea;
    //! Frame timings start
    int mFrameStart[sMaxFrameTiming];
    //! Frame timings (computations)
    int mFrameTimingComputations[sMaxFrameTiming];
    //! Frame timings (computations)
    int mFrameTimingTotal[sMaxFrameTiming];
    //! Timing current index
    int mTimingIndex;
    //! Recorded timings
    int mRecordedTimings;

    /*!
     * @brief Calculate FPS using recorded operations
     * @return FPS average calculated on last sMaxFrameTiming frames
     */
    float CalculateFPS();

    /*!
     * @brief Calculate Frame percentage using recorded operations
     * @return Frame percentage average calculated on last sMaxFrameTiming frames
     */
    float CalculateFramePercentage();
};

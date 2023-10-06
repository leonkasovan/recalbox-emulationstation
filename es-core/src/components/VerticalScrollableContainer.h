//
// Created by bkg2k on 13/06/2023.
//
// As part of the RECALBOX Project
// http://www.recalbox.com
//
#pragma once

#include "components/base/Component.h"

class VerticalScrollableContainer : public Component
{
  public:
    explicit VerticalScrollableContainer(WindowManager&window);

    void reset();

    /*!
     * @brief Process right joystick text scrollinh control
     * @param event Compact event
     * @return Implementation must return true if it consumed the event.
     */
    bool ProcessInput(const InputCompactEvent& event) override;

    /*!
     * @brief Apply auto scrolling
     * @param deltaTime Elapsed time from the previous frame, in millisecond
     */
    void Update(int deltaTime) override;

    /*!
     * @brief Draw scrolled children
     * @param parentTrans Transformation
     */
    void Render(const Transform4x4f& parentTrans) override;

  private:
    enum class ScrollSteps
    {
      LeftPause,     //!< Start with a smart pause
      ScrollToRight, //!< Scroll until th elast char is visible
      RightPause,    //!< Smart pause again to let the user read the end
      RollOver,      //!< Scroll until text #2 left part is on the start position and then restart
    };

    static constexpr int sScrollSpeed1 = 40; // In pixel per seconds
    static constexpr int sScrollSpeed2 = 80; // In pixel per seconds
    static constexpr int sScrollPause = 5000; // In milliseconds

    void getContentHeight(int& min, int& max);

    int mScrollOffset;
    int mScrollTime;
    int mLastDirection;
    ScrollSteps mScrollStep;
    bool mAutoScroll;
};

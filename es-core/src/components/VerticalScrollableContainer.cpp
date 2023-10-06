//
// Created by bkg2k on 13/06/2023.
//
// As part of the RECALBOX Project
// http://www.recalbox.com
//

#include "components/VerticalScrollableContainer.h"
#include "Renderer.h"
#include "VerticalScrollableContainer.h"


VerticalScrollableContainer::VerticalScrollableContainer(WindowManager&window)
  : Component(window)
  , mScrollOffset(0)
  , mScrollTime(0)
  , mLastDirection(0)
  , mScrollStep(ScrollSteps::LeftPause)
  , mAutoScroll(true)
{
}

void VerticalScrollableContainer::Render(const Transform4x4f& parentTrans)
{
  if (mThemeDisabled) return;

	Transform4x4f trans = parentTrans * getTransform();

	Vector2i clipPos((int)trans.translation().x(), (int)trans.translation().y());
	Vector3f dimScaled = trans * Vector3f(mSize.x(), mSize.y(), 0);
	Vector2i clipDim((int)(dimScaled.x() - trans.translation().x()), (int)(dimScaled.y() - trans.translation().y()));

	Renderer::Instance().PushClippingRect(clipPos, clipDim);

  int min = 0, max = 0;
  getContentHeight(min, max);

	trans.translate(-Vector3f(0, (float)(mScrollOffset + min), 0));
	Renderer::SetMatrix(trans);
	Component::renderChildren(trans);

  if (mScrollOffset != 0)
  {
    int childrenHeight = max - min;
    trans.translate(Vector3f(0, (float)childrenHeight + mSize.y() / 4.f, 0));
    Renderer::SetMatrix(trans);
    Component::renderChildren(trans);
  }

	Renderer::Instance().PopClippingRect();
}

void VerticalScrollableContainer::Update(int deltaTime)
{
  int min = 0, max = 0;
  getContentHeight(min, max);
  int childrenHeight = max - min;
  int boxHeight = (int)mSize.y();

  if (!mAutoScroll)
  {
     mScrollOffset += mLastDirection * (Renderer::Instance().DisplayHeightAsInt() / 200);
     if (mScrollOffset < 0) mScrollOffset = 0;
     if (mScrollOffset > childrenHeight) mScrollOffset = childrenHeight;
  }
  else if (childrenHeight > boxHeight)
  {
    switch (mScrollStep)
    {
      case ScrollSteps::LeftPause:
      {
        mScrollOffset = 0;
        if (mScrollTime > sScrollPause) { mScrollTime = 0; mScrollStep = ScrollSteps::ScrollToRight; }
        break;
      }
      case ScrollSteps::ScrollToRight:
      {
        mScrollOffset = (mScrollTime * sScrollSpeed1) / 1000;
        if (mScrollOffset >= (int)(childrenHeight - boxHeight)) { mScrollTime = 0; mScrollOffset = (int)(childrenHeight - boxHeight); mScrollStep = ScrollSteps::RightPause;}
        break;
      }
      case ScrollSteps::RightPause:
      {
        mScrollOffset = (int)(childrenHeight - boxHeight);
        if (mScrollTime > sScrollPause) { mScrollTime = 0; mScrollStep = ScrollSteps::RollOver; }
        break;
      }
      case ScrollSteps::RollOver:
      {
        mScrollOffset = (int)(childrenHeight - boxHeight) + (mScrollTime * sScrollSpeed2) / 1000;
        if (mScrollOffset >= childrenHeight + boxHeight / 4) { mScrollTime = 0; mScrollOffset = 0; mScrollStep = ScrollSteps::LeftPause;}
        break;
      }
      default: break;
    }
    mScrollTime += deltaTime;
  }

  Component::Update(deltaTime);
}

bool VerticalScrollableContainer::ProcessInput(const InputCompactEvent& event)
{
  if (event.J2DownPressed()) { mLastDirection = 1; mAutoScroll = false; return true; }
  if (event.J2UpPressed()) { mLastDirection = -1; mAutoScroll = false; return true; }
  if (event.J2DownReleased()) { mLastDirection = 0; return true; }
  if (event.J2UpReleased()) { mLastDirection = 0; return true; }

  return Component::ProcessInput(event);
}

void VerticalScrollableContainer::getContentHeight(int& min, int& max)
{
  min = INT32_MAX;
  max = INT32_MIN;
	for (int i = (int)getChildCount(); --i >= 0;)
	{
		if (int t = (int)getChild(i)->getPosition().y(); t < min) min = t;
    if (int t = (int)(getChild(i)->getPosition().y() + getChild(i)->getSize().y()); t > max) max = t;
  }
}

void VerticalScrollableContainer::reset()
{
  mScrollOffset = mScrollTime = mLastDirection = 0;
  mScrollStep =ScrollSteps::LeftPause;
  mAutoScroll = true;
}


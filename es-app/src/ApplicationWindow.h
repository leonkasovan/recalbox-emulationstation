//
// Created by bkg2k on 19/11/2019.
//
#pragma once

#include <WindowManager.h>
#include <systems/SystemManager.h>
#include <views/ViewController.h>

class ApplicationWindow: public WindowManager
{
  private:
    //! View controler
    ViewController mViewController;

    //! OSD image
    ImageComponent mOSD;

    //! Active OSD?
    bool mActiveOSD;

    //! True when the Application window is being closed ASAP
    bool mClosed;

  protected:
    /*!
     * @brief Update the help system.
     * @return True if the update system has been set, false otherwise
     */
    bool UpdateHelpSystem() override;

  public:
    /*!
     * @brief Constructor
     * @param systemManager Systeme manager instance
     */
    explicit ApplicationWindow(SystemManager& systemManager)
      : mViewController(*this, systemManager)
      , mOSD(*this)
      , mActiveOSD(false)
      , mClosed(false)
    {
    }

    bool ProcessInput(const InputCompactEvent& event) override;

    void Update(int deltaTime) override;

    void Render(Transform4x4f& transform) override;

    /*!
     * @brief Return true if the Application window is required to close
     * @return True if the window must close
     */
    [[nodiscard]] bool Closed() const { return mClosed; }

    /*!
     * @brief Check if the given UI is on top of the screen
     * @return True if the given UI is the first visible
     */
    bool AmIOnTopOfScreen(const Gui* ui) const override
    {
      if (HasGui())
        return WindowManager::AmIOnTopOfScreen(ui);
      else
        return &mViewController.CurrentUi() == ui;
    }

    /*!
     * @brief Switch the view controller to the splash screen
     */
    void GoToQuitScreen()
    {
      deleteAllGui();
      mViewController.goToQuitScreen();
      DoWake(); // Avoid screensaver to run
      RenderAll();
    }

    [[nodiscard]] bool DoNotDisturb() const override
    {
      return HasWindowInDoNotDisturb() || mViewController.DoNotDisturb();
    }

    void Rotate(RotationType rotation) override;

    /*!
     * @brief Disable OSD
     * @param imagePath OSD image
     * @param x X coordinate in the range of 0.0 (left) ... 1.0 (right)
     * @param y Y coordinate in the range of 0.0 (up) ... 1.0 (bottom)
     * @param width Width in the range of 0.0 (invisible) ... 1.0 (full screen width)
     * @param height Height in the range of 0.0 (invisible) ... 1.0 (full screen height)
     * @param autoCenter is true, x/y are ignored and the image is screen centered
     */
    void EnableOSDImage(const Path& imagePath, float x, float y, float width, float height, float alpha, bool autoCenter);

    /*!
     * @brief Disable OSD
     */
    void DisableOSDImage();
};

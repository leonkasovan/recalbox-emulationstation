//
// Created by bkg2k on 01/11/2020.
// Modified by davidb2111 for the RG351V board
//

#include "RG351VSpecialButtonsReader.h"
#include <utils/Log.h>
#include <SDL2/SDL.h>
#include <input/InputManager.h>

RG351VSpecialButtonsReader::RG351VSpecialButtonsReader(IBoardInterface& boardInterface)
  : mBoardInterface(boardInterface)
  , mDevice(nullptr)
{
}

void RG351VSpecialButtonsReader::StartReader(Sdl2Runner& sdlRunner)
{
  { LOG(LogDebug) << "[RG351V] In-game special button manager requested to start."; }

  // Create device
  if (InputManager::Instance().DeviceCount() != 0)
    mDevice = &InputManager::Instance().GetDeviceConfigurationFromIndex(0);

  // Register SDL2 events
  int Sdl2EventToRegister[] =
  {
    SDL_JOYBUTTONDOWN,
    SDL_JOYBUTTONUP,
  };
  for(int event : Sdl2EventToRegister)
    sdlRunner.Register(event, this);
}

void RG351VSpecialButtonsReader::StopReader(Sdl2Runner& sdlRunner)
{
  { LOG(LogDebug) << "[RG351V] In-game special button manager requested to stop."; }

  sdlRunner.UnregisterAll(this);
}

void RG351VSpecialButtonsReader::Sdl2EventReceived(const SDL_Event& event)
{
  switch (event.type)
  {
    case SDL_JOYBUTTONDOWN:
    case SDL_JOYBUTTONUP:
    {
      //{ LOG(LogDebug) << "[RG351V] SDL Button Event."; }
      InputEvent inputEvent(event.jbutton.which, InputEvent::EventType::Button, event.jbutton.button, event.jbutton.state == SDL_PRESSED ? 1 : 0);
      //InputManager::LogRawEvent(inputEvent);
      InputCompactEvent compactEvent = mDevice->ConvertToCompact(inputEvent);
      //InputManager::LogCompactEvent(compactEvent);
      mBoardInterface.ProcessSpecialInputs(compactEvent, nullptr);
      break;
    }
    case SDL_QUIT:
    {
      { LOG(LogDebug) << "[RG351V] In-game special button manager receive quit order."; }
      break;
    }
    default: break;
  }
}

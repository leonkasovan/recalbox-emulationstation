//
// Created by bkg2k on 30/03/2020.
//
#pragma once

#include <string>

class Parameters
{
  private:
    //! WWW Root. Default /recalbox/web
    String mWWWRoot;
    //! Root default file
    String mDefaultFile;
    //! Bind tp IP. Default is 0.0.0.0
    String mIP;
    //! Bind to port. Default is 20666
    int mPort;
    //! Number of simultaneous processor (threads)
    int mThreads;
    //! Debug log
    bool mDebug;

  public:
    /*!
     * @brief Default constructor
     */
    Parameters()
      : mWWWRoot("/recalbox/web/manager-v3"),
        mDefaultFile("/index.html"),
        mIP("0.0.0.0"),
        #ifdef DEBUG
        mPort(8081),
        #else
        mPort(81),
        #endif
        mThreads(5),
        mDebug(false)
    {
    }

    /*!
     * @brief Log configuration
     */
    void LogConfig();

    /*
     * Getters
     */

    [[nodiscard]] const String& WWWRoot() const { return mWWWRoot; }
    [[nodiscard]] const String& DefaultFile() const { return mDefaultFile; }
    [[nodiscard]] const String& IP() const { return mIP; }
    [[nodiscard]] int Port() const { return mPort; }
    [[nodiscard]] int Threads() const { return mThreads; }
    [[nodiscard]] bool Debug() const { return mDebug; }
};

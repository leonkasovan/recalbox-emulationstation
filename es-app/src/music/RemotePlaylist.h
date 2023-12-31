//
// Created by bkg2k on 29/05/22.
//
// As part of the RECALBOX Project
// http://www.recalbox.com
//
#pragma once

#include <utils/os/system/Thread.h>
#include <utils/network/HttpMd5.h>
#include <vector>
#include <utils/os/system/Signal.h>
#include <utils/os/fs/Path.h>
#include <utils/cplusplus/StaticLifeCycleControler.h>

class RemotePlaylist : public StaticLifeCycleControler<RemotePlaylist>
                     , private Thread
{
  public:
    //! Little structure to hold track specifications
    class TrackInfo
    {
      public:
        /*
         * Tools
         */

        [[nodiscard]] bool HasValidFile() const { return mPath.Size() == mSize; }

        /*
         * Accesors
         */

        //! Get name
        [[nodiscard]] const String& Name() const { return mName; }
        //! Get duration
        [[nodiscard]] int Duration() const { return mDuration; }
        //! Get file size
        [[nodiscard]] int Size() const { return mSize; }
        //! Get url
        [[nodiscard]] const String& Url() const { return mUrl; }
        //! Get md5
        [[nodiscard]] const String& Md5() const { return mMd5; }
        //! Get album
        [[nodiscard]] const String& Album() const { return mAlbum; }
        //! Get mix tape
        [[nodiscard]] const String& MixTaper() const { return mMixTape; }
        //! Get local path
        [[nodiscard]] const Path& LocalPath() const { return mPath; }

      private:
        String mName;     //!< Track ,name
        int         mDuration; //!< Duration in second
        int         mSize;     //!< File size
        String mUrl;      //!< Track url
        String mMd5;      //!< File md5
        String mAlbum;    //!< File md5
        String mMixTape;  //!< File md5
        Path        mPath;     //! Local file path

        /*!
         * @brief Constructor
         * @param name Track name
         * @param duration Track duration
         * @param size File size
         * @param url Track url
         * @param md5 File md5
         * @param album Album name
         * @param mixTape Mix tape name
         */
        TrackInfo(const String& name,
                  int duration,
                  int size,
                  const String& url,
                  const String& md5,
                  const String& album,
                  const String& mixTape)
          : mName(name)
          , mDuration(duration)
          , mSize(size)
          , mUrl(url)
          , mMd5(md5)
          , mAlbum(album)
          , mMixTape(mixTape)
          , mPath(Path(sStorageRootPath) / md5)
        {
        }

        //! Allow parent class to use the constructor
        friend class RemotePlaylist;
    };

    //! Constructor
    RemotePlaylist();

    //! Destructor
    ~RemotePlaylist() override;

    /*!
     * @brief Get next track file to play
     * @return Next track structure or nullptr
     */
    TrackInfo* GetNextTrack();

    /*!
     * @brief Last track returned by GetNextTrack() has been played. Remove it
     */
    void TrackConsumed();

  private:
    //! Url where to get playlist
    static constexpr const char* sRemotePlayListUrl = "https://api-patreon.recalbox.com/playlist";
    //! Storage root path - no file except music must be put there!
    static constexpr const char* sStorageRootPath = "/overlay/.music/.cache";
    //! Index path
    static constexpr const char* sStorageIndex = "/overlay/.music/.index";
    //! Playlist cache path
    static constexpr const char* sStorageList = "/overlay/.music/.list";
    //! Maximum cached track
    static constexpr int sMaxCachedFile = 10;

    //! Http request
    HttpMd5 mRequest;

    //! Action signal
    Signal mAction;

    //! Playlist storage
    std::vector<TrackInfo> mPlaylist;
    //! Storage root
    Path mStorageRoot;
    //! Index locker
    Mutex mLocker;
    //! Next play index
    int mNextIndex;

    /*!
     * @brief Update playlist
     * @result True if the playlist was successfuly fetched, false otherwise
     */
    bool UpdatePlayList();

    /*!
     * @brief Deserialize playlist into TrackInfo structures
     * @param jsonString JSON Playlist
     * @return True on success, false otherwise
     */
    bool DeserializePlaylist(const String& jsonString);

    /*!
     * @brief Check files on disk, delete older/bad files and load current index
     */
    void InitializeIndex();

    /*!
     * @brief Update file index using current track index
     */
    void UpdateIndex();

    /*!
     * @brief Lookup a file in the database given its md5
     * @param md5 md5 to lookup
     * @return Index in database or -1 if no track is found
     */
    int LookupTrackIndex(const String& md5);

    /*!
     * @brief Check old files and delete them all
     */
    void SanitizeFiles();

    /*!
     * @brief Download and cache files
     */
    void DownloadFiles();

    /*
     * Thread implementation
     */

    //! Run loop
    void Run() override;
};

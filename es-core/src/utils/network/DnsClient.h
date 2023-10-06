//
// Created by bkg2k on 31/03/2022.
//
// As part of the RECALBOX Project
// http://www.recalbox.com
//
#pragma once

#include <utils/String.h>
#include <utils/datetime/DateTime.h>
#include <utils/storage/HashMap.h>

class DnsClient
{
  public:
    //! Get Txt record content from the given domain
    String GetTxtRecord(const String& domain);

    //! Get A record content from the given domain
    String GetARecord(const String& domain);

  private:
    //! Cache entry
    class Entry
    {
      public:
        // Record cache structure
        class RecordCache
        {
          public:
            //! Default constructor
            RecordCache() = default;

            /*!
             * @brief Set cache data
             * @param data Data to cache. Timestamped now.
             */
            void Set(const String& data)
            {
              mRecordContent  = data;       // Recoprd data
              mRecordDateTime = DateTime(); // Now
            }

            //! Get data
            const String& Data() const { return mRecordContent; }
            //! Get timestamp
            const DateTime& TimeStamp() const { return mRecordDateTime; }

            //! Valid cache?
            bool IsValid() const { return !mRecordContent.empty() && (DateTime() - mRecordDateTime).TotalHours() < 8; }

          private:
            //! Record's cached data
            String mRecordContent;
            //! Record's cache datetime
            DateTime mRecordDateTime;
        };

        //! Default constructor
        Entry() = default;

        //! Set A record
        void SetARecord(const String& data) { mARecord.Set(data); }
        //! Set Txt record
        void SetTxtRecord(const String& data) { mTxtRecord.Set(data); }

        //! Get A record
        const RecordCache& ARecord() const { return mARecord; }
        //! Get Txt record
        const RecordCache& TxtRecord() const { return mTxtRecord; }

      private:
        //! A Record cache
        RecordCache mARecord;
        //! Txt Record cache
        RecordCache mTxtRecord;
    };

    //! Record cache
    HashMap<String, Entry> mCache;
};




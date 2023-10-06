//
// Created by bkg2k on 28/12/2019.
//
#pragma once

#include <utils/String.h>
#include <vector>
#include <bios/Bios.h>
#include <utils/Xml.h>

class BiosList
{
  private:
    //! System full name
    String mSystemFullName;
    //! System short name
    String mSystemName;

    //! Bios list
    std::vector<Bios> mBiosList;

  public:
    /*!
     * @brief Default constructor
     */
    BiosList() = default;

    /*!
     * @brief Copy constructor
     * @param source Source to copy from
     */
    BiosList(const BiosList& source) = default;

    /*!
     * @brief Move constructor
     * @param source Source to move from
     */
    BiosList(BiosList&& source) = default;

    /*!
     * @brief Move operator (required by std::sort)
     * @param source Source to move from
     */
    BiosList& operator =(BiosList&& source) = default;

    /*!
     * @brief Deserialize constructor
     * @param systemNode System node to deserialize from
     */
    explicit BiosList(XmlNode& systemNode);

    //! Get bios count for the current system
    int BiosCount() const { return mBiosList.size(); }

    /*!
     * @brief Get Bios at the given index (from 0 to BiosCounbt() - 1)
     * @param index Index
     * @return Bios object reference
     */
    const Bios& BiosAt(int index) const { return mBiosList[index]; }

    /*!
     * @brief Scan the bios file of the bios object at the given index and refresh its properties
     * @param index Index
     */
    void ScanAt(int index) { mBiosList[index].Scan(); }

    /*!
     * @brief Get missing bios file list
     * @return File list
     */
    [[nodiscard]] String::List GetMissingBiosFileList() const;

    /*!
     * @brief Generate missing bios report
     * @return Text Report
     */
    [[nodiscard]] String GenerateReport() const;

    /*
     * Accessors
     */

    //! Get system long name
    const String& FullName() const { return mSystemFullName; }
    //! Get system short name
    const String& Name() const { return mSystemName; }

    /*!
     * @brief Get summary status of this bios list
     * @return Resport status
     */
    Bios::ReportStatus ReportStatus() const;

    //! Get total bios Ok
    int TotalBiosOk() const;
    //! Get total bios Ko
    int TotalBiosKo() const;
    //! Get total bios unsafe
    int TotalBiosUnsafe() const;

    //! Get total bios not found
    int TotalFileNotFound() const;
    //! Get total bios found with a matching hash
    int TotalHashMatching() const;
    //! Get total bios found with a non-matching hash
    int TotalHashNotMatching() const;
};

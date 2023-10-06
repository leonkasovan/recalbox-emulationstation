//
// Created by gugue_u on 20/02/2021.
//

#pragma once

#include <utils/String.h>

enum class Languages
{
    EN,
    ES,
    PT,
    FR,
    DE,
    IT,
    NL,
    JA,
    ZH,
    KO,
    RU,
    DA,
    FI,
    SV,
    HU,
    NO,
    PL,
    CZ,
    SK,
    TR,
    Unknown,
};

class LanguagesTools
{
  public:

    static const String& LanguagesFullName(Languages language);

    typedef std::vector<Languages> List;
    static const List& AvailableLanguages();


    static Languages DeserializeLanguage(const String& language);
    static const String& SerializeLanguage(Languages language);


    static Languages LanguageFromString(const String& languages);

    static const String& LanguageFromEnum(Languages languages);

    static Languages GetScrapingLanguage();
};
#pragma once

#include <map>
#include <deque>
#include <random>
#include <utils/String.h>
#include <utils/os/fs/Path.h>
#include <RecalboxConf.h>
#include "pugixml/pugixml.hpp"
#include "ThemeElement.h"

template<typename T> class TextListComponent;

class Component;
class Sound;
class ImageComponent;
class NinePatchComponent;
class TextComponent;
class WindowManager;

class ThemeSet
{
  private:
    Path mPath;

  public:
    ThemeSet() = default;
    explicit ThemeSet(const Path& path) : mPath(path) {}
    inline String getName() const { return mPath.FilenameWithoutExtension(); }
    inline Path getThemePath(const String& system) const { return mPath / system / "theme.xml"; }
};

class ThemeData
{
  private:
    class ThemeView
    {
      public:
        HashMap<String, ThemeElement> elements;
        String::List orderedKeys;
    };

    static ThemeData* sCurrent;
    static bool sThemeChanged;
    static bool sThemeHasMenuView;
    static bool sThemeHasHelpSystem;

  public:

    ThemeData();

    static bool ThemeHasMenuView() { return sThemeHasMenuView; }
    static bool ThemeHasHelpSystem() { return sThemeHasHelpSystem; }

    static void SetThemeHasMenuView(bool on) { sThemeHasMenuView = on; }
    static void SetThemeHasHelpSystem(bool on) { sThemeHasHelpSystem = on; }

  	// throws ThemeException
	  void loadFile(const String& systemThemeFolder, const Path& path);

    enum class ElementProperty
    {
      NormalizedPair,
      Path,
      String,
      Color,
      Float,
      Boolean
    };

    // If expectedType is an empty string, will do no type checking.
    const ThemeElement* getElement(const String& view, const String& element, const String& expectedType) const;

    static std::vector<Component*> makeExtras(const ThemeData& theme, const String& view, WindowManager& window);

    //static const ThemeData& getDefault();
    static const ThemeData& getCurrent();
    static void SetThemeChanged(bool themeChanged);
    static bool IsThemeChanged();
    String getGameClipView() const;
    static const char *getNoTheme() { return "0 - DEFAULT"; }

    static HashMap<String, ThemeSet> getThemeSets();
	static HashMap<String, String> getThemeSubSets(const String& theme);
	static String::List sortThemeSubSets(const HashMap<String, String>& subsetmap, const String& subset);
	static Path getThemeFromCurrentSet(const String& system);
	String getTransition() const;

    bool getHasFavoritesInTheme() const
    { return (mVersion >= CURRENT_THEME_FORMAT_VERSION); }
    bool isFolderHandled() const;

    static constexpr int MINIMUM_THEME_FORMAT_VERSION = 3;
    static constexpr int CURRENT_THEME_FORMAT_VERSION = 4;

  private:
    static HashMap<String, HashMap<String, ElementProperty>>& ElementMap();
    static String::List& SupportedFeatures();
    static String::List& SupportedViews();

	std::deque<Path> mPaths;
	float mVersion;
	String mColorset;
	String mIconset;
	String mMenu;
	String mSystemview;
	String mGamelistview;
	String mRegion;
	String mGameClipView;
	String mSystemThemeFolder;
	String mRandomPath;
	static constexpr const char* sRandomMethod = "$random(";


    void parseFeatures(const pugi::xml_node& themeRoot);
    void parseIncludes(const pugi::xml_node& themeRoot);
    void parseViews(const pugi::xml_node& themeRoot);
    void parseView(const pugi::xml_node& viewNode, ThemeView& view);
    void parseElement(const pugi::xml_node& elementNode, const HashMap<String, ElementProperty>& typeMap, ThemeElement& element);
    bool parseRegion(const pugi::xml_node& root);
    bool parseSubset(const pugi::xml_node& node);
    static void crawlIncludes(const pugi::xml_node& root, HashMap<String, String>& sets, std::deque<Path>& dequepath);
    static void findRegion(const pugi::xml_document& doc, HashMap<String, String>& sets);

    static bool CheckThemeOption(String& selected, const HashMap<String, String>& subsets, const String& subset);
    static String resolveSystemVariable(const String& systemThemeFolder, const String& path, String& randomPath)
    {
      String lccc = RecalboxConf::Instance().GetSystemLanguage().LowerCase();
      String lc = "en";
      String cc = "us";
      if (lccc.size() >= 5)
      {
        int pos = lccc.Find('_');
        if (pos >=2 && pos < (int)lccc.size() - 1)
        {
          lc = lccc.SubString(0, pos);
          cc = lccc.SubString(pos + 1);
        }
      }

      String result = path;
      result.Replace("$system", systemThemeFolder)
            .Replace("$language", lc)
            .Replace("$country", cc);

      return PickRandomPath(result, randomPath);;
    }

    static String PickRandomPath(const String& value, String& randomPath)
    {
      if (!value.Contains(sRandomMethod))
        return value;

      String args;
      if (value.Extract( sRandomMethod, ")", args, true))
        if (randomPath.empty())
        {
          String::List paths = args.Split(',');
          std::random_device rd;
          std::default_random_engine engine(rd());
          const int max = (int)paths.size();
          std::uniform_int_distribution<int> distrib{0, max-1};
          randomPath = paths[distrib(engine)];
        }

      return String(value).Replace(sRandomMethod + args + ')', randomPath);
    }

    HashMap<String, ThemeView> mViews;
};

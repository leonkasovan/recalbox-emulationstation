#pragma once

#include <utils/String.h>
#include <games/MetadataType.h>

//! Method pointer to default value getters
typedef bool (MetadataDescriptor::*IsDefaultValueMethodType)() const;
//! Method pointer to string getters
typedef String (MetadataDescriptor::*GetValueMethodType)() const;
//! Method pointer to string setters
typedef void (MetadataDescriptor::*SetValueMethodType)(const String& value);
//! Method pointer to string setters
typedef void (MetadataDescriptor::*SetPathMethodType)(const Path& value);

class MetadataFieldDescriptor
{
  public:
    //! Edition type
    enum class EditableType
    {
        None,      //! Not editable
        List,      //!< Single-selection list
        ListMulti, //!< Multi-selection list
        Text,      //!< Simple text edit
        Switch,    //!< Switch on/off
        Date,      //!< Date picker
        Rating,    //!< Special rating
    };

    //! Data type
    enum class DataType
    {
        // Simple types
        String,  //!< String
        Int,     //!< int
        Short,   //!< short
        Bool,    //!< bool
        Float,   //!< float

        // Derived types
        Text,    //!< Multiline text (String)
        List,    //!< Simple text from a fixed list
        Path,    //!< File path (String)
        Rating,  //!< Floating point value between 0.0 and 1.0 (float)
        Date,    //!< Epoc (int)
        Range,   //!< Integer range: LSW:from MSW:to
        Crc32,   //!< 4byte hash (int)
    };

  private:
    String               _Key;                  //!< Identifier
    String               _DefaultValue;         //!< default value
    String               _DisplayName;          //!< displayed as this in editors
    String               _DisplayPrompt;        //!< phrase displayed in editors when prompted to enter value (currently only for strings)
    MetadataType              _MetadataType;         //!< Named metadata type
    DataType                  _DataType;             //!< Datatype
    EditableType              _EditType;             //!< Editable type
    IsDefaultValueMethodType  _IsDefaultValueMethod; //!< Is Default value?
    GetValueMethodType        _GetMethod;            //!< String getter
    SetValueMethodType        _SetMethod;            //!< String getter
    bool                      _IsStatistic;          //!< if true, ignore scraper values for this metadata
    bool                      _IsMain;               //!< if true, display on main metadata editor GUI, else in secondary

  public:
    // Public const accessors
    [[nodiscard]] const String&        Key()                  const { return _Key;                  } //!< Identifier
    [[nodiscard]] const String&        DefaultValue()         const { return _DefaultValue;         } //!< default value
    [[nodiscard]] const String&        DisplayName()          const { return _DisplayName;          } //!< displayed as this in editors
    [[nodiscard]] const String&        DisplayPrompt()        const { return _DisplayPrompt;        } //!< phrase displayed in editors when prompted to enter value (currently only for strings)
    [[nodiscard]] MetadataType              MetaType()             const { return _MetadataType;         } //!< Named metadata type
    [[nodiscard]] DataType                  Type()                 const { return _DataType;             } //!< Datatype
    [[nodiscard]] EditableType              EditType()             const { return _EditType;             } //!< Editable type
    [[nodiscard]] IsDefaultValueMethodType  IsDefaultValueMethod() const { return _IsDefaultValueMethod; } //!< Is Default value?
    [[nodiscard]] GetValueMethodType        GetValueMethod()       const { return _GetMethod;            } //!< String getter
    [[nodiscard]] SetValueMethodType        SetValueMethod()       const { return _SetMethod;            } //!< String setter
    [[nodiscard]] bool                      IsStatistic()          const { return _IsStatistic;          } //!< if true, ignore scraper values for this metadata
    [[nodiscard]] bool                      IsMain()               const { return _IsMain;               } //!< if true, display on main metadata editor GUI, else in secondary

    //! Constructor
    MetadataFieldDescriptor(const String&        key,
                            const String&        defaultValue,
                            const String&        displayName,
                            const String&        displayPrompt,
                            MetadataType              metadataType,
                            DataType                  type,
                            EditableType              edittype,
                            IsDefaultValueMethodType  isDefaultValueMethod,
                            GetValueMethodType        getMethod,
                            SetValueMethodType        setMethod,
                            bool                      isStatistic,
                            bool                      isMain)
      : _Key(key)
      , _DefaultValue(defaultValue)
      , _DisplayName(displayName)
      , _DisplayPrompt(displayPrompt)
      , _MetadataType(metadataType)
      , _DataType(type)
      , _EditType(edittype)
      , _IsDefaultValueMethod(isDefaultValueMethod)
      , _GetMethod(getMethod)
      , _SetMethod(setMethod)
      , _IsStatistic(isStatistic)
      , _IsMain(isMain)
    {
    }
};


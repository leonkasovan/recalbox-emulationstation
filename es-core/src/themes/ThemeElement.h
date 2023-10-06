//
// Created by bkg2k on 09/11/2019.
//
#pragma once

#include <map>
#include <utils/String.h>

#include <utils/math/Vector2f.h>

class ThemeElement
{
  private:
    //! Property type
    enum class Type: char
    {
      Vector,
      String,
      Integer,
      Float,
      Bool,
    };
    //! Bag of data
    class PropertyBag
    {
      private:
        String mString;
        union // Save some space by union-ing cardinal exclusive values
        {
          int mInteger;
          float mFloat;
          bool mBool;
        };
        float mSecondFloat;
        Type mType;
      public:
        PropertyBag()                              : mInteger(0), mSecondFloat(0.0f), mType(Type::String) {}
        explicit PropertyBag(const Vector2f& v)    : mFloat(v.x()), mSecondFloat(v.y()), mType(Type::Vector) {}
        explicit PropertyBag(float v1, float v2)   : mFloat(v1), mSecondFloat(v2), mType(Type::Vector) {}
        explicit PropertyBag(const String& s) : mString(s), mInteger(0), mSecondFloat(0.0f), mType(Type::String) {}
        explicit PropertyBag(int v)                : mInteger(v), mSecondFloat(0.0f), mType(Type::Integer) {}
        explicit PropertyBag(float v)              : mFloat(v), mSecondFloat(0.0f), mType(Type::Float) {}
        explicit PropertyBag(bool v)               : mBool(v), mSecondFloat(0.0f), mType(Type::Bool) {}

        [[nodiscard]] String AsString() const
        {
          switch(mType)
          {
            case Type::Vector: return String(mFloat, 4).Append(' ').Append(mSecondFloat, 4);
            case Type::String: return mString;
            case Type::Integer: return String(mInteger);
            case Type::Float: return String(mFloat, 4);
            case Type::Bool: return String(mBool ? '1' : '0');
          }
          return String();
        }
        [[nodiscard]] int AsInt() const
        {
          switch(mType)
          {
            case Type::String: { int result = 0; return mString.TryAsInt(result) ? result : 0; }
            case Type::Integer: return mInteger;
            case Type::Vector:
            case Type::Float: return (int)mFloat;
            case Type::Bool: return (int)mBool;
          }
          return 0;
        }
        [[nodiscard]] float AsFloat() const
        {
          switch(mType)
          {
            case Type::String: { float result = 0; return mString.TryAsFloat(result) ? result : 0.0f; }
            case Type::Integer: return (float)mInteger;
            case Type::Vector:
            case Type::Float: return mFloat;
            case Type::Bool: return (float)mBool;
          }
          return 0.0f;
        }
        [[nodiscard]] bool AsBool() const
        {
          switch(mType)
          {
            case Type::String: { bool result = false; return mString.TryAsBool(result) ? result : false; }
            case Type::Integer: return (bool)mInteger;
            case Type::Vector:
            case Type::Float: return (bool)mFloat;
            case Type::Bool: return mBool;
          }
          return false;
        }
        [[nodiscard]] Vector2f AsVector() const
        {
          switch(mType)
          {
            case Type::Vector: return { mFloat, mSecondFloat };
            case Type::String:
            {
              float x = 0, y = 0;
              if (mString.TryAsFloat(0, ' ', x))
              {
                int pos = mString.Find(' ');
                if (pos >= 0)
                  if (mString.TryAsFloat((int) pos + 1, 0, y))
                    return { x, y };
              }
              break;
            }
            case Type::Integer: return { (float)mInteger, (float)mInteger };
            case Type::Float: return { mFloat, mFloat };
            case Type::Bool: return {(float)mBool, (float)mBool};
          }
          return { 0.0f, 0.0f };
        }
    };

    std::map<String, PropertyBag> mProperties;
    String mType;
    bool mExtra;

  public:
    [[nodiscard]] String AsString(const String& name) const
    {
      auto it = mProperties.find(name);
      if (it != mProperties.end()) return it->second.AsString();
      return String();
    }

    [[nodiscard]] int AsInt(const String& name) const
    {
      auto it = mProperties.find(name);
      if (it != mProperties.end()) return it->second.AsInt();
      return 0;
    }

    [[nodiscard]] float AsFloat(const String& name) const
    {
      auto it = mProperties.find(name);
      if (it != mProperties.end()) return it->second.AsFloat();
      return 0.0f;
    }

    [[nodiscard]] bool AsBool(const String& name) const
    {
      auto it = mProperties.find(name);
      if (it != mProperties.end()) return it->second.AsBool();
      return false;
    }

    [[nodiscard]] Vector2f AsVector(const String& name) const
    {
      auto it = mProperties.find(name);
      if (it != mProperties.end()) return it->second.AsVector();
      return { 0.0f, 0.0f };
    }

    [[nodiscard]] bool HasProperty(const String& prop) const { return (mProperties.find(prop) != mProperties.end()); }

    [[nodiscard]] bool HasProperties() const { return !mProperties.empty(); }

    [[nodiscard]] const String& Type() const { return mType; }

    [[nodiscard]] bool Extra() const { return mExtra; }

    void SetRootData(const String& type, bool extra)
    {
      mType = type;
      mExtra = extra;
    }

    void AddVectorProperty(const String& name, float x, float y) { mProperties[name] = PropertyBag(x, y); }
    void AddVectorProperty(const String& name, const Vector2f& v) { mProperties[name] = PropertyBag(v); }
    void AddStringProperty(const String& name, const String& s) { mProperties[name] = PropertyBag(s); }
    void AddIntProperty(const String& name, int v) { mProperties[name] = PropertyBag(v); }
    void AddFloatProperty(const String& name, float v) { mProperties[name] = PropertyBag(v); }
    void AddBoolProperty(const String& name, bool v) { mProperties[name] = PropertyBag(v); }
};


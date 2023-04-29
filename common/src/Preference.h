/*
 Copyright (C) 2010-2017 Kristian Duske

 This file is part of TrenchBroom.

 TrenchBroom is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 TrenchBroom is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with TrenchBroom. If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include "IO/Path.h"
#include "Macros.h"
#include "View/KeyboardShortcut.h"

#include <optional>

#include <QJsonValue>
#include <QString>
#include <QTextStream>

class QKeySequence;

namespace TrenchBroom
{
class Color;

class PrefSerializer
{
public:
  virtual ~PrefSerializer();

  virtual bool readFromJSON(const QJsonValue& in, bool& out) const = 0;
  virtual bool readFromJSON(const QJsonValue& in, Color& out) const = 0;
  virtual bool readFromJSON(const QJsonValue& in, float& out) const = 0;
  virtual bool readFromJSON(const QJsonValue& in, int& out) const = 0;
  virtual bool readFromJSON(const QJsonValue& in, IO::Path& out) const = 0;
  virtual bool readFromJSON(const QJsonValue& in, QKeySequence& out) const = 0;
  virtual bool readFromJSON(const QJsonValue& in, QString& out) const = 0;

  virtual QJsonValue writeToJSON(bool in) const = 0;
  virtual QJsonValue writeToJSON(const Color& in) const = 0;
  virtual QJsonValue writeToJSON(float in) const = 0;
  virtual QJsonValue writeToJSON(int in) const = 0;
  virtual QJsonValue writeToJSON(const IO::Path& in) const = 0;
  virtual QJsonValue writeToJSON(const QKeySequence& in) const = 0;
  virtual QJsonValue writeToJSON(const QString& in) const = 0;
};


/**
 * Used by Qt version of TrenchBroom
 *
 * - bool serializes to JSON bool
 * - float and int serializes to JSON double
 * - QKeySequence serializes to JSON string, but with a different format than wxWidgets
 * - other types are not overridden (Color, IO::Path, QString) so serialize to JSON string
 * using the same format as wxWidgets
 */
class PreferenceSerializer : public PrefSerializer
{
public:
  bool readFromJSON(const QJsonValue& in, bool& out) const override;
  bool readFromJSON(const QJsonValue& in, Color& out) const override;
  bool readFromJSON(const QJsonValue& in, float& out) const override;
  bool readFromJSON(const QJsonValue& in, int& out) const override;
  bool readFromJSON(const QJsonValue& in, IO::Path& out) const override;
  bool readFromJSON(const QJsonValue& in, QKeySequence& out) const override;
  bool readFromJSON(const QJsonValue& in, QString& out) const override;

  QJsonValue writeToJSON(bool in) const override;
  QJsonValue writeToJSON(const Color& in) const override;
  QJsonValue writeToJSON(float in) const override;
  QJsonValue writeToJSON(int in) const override;
  QJsonValue writeToJSON(const IO::Path& in) const override;
  QJsonValue writeToJSON(const QKeySequence& in) const override;
  QJsonValue writeToJSON(const QString& in) const override;
};

class PreferenceBase
{
public:
  PreferenceBase();
  virtual ~PreferenceBase();

  PreferenceBase(const PreferenceBase& other);
  PreferenceBase(PreferenceBase&& other) noexcept;
  PreferenceBase& operator=(const PreferenceBase& other);
  PreferenceBase& operator=(PreferenceBase&& other);

  friend bool operator==(const PreferenceBase& lhs, const PreferenceBase& rhs);
  friend bool operator!=(const PreferenceBase& lhs, const PreferenceBase& rhs);

  virtual const IO::Path& path() const = 0;

public: // private to PreferenceManager
  virtual void resetToDefault() = 0;
  virtual bool valid() const = 0;
  virtual void setValid(bool _valid) = 0;
  virtual bool loadFromJSON(const PrefSerializer& format, const QJsonValue& value) = 0;
  virtual QJsonValue writeToJSON(const PrefSerializer& format) const = 0;
  virtual bool isDefault() const = 0;
};

class DynamicPreferencePatternBase
{
public:
  virtual ~DynamicPreferencePatternBase();
  virtual const IO::Path& pathPattern() const = 0;
};

template <typename T>
class DynamicPreferencePattern : public DynamicPreferencePatternBase
{
private:
  IO::Path m_pathPattern;

public:
  explicit DynamicPreferencePattern(IO::Path pathPattern)
    : m_pathPattern{std::move(pathPattern)}
  {
  }

  const IO::Path& pathPattern() const override { return m_pathPattern; }
};

/**
 * Stores the current value and default value of a preference, in deserialized form.
 * No public API for reading/writing the value, use PreferenceManager instead.
 */
template <typename T>
class Preference : public PreferenceBase
{
private:
  IO::Path m_path;
  T m_defaultValue;
  T m_value;
  bool m_valid;
  bool m_readOnly;

public:
  Preference(IO::Path path, const T& defaultValue, const bool readOnly = false)
    : m_path{std::move(path)}
    , m_defaultValue{defaultValue}
    , m_value{m_defaultValue}
    , m_valid{false}
    , m_readOnly{readOnly}
  {
  }

  Preference(const Preference& other) = default;

  // cannot be noexcept because it will call QKeySequence's copy constructor
  Preference(Preference&& other) = default;

  Preference& operator=(const Preference& other) = default;
  Preference& operator=(Preference&& other) = default;

  const IO::Path& path() const override { return m_path; }

  const T& defaultValue() const { return m_defaultValue; }

public: // PreferenceManager private
  void setValue(const T& value)
  {
    assert(!m_readOnly);
    m_value = value;
  }

  void resetToDefault() override { m_value = m_defaultValue; }

  bool valid() const override { return m_valid; }

  void setValid(const bool _valid) override { m_valid = _valid; }

  const T& value() const
  {
    assert(m_valid);
    return m_value;
  }

  bool loadFromJSON(const PrefSerializer& format, const QJsonValue& value) override
  {
    auto result = T{};
    if (format.readFromJSON(value, result))
    {
      m_value = result;
      return true;
    }
    return false;
  }

  QJsonValue writeToJSON(const PrefSerializer& format) const override
  {
    return format.writeToJSON(value());
  }

  bool isDefault() const override { return m_defaultValue == m_value; }

  bool isReadOnly() const { return m_readOnly; }
};
} // namespace TrenchBroom

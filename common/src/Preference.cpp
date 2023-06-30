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

#include "Preference.h"

#include <QJsonObject>
#include <QJsonValue>
#include <QKeySequence>

#include "Color.h"
#include "IO/PathQt.h"

namespace TrenchBroom
{
// PreferenceSerializer

bool PreferenceSerializer::readFromJson(const QJsonValue& in, bool& out) const
{
  if (!in.isBool())
  {
    return false;
  }
  out = in.toBool();
  return true;
}

bool PreferenceSerializer::readFromJson(const QJsonValue& in, Color& out) const
{
  if (!in.isString())
  {
    return false;
  }

  if (const auto color = Color::parse(in.toString().toStdString()))
  {
    out = *color;
    return true;
  }

  return false;
}

bool PreferenceSerializer::readFromJson(const QJsonValue& in, float& out) const
{
  if (!in.isDouble())
  {
    return false;
  }
  out = static_cast<float>(in.toDouble());
  return true;
}

bool PreferenceSerializer::readFromJson(const QJsonValue& in, int& out) const
{
  if (!in.isDouble())
  {
    return false;
  }
  out = static_cast<int>(in.toDouble());
  return true;
}

bool PreferenceSerializer::readFromJson(
  const QJsonValue& in, std::filesystem::path& out) const
{
  if (!in.isString())
  {
    return false;
  }

  out = IO::pathFromQString(in.toString());
  return true;
}

bool PreferenceSerializer::readFromJson(const QJsonValue& in, QKeySequence& out) const
{
  if (!in.isString())
  {
    return false;
  }
  out = QKeySequence{in.toString(), QKeySequence::PortableText};
  return true;
}

bool PreferenceSerializer::readFromJson(const QJsonValue& in, QString& out) const
{
  if (!in.isString())
  {
    return false;
  }

  out = in.toString();
  return true;
}

QJsonValue PreferenceSerializer::writeToJson(const bool in) const
{
  return {in};
}

namespace
{
template <typename T, typename L>
QJsonValue toJson(
  const T& in, const L& serialize = [](QTextStream& lhs, const T& rhs) { lhs << rhs; })
{
  // NOTE: QTextStream's default locale is C, unlike QString::arg()
  auto string = QString{};
  auto stream = QTextStream{&string};
  serialize(stream, in);
  return {string};
}

template <typename T>
QJsonValue toJson(const T& in)
{
  return toJson(in, [](QTextStream& lhs, const T& rhs) { lhs << rhs; });
}
} // namespace

QJsonValue PreferenceSerializer::writeToJson(const Color& in) const
{
  return toJson(in, [](QTextStream& lhs, const Color& rhs) {
    lhs << rhs.r() << " " << rhs.g() << " " << rhs.b() << " " << rhs.a();
  });
}

QJsonValue PreferenceSerializer::writeToJson(const float in) const
{
  return {static_cast<double>(in)};
}

QJsonValue PreferenceSerializer::writeToJson(const int in) const
{
  return {in};
}

QJsonValue PreferenceSerializer::writeToJson(const std::filesystem::path& in) const
{
  return toJson(in, [](auto& lhs, const auto& rhs) { lhs << IO::pathAsQString(rhs); });
}

QJsonValue PreferenceSerializer::writeToJson(const QKeySequence& in) const
{
  return {in.toString(QKeySequence::PortableText)};
}

QJsonValue PreferenceSerializer::writeToJson(const QString& in) const
{
  return toJson(in);
}


PreferenceBase::PreferenceBase() = default;

PreferenceBase::PreferenceBase(const PreferenceBase& other) = default;
PreferenceBase::PreferenceBase(PreferenceBase&& other) noexcept = default;
PreferenceBase& PreferenceBase::operator=(const PreferenceBase& other) = default;
PreferenceBase& PreferenceBase::operator=(PreferenceBase&& other) = default;

bool operator==(const PreferenceBase& lhs, const PreferenceBase& rhs)
{
  return &lhs == &rhs;
}

bool operator!=(const PreferenceBase& lhs, const PreferenceBase& rhs)
{
  return !(lhs == rhs);
}

PreferenceBase::~PreferenceBase() = default;

DynamicPreferencePatternBase::~DynamicPreferencePatternBase() = default;
} // namespace TrenchBroom

/*
 Copyright (C) 2010 Kristian Duske

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

#include <QJsonValue>
#include <QString>

#include <fmt/format.h>

#include <catch2/catch_test_macros.hpp>

// These are so Catch can print Qt types

namespace Catch
{

template <>
struct StringMaker<QString>
{
  static std::string convert(QString const& value) { return qUtf8Printable(value); }
};

template <>
struct StringMaker<QJsonValue>
{
  static std::string convert(QJsonValue const& value)
  {
    const auto asVariant = value.toVariant();
    return fmt::format(
      "QJsonValue<{}>({})", asVariant.typeName(), qUtf8Printable(asVariant.toString()));
  }
};

} // namespace Catch

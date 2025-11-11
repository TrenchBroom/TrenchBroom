
/*
 Copyright (C) 2025 Kristian Duske

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

#include <QString>

#include <optional>

#include <catch2/catch_test_macros.hpp>

class QFile;

namespace upd
{

std::optional<QString> readFileIntoString(QFile& file);
std::optional<QString> readFileIntoString(const QString& path);

} // namespace upd

namespace Catch
{

template <>
struct StringMaker<QString>
{
  static std::string convert(const QString& qString) { return qString.toStdString(); }
};

} // namespace Catch
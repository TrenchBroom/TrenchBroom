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

#include <QLocale>
#include <QString>

#include "vm/vec.h"

#include <optional>

namespace tb::ui
{
namespace detail
{

template <std::floating_point T, size_t S>
std::optional<vm::vec<T, S>> parse(const QStringList& parts, const QLocale& locale)
{
  auto result = vm::vec<T, S>{};
  for (size_t i = 0; i < S; ++i)
  {
    auto ok = false;
    result[i] = static_cast<T>(locale.toDouble(parts[qsizetype(i)], &ok));
    if (!ok)
    {
      return std::nullopt;
    }
  }

  return result;
}

} // namespace detail

template <typename T>
QString toString(const vm::vec<T, 3>& vec)
{
  return QString{"%L1 %L2 %L3"}.arg(vec.x()).arg(vec.y()).arg(vec.z());
}

template <std::floating_point T, size_t S>
std::optional<vm::vec<T, S>> parse(const QString& str)
{
  const auto parts = str.split(' ', Qt::SkipEmptyParts);
  if (parts.size() != S)
  {
    return std::nullopt;
  }

  if (const auto result = detail::parse<T, S>(parts, QLocale{}))
  {
    return result;
  }

  // try to parse as english format to allow pasting from compiler output and such:
  return detail::parse<T, S>(parts, QLocale::c());
}

template <std::integral T, size_t S>
std::optional<vm::vec<T, S>> parse(const QString& str)
{
  const auto parts = str.split(' ', Qt::SkipEmptyParts);
  if (parts.size() != S)
  {
    return std::nullopt;
  }

  const auto locale = QLocale{};
  auto result = vm::vec<T, S>{};
  for (size_t i = 0; i < S; ++i)
  {
    auto ok = false;
    result[i] = static_cast<T>(locale.toLong(parts[qsizetype(i)], &ok));
    if (!ok)
    {
      return std::nullopt;
    }
  }
  return result;
}

} // namespace tb::ui

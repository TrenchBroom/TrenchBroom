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

#include <QLocale>

#include "ui/QtUtils.h"

#include "vm/vec.h"

#include <optional>

#include "Catch2.h" // IWYU pragma: keep

namespace tb::ui
{
namespace
{
class OverrideLocale
{
private:
  QLocale m_previousLocale;

public:
  explicit OverrideLocale(const QLocale& locale)
    : m_previousLocale(QLocale::system())
  {
    QLocale::setDefault(locale);
  }

  ~OverrideLocale() { QLocale::setDefault(m_previousLocale); }
};
} // namespace

TEST_CASE("QtUtils")
{
  const auto en_US = QLocale{QLocale::English, QLocale::UnitedStates};
  const auto de_DE = QLocale{QLocale::German, QLocale::Germany};

  SECTION("toString")
  {
    using T = std::tuple<QLocale, vm::vec3d, QString>;

    const auto& [locale, vec, expectedString] = GENERATE_COPY(values<T>({
      {en_US, {1.1, 2.2, 3.3}, "1.1 2.2 3.3"},
      {en_US, {1, 2, 3}, "1 2 3"},
      {de_DE, {1.1, 2.2, 3.3}, "1,1 2,2 3,3"},
      {de_DE, {1, 2, 3}, "1 2 3"},
    }));

    CAPTURE(locale.name(), vec);

    const auto overrideLocale = OverrideLocale{locale};

    CHECK(toString(vec) == expectedString);
  }

  SECTION("parse")
  {
    using T = std::tuple<QLocale, QString, std::optional<vm::vec3d>>;

    const auto& [locale, str, expectedVec] = GENERATE_COPY(values<T>({
      {en_US, "asdf", std::nullopt},
      {en_US, "1.1 2.2 3.3", vm::vec3d{1.1, 2.2, 3.3}},
      {en_US, "1 2 3", vm::vec3d{1, 2, 3}},
      {de_DE, "asdf", std::nullopt},
      {de_DE, "1,1 2,2 3,3", vm::vec3d{1.1, 2.2, 3.3}},
      {de_DE, "1 2 3", vm::vec3d{1, 2, 3}},
    }));

    CAPTURE(locale.name(), str);

    const auto overrideLocale = OverrideLocale{locale};

    CHECK(parse<double, 3>(str) == expectedVec);
  }
}

} // namespace tb::ui

/*
 Copyright (C) 2026 Kristian Duske

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

#include <QListWidget>

#include "mdl/EntityDefinitionFileSpec.h"
#include "mdl/GameConfig.h"
#include "mdl/Map.h"
#include "mdl/MapFixture.h"
#include "mdl/Map_Assets.h"
#include "ui/CatchConfig.h"
#include "ui/EntityDefinitionFileChooser.h"
#include "ui/MapDocument.h"
#include "ui/MapDocumentFixture.h"

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_range_equals.hpp>

namespace tb::ui
{

TEST_CASE("EntityDefinitionFileChooser")
{
  using Catch::Matchers::RangeEquals;

  auto fixtureConfig = mdl::MapFixtureConfig{};
  fixtureConfig.gameInfo.gameConfig.entityConfig.defFilePaths =
    std::vector<std::filesystem::path>{
      "tex10.fgd",
      "tex9.fgd",
      "tex2.fgd",
    };

  auto documentFixture = MapDocumentFixture{};
  auto& document = documentFixture.create(fixtureConfig);
  auto& map = document.map();

  auto chooser = EntityDefinitionFileChooser{document};

  // The chooser only rebuilds its list in response to a notifier fired after
  // construction, so trigger one here.
  mdl::setEntityDefinitionFile(
    map, mdl::EntityDefinitionFileSpec::makeBuiltin("tex9.fgd"));

  auto* list = chooser.findChild<SingleSelectionListWidget*>();
  REQUIRE(list != nullptr);
  REQUIRE(list->count() == 3);

  const auto names =
    std::views::iota(0, list->count()) | std::views::transform([&](const auto i) {
      return list->item(i)->data(Qt::DisplayRole).toString().toStdString();
    });

  // A plain lexicographic sort would order these as tex10.fgd, tex2.fgd, tex9.fgd.
  CHECK_THAT(
    names, RangeEquals(std::vector<std::string>{"tex2.fgd", "tex9.fgd", "tex10.fgd"}));
}

} // namespace tb::ui

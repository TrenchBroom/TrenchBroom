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

#include <QApplication>
#include <QCheckBox>
#include <QPushButton>
#include <QtTest/QTest>

#include "mdl/EditorContext.h"
#include "mdl/EntityDefinition.h"
#include "mdl/EntityDefinitionManager.h"
#include "mdl/Map.h"
#include "ui/CatchConfig.h"
#include "ui/MapDocument.h"
#include "ui/MapDocumentFixture.h"
#include "ui/ViewEditor.h"

#include <ranges>
#include <vector>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_range_equals.hpp>

namespace tb::ui
{
using Catch::Matchers::RangeEquals;

namespace
{

auto getDefinitionCheckBoxes(EntityDefinitionCheckBoxList& list)
{
  return list.findChildren<QCheckBox*>("entityDefinition_checkboxWidget");
}

auto getGroupCheckBoxes(EntityDefinitionCheckBoxList& list)
{
  return list.findChildren<QCheckBox*>() | std::views::filter([](const auto* checkBox) {
           return checkBox->objectName() != "entityDefinition_checkboxWidget";
         });
}

QCheckBox* findCheckBox(auto checkBoxes, const QString& text)
{
  if (const auto iGroupCheckBox = std::ranges::find_if(
        checkBoxes, [&](const auto* checkBox) { return checkBox->text() == text; });
      iGroupCheckBox != checkBoxes.end())
  {
    return *iGroupCheckBox;
  }

  return nullptr;
}

QPushButton* findButton(EntityDefinitionCheckBoxList& list, const QString& text)
{
  auto buttons = list.findChildren<QPushButton*>();
  if (const auto iButton = std::ranges::find_if(
        buttons, [&](const auto* button) { return button->text() == text; });
      iButton != buttons.end())
  {
    return *iButton;
  }

  return nullptr;
}

} // namespace

TEST_CASE("EntityDefinitionCheckBoxList")
{
  auto fixture = MapDocumentFixture{};
  auto& document = fixture.create();
  auto& map = document.map();

  map.entityDefinitionManager().setDefinitions({
    {"func_zeta", {}, "", {}, tb::mdl::PointEntityDefinition{}},
    {"func_alpha", {}, "", {}, tb::mdl::PointEntityDefinition{}},
    {"trigger_once", {}, "", {}, tb::mdl::PointEntityDefinition{}},
  });

  auto list =
    EntityDefinitionCheckBoxList{map.entityDefinitionManager(), map.editorContext()};
  list.resize(400, 400);
  list.show();
  QApplication::processEvents();

  const auto* funcZetaDefinition = map.entityDefinitionManager().definition("func_zeta");
  const auto* funcAlphaDefinition =
    map.entityDefinitionManager().definition("func_alpha");
  const auto* triggerOnceDefinition =
    map.entityDefinitionManager().definition("trigger_once");

  REQUIRE(funcZetaDefinition != nullptr);
  REQUIRE(funcAlphaDefinition != nullptr);
  REQUIRE(triggerOnceDefinition != nullptr);

  SECTION("EntityDefinitionCheckBoxList")
  {
    SECTION("keeps definition order from group definitions")
    {
      const auto definitionCheckBoxes = getDefinitionCheckBoxes(list);

      REQUIRE(definitionCheckBoxes.size() == 3u);

      CHECK_THAT(
        definitionCheckBoxes | std::views::transform([](const auto* checkBox) {
          return checkBox->text().toStdString();
        }),
        RangeEquals({"func_zeta", "func_alpha", "trigger_once"}));
    }
  }

  SECTION("refresh")
  {
    map.editorContext().setEntityDefinitionHidden(*funcZetaDefinition, true);
    map.editorContext().setEntityDefinitionHidden(*funcAlphaDefinition, false);
    map.editorContext().setEntityDefinitionHidden(*triggerOnceDefinition, false);
    list.refresh();

    const auto* funcGroupCheckBox = findCheckBox(getGroupCheckBoxes(list), "Func");
    REQUIRE(funcGroupCheckBox != nullptr);
    CHECK(funcGroupCheckBox->checkState() == Qt::PartiallyChecked);
  }

  SECTION("groupCheckBoxChanged")
  {
    map.editorContext().setEntityDefinitionHidden(*funcZetaDefinition, true);
    map.editorContext().setEntityDefinitionHidden(*funcAlphaDefinition, true);
    list.refresh();

    auto* funcGroupCheckBox = findCheckBox(getGroupCheckBoxes(list), "Func");
    REQUIRE(funcGroupCheckBox != nullptr);

    QTest::mouseClick(funcGroupCheckBox, Qt::LeftButton);
    QApplication::processEvents();

    CHECK(!map.editorContext().entityDefinitionHidden(*funcZetaDefinition));
    CHECK(!map.editorContext().entityDefinitionHidden(*funcAlphaDefinition));
    CHECK(!map.editorContext().entityDefinitionHidden(*triggerOnceDefinition));
  }

  SECTION("defCheckBoxChanged")
  {
    auto* funcZetaCheckBox = findCheckBox(getDefinitionCheckBoxes(list), "func_zeta");
    REQUIRE(funcZetaCheckBox != nullptr);

    QTest::mouseClick(funcZetaCheckBox, Qt::LeftButton);
    QApplication::processEvents();

    CHECK(map.editorContext().entityDefinitionHidden(*funcZetaDefinition));
    CHECK(!map.editorContext().entityDefinitionHidden(*funcAlphaDefinition));
    CHECK(!map.editorContext().entityDefinitionHidden(*triggerOnceDefinition));
  }

  SECTION("showAllClicked and hideAllClicked")
  {
    auto* hideAllButton = findButton(list, "Hide all");
    auto* showAllButton = findButton(list, "Show all");

    REQUIRE(hideAllButton != nullptr);
    REQUIRE(showAllButton != nullptr);

    QTest::mouseClick(hideAllButton, Qt::LeftButton);
    QApplication::processEvents();

    CHECK(map.editorContext().entityDefinitionHidden(*funcZetaDefinition));
    CHECK(map.editorContext().entityDefinitionHidden(*funcAlphaDefinition));
    CHECK(map.editorContext().entityDefinitionHidden(*triggerOnceDefinition));

    QTest::mouseClick(showAllButton, Qt::LeftButton);
    QApplication::processEvents();

    CHECK(!map.editorContext().entityDefinitionHidden(*funcZetaDefinition));
    CHECK(!map.editorContext().entityDefinitionHidden(*funcAlphaDefinition));
    CHECK(!map.editorContext().entityDefinitionHidden(*triggerOnceDefinition));
  }
}

} // namespace tb::ui
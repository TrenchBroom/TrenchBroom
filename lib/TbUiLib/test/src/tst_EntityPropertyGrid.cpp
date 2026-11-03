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

#include <QAbstractItemView>
#include <QCheckBox>
#include <QCompleter>
#include <QCoreApplication>
#include <QLineEdit>
#include <QMenu>
#include <QToolButton>
#include <QtTest/QSignalSpy>
#include <QtTest/QTest>

#include "mdl/EntityDefinitionManager.h"
#include "mdl/EntityNode.h"
#include "mdl/GroupNode.h"
#include "mdl/Map.h"
#include "mdl/Map_Groups.h"
#include "mdl/Map_Nodes.h"
#include "mdl/Map_Selection.h"
#include "ui/AppControllerFixture.h"
#include "ui/CatchConfig.h"
#include "ui/EntityPropertyGrid.h"
#include "ui/EntityPropertyModel.h"
#include "ui/EntityPropertyTable.h"
#include "ui/MapDocument.h"
#include "ui/MapDocumentFixture.h"

#include "kd/ranges/cartesian_product_view.h"
#include "kd/ranges/to.h"
#include "kd/reflection_impl.h"
#include "kd/unpack.h"

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

namespace tb::ui
{
namespace
{

struct Cell
{
  std::string key;
  int column;

  kdl_reflect_inline(Cell, key, column);
};

QModelIndex findIndex(
  const QAbstractItemModel& model, const QString& key, const int column)
{
  const auto rows = std::views::iota(0, model.rowCount());
  const auto iRow = std::ranges::find_if(rows, [&](const auto row) {
    return model.index(row, EntityPropertyModel::ColumnKey).data().toString() == key;
  });

  return iRow != rows.end() ? model.index(*iRow, column) : QModelIndex{};
}

Cell cell(const QModelIndex& index)
{
  return {
    index.siblingAtColumn(EntityPropertyModel::ColumnKey).data().toString().toStdString(),
    index.column(),
  };
}

std::vector<std::string> rowKeys(const QAbstractItemModel& model)
{
  return std::views::iota(0, model.rowCount())
         | std::views::transform([&](const auto row) {
             return model.index(row, EntityPropertyModel::ColumnKey)
               .data()
               .toString()
               .toStdString();
           })
         | kdl::ranges::to<std::vector>();
}

std::vector<Cell> openEditors(const QAbstractItemView& view)
{
  const auto& model = *view.model();

  const auto toModelIndex = kdl::unpack(
    [&](const auto row, const auto column) { return model.index(row, column); });
  const auto isIndexWidget = [&](const auto& index) { return view.indexWidget(index); };

  return kdl::views::cartesian_product(
           std::views::iota(0, model.rowCount()),
           std::views::iota(0, model.columnCount()))
         | std::views::transform(toModelIndex) | std::views::filter(isIndexWidget)
         | std::views::transform(cell) | kdl::ranges::to<std::vector>();
}

/**
 * Starts editing the given cell and replaces its text with the given text. Returns the
 * editor's completion popup, or null if no editor was opened.
 */
QAbstractItemView* editCell(
  QAbstractItemView& view, const QModelIndex& index, const QString& text)
{
  view.setCurrentIndex(index);
  view.edit(index);

  auto* lineEdit = qobject_cast<QLineEdit*>(view.indexWidget(index));
  if (!lineEdit)
  {
    return nullptr;
  }

  lineEdit->selectAll();
  QTest::keyClicks(lineEdit, text);
  return lineEdit->completer()->popup();
}

void processEvents()
{
  // the grid updates its table with a delay
  QCoreApplication::processEvents();
}

} // namespace

TEST_CASE("EntityPropertyGrid")
{
  auto appControllerFixture = AppControllerFixture{};

  auto documentFixture = MapDocumentFixture{};
  auto& document = documentFixture.create();
  auto& map = document.map();

  map.entityDefinitionManager().setDefinitions({
    {"source_entity",
     {},
     {},
     {
       {
         mdl::EntityPropertyKeys::Target,
         mdl::PropertyValueTypes::LinkSource{},
         {},
         {},
       },
       {
         mdl::EntityPropertyKeys::Killtarget,
         mdl::PropertyValueTypes::LinkSource{},
         {},
         {},
       },
     }},
    {"target_entity",
     {},
     {},
     {
       {
         mdl::EntityPropertyKeys::Targetname,
         mdl::PropertyValueTypes::LinkTarget{},
         {},
         {},
       },
       {
         "readonly_key",
         mdl::PropertyValueTypes::String{},
         {},
         {},
         true,
       },
     }},
    {"default_entity",
     {},
     {},
     {
       {
         "existing_key",
         mdl::PropertyValueTypes::String{"existing_default"},
         {},
         {},
       },
       {
         "missing_key",
         mdl::PropertyValueTypes::String{"missing_default"},
         {},
         {},
       },
     }},
  });

  auto* sourceEntity = new mdl::EntityNode{mdl::Entity{{
    {"classname", "source_entity"},
    {"target", "some_target"},
    {"some_key", "some_value"},
  }}};

  auto* targetEntity = new mdl::EntityNode{mdl::Entity{{
    {"classname", "target_entity"},
    {"targetname", "main_teleporter"},
    {"other_key", "other_value"},
    {"readonly_key", "readonly_value"},
  }}};

  auto* defaultEntity = new mdl::EntityNode{mdl::Entity{{
    {"classname", "default_entity"},
    {"existing_key", "existing_value"},
  }}};

  auto* groupedEntity = new mdl::EntityNode{mdl::Entity{{
    {"some_key", "some_value"},
  }}};

  mdl::addNodes(
    map,
    {{&mdl::parentForNodes(map),
      {sourceEntity, targetEntity, defaultEntity, groupedEntity}}});

  mdl::selectNodes(map, {groupedEntity});
  mdl::groupSelectedNodes(map, "group");

  auto* linkedGroupNode = mdl::createLinkedDuplicate(map);
  REQUIRE(linkedGroupNode != nullptr);

  auto* linkedEntity =
    dynamic_cast<mdl::EntityNode*>(linkedGroupNode->children().front());
  REQUIRE(linkedEntity != nullptr);

  mdl::deselectAll(map);

  auto grid = EntityPropertyGrid{document};
  grid.show();
  processEvents();

  auto* table = grid.findChild<EntityPropertyTable*>("EntityPropertyGrid_Table");
  auto* addPropertyButton =
    grid.findChild<QToolButton*>("EntityPropertyGrid_AddPropertyButton");
  auto* addProtectedPropertyButton =
    grid.findChild<QToolButton*>("EntityPropertyGrid_AddProtectedPropertyButton");
  auto* removePropertiesButton =
    grid.findChild<QToolButton*>("EntityPropertyGrid_RemovePropertiesButton");
  auto* setDefaultPropertiesButton =
    grid.findChild<QToolButton*>("EntityPropertyGrid_SetDefaultPropertiesButton");
  auto* showDefaultPropertiesCheckBox =
    grid.findChild<QCheckBox*>("EntityPropertyGrid_ShowDefaultPropertiesCheckBox");

  REQUIRE(table != nullptr);
  REQUIRE(addPropertyButton != nullptr);
  REQUIRE(addProtectedPropertyButton != nullptr);
  REQUIRE(removePropertiesButton != nullptr);
  REQUIRE(setDefaultPropertiesButton != nullptr);
  REQUIRE(showDefaultPropertiesCheckBox != nullptr);

  const auto& model = *table->model();

  SECTION("updateControls")
  {
    SECTION("Shows the selected entity's properties")
    {
      mdl::selectNodes(map, {sourceEntity});
      processEvents();

      CHECK(
        rowKeys(model)
        == std::vector<std::string>{"classname", "some_key", "target", "killtarget"});
    }

    SECTION("Shows protected properties if all selected entities are in groups")
    {
      mdl::selectNodes(map, {groupedEntity});
      processEvents();

      CHECK(!table->isColumnHidden(EntityPropertyModel::ColumnProtected));
      CHECK(!addProtectedPropertyButton->isHidden());

      mdl::deselectAll(map);
      mdl::selectNodes(map, {sourceEntity});
      processEvents();

      CHECK(table->isColumnHidden(EntityPropertyModel::ColumnProtected));
      CHECK(addProtectedPropertyButton->isHidden());
    }
  }

  SECTION("updateControlsEnabled")
  {
    SECTION("Enables the controls for worldspawn if no entity is selected")
    {
      mdl::selectNodes(map, {sourceEntity});
      mdl::deselectAll(map);
      processEvents();

      CHECK(
        findIndex(model, "classname", EntityPropertyModel::ColumnValue).data()
        == "worldspawn");
      CHECK(table->isEnabled());
      CHECK(addPropertyButton->isEnabled());
      CHECK(setDefaultPropertiesButton->isEnabled());
    }

    SECTION("Enables the controls if an entity is selected")
    {
      mdl::selectNodes(map, {sourceEntity});
      processEvents();

      CHECK(table->isEnabled());
      CHECK(addPropertyButton->isEnabled());
      CHECK(setDefaultPropertiesButton->isEnabled());
    }

    SECTION("Disables the controls if the linked groups cannot be updated")
    {
      mdl::selectNodes(map, {groupedEntity, linkedEntity});
      processEvents();

      CHECK(!table->isEnabled());
      CHECK(!addPropertyButton->isEnabled());
      CHECK(!removePropertiesButton->isEnabled());
      CHECK(!setDefaultPropertiesButton->isEnabled());
    }

    SECTION("Enables the remove button if the current property can be removed")
    {
      mdl::selectNodes(map, {sourceEntity});
      processEvents();

      // the table makes its first cell current when it receives focus, which depends on
      // whether the window is active
      table->setCurrentIndex(QModelIndex{});
      CHECK(!removePropertiesButton->isEnabled());

      table->setCurrentIndex(
        findIndex(model, "some_key", EntityPropertyModel::ColumnValue));
      CHECK(removePropertiesButton->isEnabled());

      // default properties are unset
      table->setCurrentIndex(
        findIndex(model, "killtarget", EntityPropertyModel::ColumnValue));
      CHECK(!removePropertiesButton->isEnabled());
    }
  }

  SECTION("addProperty")
  {
    SECTION("Adds a new property and makes its key current")
    {
      mdl::selectNodes(map, {sourceEntity});
      processEvents();

      SECTION("Using the add button")
      {
        QTest::mouseClick(addPropertyButton, Qt::LeftButton);
      }

      SECTION("Using the keyboard shortcut")
      {
        QTest::keyClick(table, Qt::Key_Return, Qt::ControlModifier);
      }

      processEvents();

      CHECK(
        sourceEntity->entity().properties()
        == std::vector<mdl::EntityProperty>{
          {"classname", "source_entity"},
          {"target", "some_target"},
          {"some_key", "some_value"},
          {"property 1", ""},
        });
      CHECK(sourceEntity->entity().protectedProperties().empty());
      CHECK(
        cell(table->currentIndex())
        == Cell{"property 1", EntityPropertyModel::ColumnKey});
      CHECK(grid.selectedRowName() == "property 1");
    }

    SECTION("Numbers new properties consecutively")
    {
      mdl::selectNodes(map, {sourceEntity});
      processEvents();

      QTest::mouseClick(addPropertyButton, Qt::LeftButton);
      processEvents();
      QTest::mouseClick(addPropertyButton, Qt::LeftButton);
      processEvents();

      CHECK(
        sourceEntity->entity().properties()
        == std::vector<mdl::EntityProperty>{
          {"classname", "source_entity"},
          {"target", "some_target"},
          {"some_key", "some_value"},
          {"property 1", ""},
          {"property 2", ""},
        });
      CHECK(grid.selectedRowName() == "property 2");
    }

    SECTION("Adds a new protected property")
    {
      mdl::selectNodes(map, {groupedEntity});
      processEvents();

      QTest::mouseClick(addProtectedPropertyButton, Qt::LeftButton);
      processEvents();

      CHECK(
        groupedEntity->entity().properties()
        == std::vector<mdl::EntityProperty>{
          {"some_key", "some_value"},
          {"property 1", ""},
        });
      CHECK(
        groupedEntity->entity().protectedProperties()
        == std::vector<std::string>{"property 1"});
    }
  }

  SECTION("removeSelectedProperties")
  {
    mdl::selectNodes(map, {sourceEntity});
    processEvents();

    SECTION("Removes the current property")
    {
      table->setCurrentIndex(
        findIndex(model, "some_key", EntityPropertyModel::ColumnValue));

      SECTION("Using the remove button")
      {
        QTest::mouseClick(removePropertiesButton, Qt::LeftButton);
      }

      SECTION("Using the keyboard shortcut")
      {
        const auto key = GENERATE(Qt::Key_Delete, Qt::Key_Backspace);
        CAPTURE(key);

        QTest::keyClick(table, key);
      }

      processEvents();

      CHECK(
        sourceEntity->entity().properties()
        == std::vector<mdl::EntityProperty>{
          {"classname", "source_entity"},
          {"target", "some_target"},
        });
    }

    SECTION("Removes all selected properties in one transaction")
    {
      table->setCurrentIndex(
        findIndex(model, "some_key", EntityPropertyModel::ColumnValue));
      table->selectionModel()->select(
        findIndex(model, "target", EntityPropertyModel::ColumnValue),
        QItemSelectionModel::Select);

      QTest::mouseClick(removePropertiesButton, Qt::LeftButton);
      processEvents();

      CHECK(
        sourceEntity->entity().properties()
        == std::vector<mdl::EntityProperty>{
          {"classname", "source_entity"},
        });

      map.undoCommand();

      CHECK(
        sourceEntity->entity().properties()
        == std::vector<mdl::EntityProperty>{
          {"classname", "source_entity"},
          {"target", "some_target"},
          {"some_key", "some_value"},
        });
    }

    SECTION("Does not remove anything if a selected property cannot be removed")
    {
      table->setCurrentIndex(
        findIndex(model, "some_key", EntityPropertyModel::ColumnValue));
      table->selectionModel()->select(
        findIndex(model, "killtarget", EntityPropertyModel::ColumnValue),
        QItemSelectionModel::Select);

      QTest::keyClick(table, Qt::Key_Delete);
      processEvents();

      CHECK(
        sourceEntity->entity().properties()
        == std::vector<mdl::EntityProperty>{
          {"classname", "source_entity"},
          {"target", "some_target"},
          {"some_key", "some_value"},
        });
    }
  }

  SECTION("backupSelection and restoreSelection")
  {
    mdl::selectNodes(map, {sourceEntity});
    processEvents();

    table->setCurrentIndex(
      findIndex(model, "some_key", EntityPropertyModel::ColumnValue));
    REQUIRE(grid.selectedRowName() == "some_key");

    // targetEntity doesn't have some_key, so the selection is lost
    mdl::deselectAll(map);
    mdl::selectNodes(map, {targetEntity});
    processEvents();

    CHECK(table->selectionModel()->selectedIndexes().empty());

    // the selection is restored when some_key is shown again
    mdl::deselectAll(map);
    mdl::selectNodes(map, {sourceEntity});
    processEvents();

    CHECK(
      table->selectionModel()->selectedIndexes()
      == QModelIndexList{findIndex(model, "some_key", EntityPropertyModel::ColumnValue)});
    CHECK(
      cell(table->currentIndex()) == Cell{"some_key", EntityPropertyModel::ColumnValue});
  }

  SECTION("selectedRowName")
  {
    mdl::selectNodes(map, {sourceEntity});
    processEvents();

    // the table makes its first cell current when it receives focus, which depends on
    // whether the window is active
    table->setCurrentIndex(QModelIndex{});
    CHECK(grid.selectedRowName() == "");

    auto currentRowChangedSpy = QSignalSpy{&grid, &EntityPropertyGrid::currentRowChanged};

    table->setCurrentIndex(findIndex(model, "target", EntityPropertyModel::ColumnKey));

    CHECK(grid.selectedRowName() == "target");
    CHECK(currentRowChangedSpy.count() > 0);
  }

  SECTION("Set default properties menu")
  {
    mdl::selectNodes(map, {defaultEntity});
    processEvents();

    using T = std::tuple<std::string, std::vector<mdl::EntityProperty>>;
    const auto [actionName, expectedProperties] = GENERATE(values<T>({
      {"EntityPropertyGrid_SetExistingDefaultPropertiesAction",
       {
         {"classname", "default_entity"},
         {"existing_key", "existing_default"},
       }},
      {"EntityPropertyGrid_SetMissingDefaultPropertiesAction",
       {
         {"classname", "default_entity"},
         {"existing_key", "existing_value"},
         {"missing_key", "missing_default"},
       }},
      {"EntityPropertyGrid_SetAllDefaultPropertiesAction",
       {
         {"classname", "default_entity"},
         {"existing_key", "existing_default"},
         {"missing_key", "missing_default"},
       }},
    }));
    CAPTURE(actionName);

    auto* action = setDefaultPropertiesButton->menu()->findChild<QAction*>(
      QString::fromStdString(actionName));
    REQUIRE(action != nullptr);

    action->trigger();

    CHECK(defaultEntity->entity().properties() == expectedProperties);
  }

  SECTION("Show default properties check box")
  {
    mdl::selectNodes(map, {sourceEntity});
    processEvents();

    REQUIRE(showDefaultPropertiesCheckBox->isChecked());
    REQUIRE(
      rowKeys(model)
      == std::vector<std::string>{"classname", "some_key", "target", "killtarget"});

    showDefaultPropertiesCheckBox->click();

    CHECK(!showDefaultPropertiesCheckBox->isChecked());
    CHECK(rowKeys(model) == std::vector<std::string>{"classname", "some_key", "target"});

    showDefaultPropertiesCheckBox->click();

    CHECK(showDefaultPropertiesCheckBox->isChecked());
    CHECK(
      rowKeys(model)
      == std::vector<std::string>{"classname", "some_key", "target", "killtarget"});
  }

  SECTION("Editing")
  {
    mdl::selectNodes(map, {sourceEntity});
    processEvents();

    SECTION("Return starts editing the current cell")
    {
      const auto index = findIndex(model, "some_key", EntityPropertyModel::ColumnValue);
      table->setCurrentIndex(index);

      QTest::keyClick(table, Qt::Key_Return);

      auto* lineEdit = qobject_cast<QLineEdit*>(table->indexWidget(index));
      REQUIRE(lineEdit != nullptr);
      CHECK(lineEdit->text() == "some_value");
    }

    SECTION("Typing starts editing the current cell")
    {
      const auto index = findIndex(model, "some_key", EntityPropertyModel::ColumnValue);
      table->setCurrentIndex(index);

      QTest::keyClick(table, Qt::Key_X);

      auto* lineEdit = qobject_cast<QLineEdit*>(table->indexWidget(index));
      REQUIRE(lineEdit != nullptr);
      CHECK(lineEdit->text() == "x");
    }

    SECTION("Typing on a read only cell does nothing")
    {
      mdl::deselectAll(map);
      mdl::selectNodes(map, {targetEntity});
      processEvents();

      const auto index =
        findIndex(model, "readonly_key", EntityPropertyModel::ColumnValue);
      table->setCurrentIndex(index);

      // would search for other_value otherwise
      QTest::keyClick(table, Qt::Key_O);

      CHECK(
        cell(table->currentIndex())
        == Cell{"readonly_key", EntityPropertyModel::ColumnValue});
      CHECK(openEditors(*table).empty());
    }

    SECTION("Clicking a selected cell starts editing it")
    {
      const auto index = findIndex(model, "some_key", EntityPropertyModel::ColumnValue);
      const auto position = table->visualRect(index).center();

      QTest::mouseClick(table->viewport(), Qt::LeftButton, {}, position);

      CHECK(
        cell(table->currentIndex())
        == Cell{"some_key", EntityPropertyModel::ColumnValue});
      CHECK(openEditors(*table).empty());

      QTest::mouseClick(table->viewport(), Qt::LeftButton, {}, position);

      CHECK(
        openEditors(*table)
        == std::vector<Cell>{{"some_key", EntityPropertyModel::ColumnValue}});
    }

    SECTION("Return commits the value and keeps the cell current")
    {
      const auto index = findIndex(model, "target", EntityPropertyModel::ColumnValue);

      // doesn't match any completion
      auto* popup = editCell(*table, index, "some_other_target");
      REQUIRE(popup != nullptr);
      REQUIRE(!popup->isVisible());

      QTest::keyClick(table->indexWidget(index), Qt::Key_Return);
      processEvents();

      CHECK(
        sourceEntity->entity().properties()
        == std::vector<mdl::EntityProperty>{
          {"classname", "source_entity"},
          {"target", "some_other_target"},
          {"some_key", "some_value"},
        });
      CHECK(
        cell(table->currentIndex()) == Cell{"target", EntityPropertyModel::ColumnValue});
      CHECK(openEditors(*table).empty());
    }

    SECTION("Accepting a completion with return")
    {
      // While the completion popup is visible, it receives all key events, so the tests
      // send them to the popup as well.

      SECTION("Value of a set property")
      {
        const auto index = findIndex(model, "target", EntityPropertyModel::ColumnValue);

        auto* popup = editCell(*table, index, "main");
        REQUIRE(popup != nullptr);
        REQUIRE(popup->isVisible());

        QTest::keyClick(popup, Qt::Key_Down);
        QTest::keyClick(popup, Qt::Key_Return);
        processEvents();

        CHECK(
          sourceEntity->entity().properties()
          == std::vector<mdl::EntityProperty>{
            {"classname", "source_entity"},
            {"target", "main_teleporter"},
            {"some_key", "some_value"},
          });

        // editing continues with the next cell, which is the key of the next row
        CHECK(
          cell(table->currentIndex())
          == Cell{"killtarget", EntityPropertyModel::ColumnKey});
        CHECK(table->indexWidget(table->currentIndex()) != nullptr);
      }

      SECTION("Value of a default property")
      {
        const auto index =
          findIndex(model, "killtarget", EntityPropertyModel::ColumnValue);

        auto* popup = editCell(*table, index, "main");
        REQUIRE(popup != nullptr);
        REQUIRE(popup->isVisible());

        QTest::keyClick(popup, Qt::Key_Down);
        QTest::keyClick(popup, Qt::Key_Return);
        processEvents();

        CHECK(
          sourceEntity->entity().properties()
          == std::vector<mdl::EntityProperty>{
            {"classname", "source_entity"},
            {"target", "some_target"},
            {"some_key", "some_value"},
            {"killtarget", "main_teleporter"},
          });

        // killtarget was the last row, so there is no next cell to edit
        CHECK(
          cell(table->currentIndex())
          == Cell{"killtarget", EntityPropertyModel::ColumnValue});
        CHECK(openEditors(*table).empty());
      }

      SECTION("Key")
      {
        const auto index = findIndex(model, "some_key", EntityPropertyModel::ColumnKey);

        auto* popup = editCell(*table, index, "other");
        REQUIRE(popup != nullptr);
        REQUIRE(popup->isVisible());

        QTest::keyClick(popup, Qt::Key_Down);
        QTest::keyClick(popup, Qt::Key_Return);
        processEvents();

        CHECK(
          sourceEntity->entity().properties()
          == std::vector<mdl::EntityProperty>{
            {"classname", "source_entity"},
            {"target", "some_target"},
            {"other_key", "some_value"},
          });

        // editing continues with the next cell, which is the value of the same row
        CHECK(
          cell(table->currentIndex())
          == Cell{"other_key", EntityPropertyModel::ColumnValue});
        CHECK(table->indexWidget(table->currentIndex()) != nullptr);
      }
    }

    SECTION("Return while completions are shown but none is highlighted")
    {
      const auto index = findIndex(model, "target", EntityPropertyModel::ColumnValue);

      auto* popup = editCell(*table, index, "main");
      REQUIRE(popup != nullptr);
      REQUIRE(popup->isVisible());

      // the popup receives all key events while it is visible
      QTest::keyClick(popup, Qt::Key_Return);
      processEvents();

      CHECK(
        sourceEntity->entity().properties()
        == std::vector<mdl::EntityProperty>{
          {"classname", "source_entity"},
          {"target", "main"},
          {"some_key", "some_value"},
        });

      // editing continues with the next cell, which is the key of the next row
      CHECK(
        cell(table->currentIndex())
        == Cell{"killtarget", EntityPropertyModel::ColumnKey});
      CHECK(table->indexWidget(table->currentIndex()) != nullptr);
    }

    SECTION("Accepting a completion by clicking it")
    {
      const auto index = findIndex(model, "target", EntityPropertyModel::ColumnValue);

      auto* popup = editCell(*table, index, "main");
      REQUIRE(popup != nullptr);
      REQUIRE(popup->isVisible());

      const auto completionIndex = popup->model()->index(0, 0);
      QTest::mouseClick(
        popup->viewport(),
        Qt::LeftButton,
        {},
        popup->visualRect(completionIndex).center());
      processEvents();

      CHECK(
        sourceEntity->entity().properties()
        == std::vector<mdl::EntityProperty>{
          {"classname", "source_entity"},
          {"target", "main_teleporter"},
          {"some_key", "some_value"},
        });

      // editing continues with the next cell, which is the key of the next row
      CHECK(
        cell(table->currentIndex())
        == Cell{"killtarget", EntityPropertyModel::ColumnKey});
      CHECK(table->indexWidget(table->currentIndex()) != nullptr);
    }
  }
}

} // namespace tb::ui

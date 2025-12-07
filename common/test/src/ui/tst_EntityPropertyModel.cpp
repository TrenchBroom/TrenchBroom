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

#include "mdl/EntityDefinitionManager.h"
#include "mdl/Map.h"
#include "mdl/Map_Nodes.h"
#include "mdl/Map_Selection.h"
#include "ui/EntityPropertyModel.h"
#include "ui/MapDocumentFixture.h"

#include "catch/CatchConfig.h"

#include <catch2/catch_test_macros.hpp>

namespace tb::ui
{

TEST_CASE("EntityPropertyModel")
{
  auto fixture = MapDocumentFixture{};
  auto& document = fixture.document();
  auto& map = fixture.map();
  fixture.create();

  auto* entityNode1 = new mdl::EntityNode{mdl::Entity{{
    {"some_key", "some_value"},
  }}};

  auto* entityNode2 = new mdl::EntityNode{mdl::Entity{{
    {"some_key", "some_other_value"},
    {"some_other_key", "yet_another_value"},
  }}};

  auto* groupedEntityNode = new mdl::EntityNode{mdl::Entity{{
    {"some_key", "some_value"},
  }}};

  auto* groupNode = new mdl::GroupNode{mdl::Group{"group"}};
  groupNode->addChild(groupedEntityNode);

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
     }},
    {"readonly_entity",
     {},
     {},
     {
       {
         "readonly",
         mdl::PropertyValueTypes::String{},
         {},
         {},
         true,
       },
     }},
  });

  auto* sourceEntity = new mdl::EntityNode{mdl::Entity{{
    {"classname", "source_entity"},
    {"target", "some_target"},
  }}};

  auto* targetEntity = new mdl::EntityNode{mdl::Entity{{
    {"classname", "target_entity"},
    {"targetname", "some_target"},
  }}};

  auto* readonlyEntity = new mdl::EntityNode{mdl::Entity{{
    {"classname", "readonly_entity"},
    {"readonly", "some_value"},
  }}};

  mdl::addNodes(
    map,
    {
      {mdl::parentForNodes(map),
       {
         entityNode1,
         entityNode2,
         groupNode,
         sourceEntity,
         targetEntity,
         readonlyEntity,
       }},
    });

  auto model = EntityPropertyModel{document};

  SECTION("constructor")
  {
    CHECK(model.showDefaultRows());
    CHECK(!model.shouldShowProtectedProperties());

    SECTION("calls updateFromMap")
    {
      CHECK(
        model.rows()
        == std::vector<PropertyRow>{
          {
            .key = "classname",
            .value = "worldspawn",
            .valueState = ValueState::SingleValue,
            .keyMutable = false,
            .valueMutable = false,
            .protection = PropertyProtection::NotProtectable,
            .linkType = LinkType::None,
            .tooltip = "No description found",
          },
        });
    }
  }

  SECTION("showDefaultRows")
  {
    REQUIRE(model.showDefaultRows());

    // select a node so that we can check that updateFromMap is called
    mdl::selectNodes(map, {entityNode1});

    model.setShowDefaultRows(false);

    CHECK(!model.showDefaultRows());
    CHECK(
      model.rows()
      == std::vector<PropertyRow>{
        {
          .key = "some_key",
          .value = "some_value",
          .valueState = ValueState::SingleValue,
          .keyMutable = true,
          .valueMutable = true,
          .protection = PropertyProtection::NotProtectable,
          .linkType = LinkType::None,
          .tooltip = "No description found",
        },
      });
  }

  SECTION("shouldShowProtectedProperties")
  {
    REQUIRE(!model.shouldShowProtectedProperties());

    SECTION("nothing selected")
    {
      mdl::deselectAll(map);
      model.updateFromMap();
      CHECK(!model.shouldShowProtectedProperties());
    }

    SECTION("only ungrouped entities selected")
    {
      mdl::selectNodes(map, {entityNode1});
      model.updateFromMap();
      CHECK(!model.shouldShowProtectedProperties());
    }

    SECTION("mixed selection of grouped and ungrouped entities")
    {
      mdl::selectNodes(map, {entityNode1, groupedEntityNode});
      model.updateFromMap();
      CHECK(!model.shouldShowProtectedProperties());
    }

    SECTION("only grouped entities selected")
    {
      mdl::selectNodes(map, {groupedEntityNode});
      model.updateFromMap();
      CHECK(model.shouldShowProtectedProperties());
    }
  }

  SECTION("rowForModelIndex")
  {
    // cannot be tested because QModelIndex cannot be created
  }

  SECTION("rowIndexForPropertyKey")
  {
    selectNodes(map, {entityNode2});
    model.updateFromMap();

    CHECK(
      model.rows()
      == std::vector<PropertyRow>{
        {
          .key = "some_key",
          .value = "some_other_value",
          .valueState = ValueState::SingleValue,
          .keyMutable = true,
          .valueMutable = true,
          .protection = PropertyProtection::NotProtectable,
          .linkType = LinkType::None,
          .tooltip = "No description found",
        },
        {
          .key = "some_other_key",
          .value = "yet_another_value",
          .valueState = ValueState::SingleValue,
          .keyMutable = true,
          .valueMutable = true,
          .protection = PropertyProtection::NotProtectable,
          .linkType = LinkType::None,
          .tooltip = "No description found",
        },
      });

    CHECK(model.rowIndexForPropertyKey("asdf") == -1);
    CHECK(model.rowIndexForPropertyKey("some_key") == 0);
    CHECK(model.rowIndexForPropertyKey("some_other_key") == 1);
  }

  SECTION("getCompletions")
  {
    // cannot be tested because QModelIndex cannot be created
  }

  SECTION("updateFromMap")
  {
    SECTION("nothing selected")
    {
      mdl::deselectAll(map);
      model.updateFromMap();

      CHECK(
        model.rows()
        == std::vector<PropertyRow>{
          {
            .key = "classname",
            .value = "worldspawn",
            .valueState = ValueState::SingleValue,
            .keyMutable = false,
            .valueMutable = false,
            .protection = PropertyProtection::NotProtectable,
            .linkType = LinkType::None,
            .tooltip = "No description found",
          },
        });
    }

    SECTION("only a single entity selected")
    {
      mdl::selectNodes(map, {entityNode1});
      model.updateFromMap();

      CHECK(
        model.rows()
        == std::vector<PropertyRow>{
          {
            .key = "some_key",
            .value = "some_value",
            .valueState = ValueState::SingleValue,
            .keyMutable = true,
            .valueMutable = true,
            .protection = PropertyProtection::NotProtectable,
            .linkType = LinkType::None,
            .tooltip = "No description found",
          },
        });
    }

    SECTION("multiple entities selected")
    {
      mdl::selectNodes(map, {entityNode1, entityNode2});
      model.updateFromMap();

      CHECK(
        model.rows()
        == std::vector<PropertyRow>{
          {
            .key = "some_key",
            .value = "multi",
            .valueState = ValueState::MultipleValues,
            .keyMutable = true,
            .valueMutable = true,
            .protection = PropertyProtection::NotProtectable,
            .linkType = LinkType::None,
            .tooltip = "No description found",
          },
          {
            .key = "some_other_key",
            .value = "yet_another_value",
            .valueState = ValueState::SingleValueAndUnset,
            .keyMutable = true,
            .valueMutable = true,
            .protection = PropertyProtection::NotProtectable,
            .linkType = LinkType::None,
            .tooltip = "No description found",
          },
        });
    }

    SECTION("source entity")
    {
      mdl::selectNodes(map, {sourceEntity});
      model.updateFromMap();

      CHECK(
        model.rows()
        == std::vector<PropertyRow>{
          {
            .key = "classname",
            .value = "source_entity",
            .valueState = ValueState::SingleValue,
            .keyMutable = true,
            .valueMutable = true,
            .protection = PropertyProtection::NotProtectable,
            .linkType = LinkType::None,
            .tooltip = "No description found",
          },
          {
            .key = mdl::EntityPropertyKeys::Target,
            .value = "some_target",
            .valueState = ValueState::SingleValue,
            .keyMutable = true,
            .valueMutable = true,
            .protection = PropertyProtection::NotProtectable,
            .linkType = LinkType::Source,
            .tooltip = "",
          },
        });
    }

    SECTION("target entity")
    {
      mdl::selectNodes(map, {targetEntity});
      model.updateFromMap();

      CHECK(
        model.rows()
        == std::vector<PropertyRow>{
          {
            .key = "classname",
            .value = "target_entity",
            .valueState = ValueState::SingleValue,
            .keyMutable = true,
            .valueMutable = true,
            .protection = PropertyProtection::NotProtectable,
            .linkType = LinkType::None,
            .tooltip = "No description found",
          },
          {
            .key = mdl::EntityPropertyKeys::Targetname,
            .value = "some_target",
            .valueState = ValueState::SingleValue,
            .keyMutable = true,
            .valueMutable = true,
            .protection = PropertyProtection::NotProtectable,
            .linkType = LinkType::Target,
            .tooltip = "",
          },
        });
    }

    SECTION("readonly properties")
    {
      mdl::selectNodes(map, {readonlyEntity});
      model.updateFromMap();

      CHECK(
        model.rows()
        == std::vector<PropertyRow>{
          {
            .key = "classname",
            .value = "readonly_entity",
            .valueState = ValueState::SingleValue,
            .keyMutable = true,
            .valueMutable = true,
            .protection = PropertyProtection::NotProtectable,
            .linkType = LinkType::None,
            .tooltip = "No description found",
          },
          {
            .key = "readonly",
            .value = "some_value",
            .valueState = ValueState::SingleValue,
            .keyMutable = false,
            .valueMutable = false,
            .protection = PropertyProtection::NotProtectable,
            .linkType = LinkType::None,
            .tooltip = "",
          },
        });
    }
  }
}

} // namespace tb::ui

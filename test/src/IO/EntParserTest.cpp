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

#include <gtest/gtest.h>

#include "CollectionUtils.h"
#include "TestUtils.h"
#include "Assets/EntityDefinition.h"
#include "Assets/AttributeDefinition.h"
#include "Assets/EntityDefinitionTestUtils.h"
#include "IO/DiskIO.h"
#include "IO/EntParser.h"
#include "IO/Path.h"
#include "IO/TestParserStatus.h"
#include "Model/ModelTypes.h"

#include <algorithm>

namespace TrenchBroom {
    namespace IO {
        TEST(EntParserTest, parseIncludedEntFiles) {
            const Path basePath = Disk::getCurrentWorkingDir() + Path("data/games");
            const Path::List cfgFiles = Disk::findItemsRecursively(basePath, IO::FileExtensionMatcher("ent"));

            for (const Path& path : cfgFiles) {
                MappedFile::Ptr file = Disk::openFile(path);
                const Color defaultColor(1.0f, 1.0f, 1.0f, 1.0f);
                EntParser parser(file->begin(), file->end(), defaultColor);

                TestParserStatus status;
                ASSERT_NO_THROW(parser.parseDefinitions(status)) << "Parsing ENT file " << path.asString() << " failed";
                ASSERT_EQ(0u, status.countStatus(Logger::LogLevel_Warn))
                                    << "Parsing FGD file " << path.asString() << " produced warnings";
                ASSERT_EQ(0u, status.countStatus(Logger::LogLevel_Error))
                                    << "Parsing FGD file " << path.asString() << " produced errors";
            }
        }

        TEST(EntParserTest, parseEmptyFile) {
            const String file = "";
            const Color defaultColor(1.0f, 1.0f, 1.0f, 1.0f);
            EntParser parser(file, defaultColor);

            TestParserStatus status;
            Assets::EntityDefinitionList definitions = parser.parseDefinitions(status);
            ASSERT_TRUE(definitions.empty());
            VectorUtils::clearAndDelete(definitions);
        }

        TEST(EntParserTest, parseWhitespaceFile) {
            const String file = "     \n  \t \n  ";
            const Color defaultColor(1.0f, 1.0f, 1.0f, 1.0f);
            EntParser parser(file, defaultColor);

            TestParserStatus status;
            Assets::EntityDefinitionList definitions = parser.parseDefinitions(status);
            ASSERT_TRUE(definitions.empty());
            VectorUtils::clearAndDelete(definitions);
        }

        TEST(EntParserTest, parseSimplePointEntityDefinition) {
            const String file = R"(
<?xml version="1.0"?>
<!--
Quake3 Arena entity definition file for Q3Radiant
Based on draft by Suicide 20 7.30.99 and inolen 9-3-99
Upgraded by Eutectic: eutectic@ritualistic.com
(visible models added by raYGunn - paths provided by Suicide 20)
(terrain information added to func_group entity by Paul Jaquays)
Q3Map2 entities/keys added by ydnar
Additional Q3Map2 and Q3A PR 1.32 entities/keys added by Obsidian
Entities.def for GtkRadiant 1.4 and ZeroRadiant 1.6
Entities.ent for GtkRadiant 1.5
Version: 1.7.3
Updated: 2011-03-02
-->
<classes>
    <!--
    =============================================================================

    Q3MAP2 ENTITIES

    =============================================================================
    -->

    <point name="_skybox" color="0.77 0.88 1.0" box="-4 -4 -4 4 4 4">
    -------- KEYS --------
    <angle key="angle" name="Yaw Angle">Rotation angle of the sky surfaces.</angle>
    <angles key="angles" name="Pitch Yaw Roll">Individual control of PITCH, YAW, and ROLL (default 0 0 0).</angles>
    <real key="_scale" name="Scale" value="64">Scaling factor (default 64), good values are between 50 and 300, depending on the map.</real>
    -------- NOTES --------
    Compiler-only entity that specifies the origin of a skybox (a wholly contained, separate area of the map), similar to some games portal skies. When compiled with Q3Map2, the skybox surfaces will be visible from any place where sky is normally visible. It will cast shadows on the normal parts of the map, and can be used with cloud layers and other effects.
    </point>
</classes>
)";

            const Color defaultColor(1.0f, 1.0f, 1.0f, 1.0f);
            EntParser parser(file, defaultColor);

            TestParserStatus status;
            const auto definitions = parser.parseDefinitions(status);
            ASSERT_EQ(1u, definitions.size()) << "Expected one entity definition";

            const auto* pointDefinition = dynamic_cast<const Assets::PointEntityDefinition*>(definitions.front());
            ASSERT_NE(nullptr, pointDefinition) << "Definition must be a point entity definition";

            const auto expectedDescription = R"(
    -------- NOTES --------
    Compiler-only entity that specifies the origin of a skybox (a wholly contained, separate area of the map), similar to some games portal skies. When compiled with Q3Map2, the skybox surfaces will be visible from any place where sky is normally visible. It will cast shadows on the normal parts of the map, and can be used with cloud layers and other effects.
    )";
            ASSERT_EQ(expectedDescription, pointDefinition->description()) << "Expected text value as entity defintion description";

            ASSERT_TRUE(vm::isEqual(Color(0.77f, 0.88f, 1.0f, 1.0f), pointDefinition->color(), 0.01f)) << "Expected matching bounds";
            ASSERT_TRUE(vm::isEqual(vm::bbox3(vm::vec3(-4.0, -4.0, -4.0), vm::vec3(+4.0, +4.0, +4.0)), pointDefinition->bounds(), 0.01)) << "Expected matching color";

            ASSERT_EQ(3u, pointDefinition->attributeDefinitions().size()) << "Expected three attribute definitions";

            const auto* angleDefinition = pointDefinition->attributeDefinition("angle");
            ASSERT_NE(nullptr, angleDefinition) << "Missing attribute definition for 'angle' key";
            ASSERT_EQ(Assets::AttributeDefinition::Type_StringAttribute, angleDefinition->type()) << "Expected angle attribute definition to be of String type";

            ASSERT_EQ("angle", angleDefinition->name()) << "Expected matching attribute definition name";
            ASSERT_EQ("Yaw Angle", angleDefinition->shortDescription()) << "Expected attribute definition's short description to match name";
            ASSERT_EQ("Rotation angle of the sky surfaces.", angleDefinition->longDescription()) << "Expected attribute definition's long description to match element text";

            const auto* anglesDefinition = pointDefinition->attributeDefinition("angles");
            ASSERT_NE(nullptr, anglesDefinition) << "Missing attribute definition for 'angles' key";
            ASSERT_EQ(Assets::AttributeDefinition::Type_StringAttribute, anglesDefinition->type()) << "Expected angles attribute definition to be of String type";

            ASSERT_EQ("angles", anglesDefinition->name()) << "Expected matching attribute definition name";
            ASSERT_EQ("Pitch Yaw Roll", anglesDefinition->shortDescription()) << "Expected attribute definition's short description to match name";
            ASSERT_EQ("Individual control of PITCH, YAW, and ROLL (default 0 0 0).", anglesDefinition->longDescription()) << "Expected attribute definition's long description to match element text";

            const auto* scaleDefinition = dynamic_cast<const Assets::FloatAttributeDefinition*>(pointDefinition->attributeDefinition("_scale"));
            ASSERT_NE(nullptr, scaleDefinition) << "Missing attribute definition for '_scale' key";
            ASSERT_EQ(Assets::AttributeDefinition::Type_FloatAttribute, scaleDefinition->type()) << "Expected angles attribute definition to be of Float type";

            ASSERT_EQ("_scale", scaleDefinition->name()) << "Expected matching attribute definition name";
            ASSERT_EQ("Scale", scaleDefinition->shortDescription()) << "Expected attribute definition's short description to match name";
            ASSERT_EQ(64.0f, scaleDefinition->defaultValue()) << "Expected correct default value for '_scale' attribute definition";
            ASSERT_EQ("Scaling factor (default 64), good values are between 50 and 300, depending on the map.", scaleDefinition->longDescription()) << "Expected attribute definition's long description to match element text";
        }


        TEST(EntParserTest, parseInvalidRealAttributeDefinition) {
            const String file = R"(
<?xml version="1.0"?>
<classes>
    <point name="_skybox" color="0.77 0.88 1.0" box="-4 -4 -4 4 4 4">
        <real key="_scale" name="Scale" value="asdf" />
    </point>
</classes>
                        )";

                        const Color defaultColor(1.0f, 1.0f, 1.0f, 1.0f);
            EntParser parser(file, defaultColor);

            TestParserStatus status;
            const auto definitions = parser.parseDefinitions(status);
            ASSERT_EQ(1u, definitions.size()) << "Expected one entity definition";

            const auto* pointDefinition = dynamic_cast<const Assets::PointEntityDefinition*>(definitions.front());
            ASSERT_NE(nullptr, pointDefinition) << "Definition must be a point entity definition";

            ASSERT_EQ(1u, pointDefinition->attributeDefinitions().size()) << "Expected one attribute definitions";

            const auto* scaleDefinition = dynamic_cast<const Assets::StringAttributeDefinition*>(pointDefinition->attributeDefinition("_scale"));
            ASSERT_NE(nullptr, scaleDefinition) << "Missing attribute definition for '_scale' key";
            ASSERT_EQ(Assets::AttributeDefinition::Type_StringAttribute, scaleDefinition->type()) << "Expected angles attribute definition to be of Float type";

            ASSERT_EQ("asdf", scaleDefinition->defaultValue()) << "Expected correct default value for '_scale' attribute definition";
        }    }
}

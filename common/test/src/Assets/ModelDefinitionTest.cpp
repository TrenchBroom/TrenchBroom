/*
 Copyright (C) 2021 Kristian Duske

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

#include "Assets/ModelDefinition.h"
#include "EL/Expression.h"
#include "EL/VariableStore.h"
#include "IO/ELParser.h"
#include "IO/Path.h"

#include <map>
#include <tuple>

#include "Catch2.h"

namespace TrenchBroom {
    namespace Assets {
        static ModelDefinition makeModelDefinition(const std::string& expression) {
            auto parser = IO::ELParser{IO::ELParser::Mode::Strict, expression};
            return ModelDefinition{parser.parse()};
        }

        TEST_CASE("ModelDefinitionTest.append") {
            auto d1 = makeModelDefinition(R"("maps/b_shell0.bsp")");
            REQUIRE(d1.modelSpecification(EL::NullVariableStore{}) == ModelSpecification{IO::Path{"maps/b_shell0.bsp"}, 0, 0});

            d1.append(makeModelDefinition(R"("maps/b_shell1.bsp")"));
            CHECK(d1.modelSpecification(EL::NullVariableStore{}) == ModelSpecification{IO::Path{"maps/b_shell0.bsp"}, 0, 0});
        }

        TEST_CASE("ModelDefinitionTest.modelSpecification") {
            using T = std::tuple<std::string, std::map<std::string, EL::Value>, ModelSpecification>;

            const auto 
            [expression,                                            variables, expectedModelSpecification] = GENERATE(values<T>({
            {R"("maps/b_shell0.bsp")",                              {},        {IO::Path{"maps/b_shell0.bsp"}, 0, 0}},
            {R"({ path: "maps/b_shell0.bsp", skin: 1, frame: 2 })", {},        {IO::Path{"maps/b_shell0.bsp"}, 1, 2}},
            
            {R"({{
                spawnflags == 1 -> "maps/b_shell0.bsp",
                                   "maps/b_shell1.bsp"
            }})",                                                   {},
                                                                               {IO::Path{"maps/b_shell1.bsp"}, 0, 0}},
            
            {R"({{
                spawnflags == 1 -> "maps/b_shell0.bsp",
                                   "maps/b_shell1.bsp"
            }})",                                                   {{"spawnflags", EL::Value{1}}},
                                                                               {IO::Path{"maps/b_shell0.bsp"}, 0, 0}},

            {R"({path: model, skin: skin, frame: frame})",          {{"model", EL::Value{"maps/b_shell0.bsp"}},
                                                                     {"skin",  EL::Value{1}},
                                                                     {"frame", EL::Value{2}}},
                                                                               {IO::Path{"maps/b_shell0.bsp"}, 1, 2}},
            
            }));

            CAPTURE(expression, variables);

            const auto modelDefinition = makeModelDefinition(expression);
            CHECK(modelDefinition.modelSpecification(EL::VariableTable{variables}) == expectedModelSpecification);
        }

        TEST_CASE("ModelDefinitionTest.defaultModelSpecification") {
            using T = std::tuple<std::string, ModelSpecification>;

            const auto 
            [expression,                                            expectedModelSpecification] = GENERATE(values<T>({
            {R"("maps/b_shell0.bsp")",                              {IO::Path{"maps/b_shell0.bsp"}, 0, 0}},
            {R"({ path: "maps/b_shell0.bsp", skin: 1, frame: 2 })", {IO::Path{"maps/b_shell0.bsp"}, 1, 2}},
            
            {R"({{
                spawnflags == 1 -> "maps/b_shell0.bsp",
                                   "maps/b_shell1.bsp"
            }})",                                                   {IO::Path{"maps/b_shell1.bsp"}, 0, 0}},

            {R"({path: model, skin: skin, frame: frame})",          {}},
            
            }));

            CAPTURE(expression);

            const auto modelDefinition = makeModelDefinition(expression);
            CHECK(modelDefinition.defaultModelSpecification() == expectedModelSpecification);
        }
    }
}

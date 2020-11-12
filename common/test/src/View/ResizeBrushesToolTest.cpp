/*
 Copyright (C) 2010-2017 Kristian Duske
 Copyright (C) 2020 Eric Wasylishen

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

#include "Model/Brush.h"
#include "Model/BrushBuilder.h"
#include "Model/BrushFace.h"
#include "Model/BrushNode.h"
#include "Model/LayerNode.h"
#include "Model/PickResult.h"
#include "Model/WorldNode.h"
#include "View/ResizeBrushesTool.h"

#include <kdl/result.h>
#include <kdl/string_utils.h>

#include <vecmath/ray.h>
#include <vecmath/scalar.h>
#include <vecmath/vec.h>
#include <vecmath/vec_io.h>

#include "Catch2.h"
#include "GTestCompat.h"

#include "MapDocumentTest.h"

namespace TrenchBroom {
    namespace View {
        class ResizeBrushesToolTest : public MapDocumentTest {
        public:
            ResizeBrushesToolTest() : MapDocumentTest(Model::MapFormat::Valve) {}
        };

        static const auto PickRay = vm::ray3(vm::vec3(0.0, -100.0, 0.0), vm::normalize(vm::vec3(-1.0, 1.0, 0)));

        TEST_CASE_METHOD(ResizeBrushesToolTest, "ResizeBrushesToolTest.pickBrush") {
            ResizeBrushesTool tool(document);

            const auto bboxMax = GENERATE(vm::vec3::fill(0.01),
                                          vm::vec3::fill(8.0));

            Model::BrushBuilder builder(document->world(), document->worldBounds());
            Model::BrushNode* brush1 = document->world()->createBrush(builder.createCuboid(vm::bbox3(vm::vec3::fill(0.0), bboxMax), "texture").value());

            document->addNode(brush1, document->currentLayer());
            document->select(brush1);

            const Model::Hit hit = tool.pick3D(PickRay, Model::PickResult());
            CHECK(hit.isMatch());
            CHECK(hit.type() == ResizeBrushesTool::Resize3DHitType);
            CHECK(!vm::is_nan(hit.hitPoint()));
            CHECK(!vm::is_nan(hit.distance()));

            const auto hitHandle = hit.target<ResizeBrushesTool::Resize3DHitData>();
            CHECK(hitHandle.node() == brush1);
            CHECK(hitHandle.faceIndex() == brush1->brush().findFace(vm::vec3::neg_x()).value());
        }
    }
}

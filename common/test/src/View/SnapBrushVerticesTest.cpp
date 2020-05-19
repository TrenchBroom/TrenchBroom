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

#include <catch2/catch.hpp>

#include "GTestCompat.h"

#include "Model/BrushNode.h"
#include "Model/NodeCollection.h"
#include "View/MapDocumentTest.h"
#include "View/MapDocument.h"
#include "View/Grid.h"

namespace TrenchBroom {
    namespace View {
        class SnapBrushVerticesTest : public MapDocumentTest {};

        // see https://github.com/kduske/TrenchBroom/issues/2244
        TEST_CASE_METHOD(SnapBrushVerticesTest, "SnapBrushVerticesTest.snapVerticesCrash_2244") {
            document->selectAllNodes();
            document->deleteObjects();

            const auto brush = R"(
// Game: Quake
// Format: Standard
// entity 0
{
"classname" "worldspawn"
// brush 0
{
( -96 -0 116 ) ( -96 -64 116 ) ( -96 -64 172 ) karch1 -0 -0 -0 1 1
( -96 -0 172 ) ( -96 -64 172 ) ( -116 -64 144 ) karch1 -84 176 -0 1 1
( -116 -64 144 ) ( -96 -64 116 ) ( -96 -0 116 ) karch_sup6 2 -64 -0 1 1
( -96 -0 116 ) ( -96 -0 172 ) ( -116 -0 144 ) karch1 -0 -0 -0 1 1
( -96 -64 172 ) ( -96 -64 116 ) ( -116 -64 144 ) karch1 -0 -0 -0 1 1
}
})";
            document->paste(brush);
            document->selectAllNodes();

            ASSERT_EQ(1u, document->selectedNodes().brushCount());
            ASSERT_NO_THROW(document->snapVertices(document->grid().actualSize()));
        }
    }
}

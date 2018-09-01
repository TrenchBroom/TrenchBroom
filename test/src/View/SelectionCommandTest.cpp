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

#include <stdio.h>

#include <gtest/gtest.h>

#include "Model/Brush.h"
#include "Model/BrushBuilder.h"
#include "Model/Entity.h"
#include "Model/Group.h"
#include "Model/Layer.h"
#include "Model/World.h"
#include "View/MapDocumentTest.h"
#include "View/MapDocument.h"

namespace TrenchBroom {
    namespace View {
        class SelectionCommandTest : public MapDocumentTest {};
        
        TEST_F(SelectionCommandTest, faceSelectionUndoAfterTranslationUndo) {
            Model::Brush* brush = createBrush();
            ASSERT_EQ(vec3::zero, brush->bounds().center());
            
            document->addNode(brush, document->currentParent());

            // select the top face
            document->select(brush->findFace(vec3::pos_z));
            ASSERT_EQ(Model::BrushFaceList{brush->findFace(vec3::pos_z)}, document->selectedBrushFaces());

            // deselect it
            document->deselect(brush->findFace(vec3::pos_z));
            ASSERT_EQ(Model::BrushFaceList{}, document->selectedBrushFaces());

            // select the brush
            document->select(brush);
            ASSERT_EQ(Model::BrushList{brush}, document->selectedNodes().brushes());

            // translate the brush
            document->translateObjects(vec3(10.0, 0.0, 0.0));
            ASSERT_EQ(vec3(10.0, 0.0, 0.0), brush->bounds().center());
            
            // Start undoing changes
            
            document->undoLastCommand();
            ASSERT_EQ(vec3::zero, brush->bounds().center());
            ASSERT_EQ(Model::BrushList{brush}, document->selectedNodes().brushes());
            ASSERT_EQ(Model::BrushFaceList{}, document->selectedBrushFaces());
            
            document->undoLastCommand();
            ASSERT_EQ(Model::BrushList{}, document->selectedNodes().brushes());
            ASSERT_EQ(Model::BrushFaceList{}, document->selectedBrushFaces());
            
            document->undoLastCommand();
            ASSERT_EQ(Model::BrushFaceList{brush->findFace(vec3::pos_z)}, document->selectedBrushFaces());
        }
    }
}

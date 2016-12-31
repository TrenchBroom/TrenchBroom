/*
 Copyright (C) 2010-2016 Kristian Duske
 
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

#include "Assets/Texture.h"
#include "Assets/TextureCollection.h"
#include "Assets/TextureManager.h"
#include "Model/Brush.h"
#include "Model/BrushFace.h"
#include "Model/Entity.h"
#include "Model/Group.h"
#include "Model/Layer.h"
#include "Model/World.h"
#include "View/MapDocumentTest.h"
#include "View/MapDocument.h"

#include <cassert>

namespace TrenchBroom {
    namespace View {
        class SnapshotTest : public MapDocumentTest {};
        
        TEST_F(SnapshotTest, setTexturesAfterRestore) {
            document->setEnabledTextureCollections(IO::Path::List{ IO::Path("data/IO/Wad/cr8_czg.wad") });
            
            Model::Brush* brush = createBrush("coffin1");
            document->addNode(brush, document->currentParent());
            
            const Assets::Texture* texture = document->textureManager().texture("coffin1");
            ASSERT_EQ(6u, texture->usageCount());
            
            for (Model::BrushFace* face : brush->faces())
                ASSERT_EQ(texture, face->texture());
            
            document->translateObjects(Vec3(1, 1, 1));
            ASSERT_EQ(6u, texture->usageCount());
            
            document->undoLastCommand();
            ASSERT_EQ(6u, texture->usageCount());
            
            for (Model::BrushFace* face : brush->faces())
                ASSERT_EQ(texture, face->texture());
        }
    }
}

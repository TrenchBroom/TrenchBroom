/*
 Copyright (C) 2010-2013 Kristian Duske
 
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
 along with TrenchBroom.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "MapRenderer.h"

#include "Model/Brush.h"
#include "Model/BrushFace.h"
#include "Model/Entity.h"
#include "Renderer/Mesh.h"

namespace TrenchBroom {
    namespace Renderer {
        struct BuildBrushFaceMesh {
            Model::BrushFace::Mesh mesh;
            inline void operator()(Model::BrushFace::Ptr face){
                face->addToMesh(mesh);
            }
        };
        
        struct BuildBrushFaceMeshFilter {
            inline bool operator()(Model::Entity::Ptr entity) {
                return true;
            }
            
            inline bool operator()(Model::Brush::Ptr brush) {
                return true;
            }
            
            inline bool operator()(Model::BrushFace::Ptr face) {
                return true;
            }
        };
        
        void MapRenderer::loadMap(const Model::Map::Ptr map) {
            BuildBrushFaceMesh builder;
            BuildBrushFaceMeshFilter filter;
            map->eachBrushFace(builder, filter);
        }
        
        void MapRenderer::clear() {
        }
        
        void MapRenderer::render(const RenderContext& context) {
        }
    }
}

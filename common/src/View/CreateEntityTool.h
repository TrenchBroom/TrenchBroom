/*
 Copyright (C) 2010-2014 Kristian Duske
 
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

#ifndef TrenchBroom_CreateEntityTool
#define TrenchBroom_CreateEntityTool

#include "TrenchBroom.h"
#include "VecMath.h"
#include "StringUtils.h"
#include "View/Tool.h"
#include "View/ViewTypes.h"

namespace TrenchBroom {
    namespace Model {
        class Entity;
        class PickResult;
    }
    
    namespace View {
        class CreateEntityTool : public Tool {
        private:
            MapDocumentWPtr m_document;
            Model::Entity* m_entity;
            BBox3 m_referenceBounds;
        public:
            CreateEntityTool(MapDocumentWPtr document);
            
            bool createEntity(const String& classname);
            void removeEntity();
            void commitEntity();
            
            void updateEntityPosition2D(const Ray3& pickRay);
            void updateEntityPosition3D(const Ray3& pickRay, const Model::PickResult& pickResult);
        };
    }
}

#endif /* defined(TrenchBroom_CreateEntityTool) */

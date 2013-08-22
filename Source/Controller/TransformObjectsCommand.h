/*
 Copyright (C) 2010-2012 Kristian Duske
 
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

#ifndef __TrenchBroom__TransformObjectsCommand__
#define __TrenchBroom__TransformObjectsCommand__

#include "Controller/SnapshotCommand.h"
#include "Controller/ObjectsCommand.h"

#include "Model/BrushTypes.h"
#include "Model/EntityTypes.h"
#include "Utility/VecMath.h"

using namespace TrenchBroom::VecMath;

namespace TrenchBroom {
    namespace Controller {
        class TransformObjectsCommand : public SnapshotCommand, ObjectsCommand {
        protected:
            Model::EntityList m_entities;
            Model::BrushList m_brushes;
            
            const Mat4f m_pointTransform;
            const Mat4f m_vectorTransform;
            bool m_lockTextures;
            bool m_invertOrientation;
            
            bool performDo();
            bool performUndo();

            TransformObjectsCommand(Model::MapDocument& document, const Model::EntityList& entities, const Model::BrushList& brushes, const wxString& name, const Mat4f& pointTransform, const Mat4f& vectorTransform, bool invertOrientation);
        public:
            static TransformObjectsCommand* translateObjects(Model::MapDocument& document, const Model::EntityList& entities, const Model::BrushList& brushes, const Vec3f& delta);
            static TransformObjectsCommand* translateEntity(Model::MapDocument& document, Model::Entity& entity, const Vec3f& delta);
            static TransformObjectsCommand* rotateObjects(Model::MapDocument& document, const Model::EntityList& entities, const Model::BrushList& brushes, const Vec3f& axis, float angle, bool clockwise, const Vec3f& center);
            static TransformObjectsCommand* flipObjects(Model::MapDocument& document, const Model::EntityList& entities, const Model::BrushList& brushes, const Axis::Type& axis, const Vec3f& center);
            
            const Model::EntityList& entities() const {
                return m_entities;
            }
            
            const Model::BrushList& brushes() const {
                return m_brushes;
            }
        };
    }
}

#endif /* defined(__TrenchBroom__TransformObjectsCommand__) */

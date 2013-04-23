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

#ifndef __TrenchBroom__RotateObjectsCommand__
#define __TrenchBroom__RotateObjectsCommand__

#include "Controller/SnapshotCommand.h"
#include "Controller/ObjectsCommand.h"

#include "Model/BrushTypes.h"
#include "Model/EntityTypes.h"
#include "Utility/VecMath.h"

using namespace TrenchBroom::VecMath;

namespace TrenchBroom {
    namespace Controller {
        class RotateObjectsCommand : public SnapshotCommand, ObjectsCommand {
        protected:
            Model::EntityList m_entities;
            Model::BrushList m_brushes;
            
            Vec3f m_axis;
            float m_angle;
            Vec3f m_center;
            bool m_lockTextures;
            
            bool performDo();
            bool performUndo();

            RotateObjectsCommand(Model::MapDocument& document, const Model::EntityList& entities, const Model::BrushList& brushes, const wxString& name, const Vec3f& axis, float angle, bool clockwise, const Vec3f& center, bool lockTextures);
        public:
            static RotateObjectsCommand* rotate(Model::MapDocument& document, const Model::EntityList& entities, const Model::BrushList& brushes, const Vec3f& axis, float angle, bool clockwise, const Vec3f& center, bool lockTextures);
            
            const Model::EntityList& entities() const {
                return m_entities;
            }
            
            const Model::BrushList& brushes() const {
                return m_brushes;
            }
        };
    }
}

#endif /* defined(__TrenchBroom__RotateObjectsCommand__) */

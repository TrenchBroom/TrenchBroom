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

#ifndef __TrenchBroom__FlipObjectsCommand__
#define __TrenchBroom__FlipObjectsCommand__

#include "Controller/Command.h"

#include "Model/BrushTypes.h"
#include "Model/EntityTypes.h"
#include "Utility/VecMath.h"

using namespace TrenchBroom::VecMath;

namespace TrenchBroom {
    namespace Controller {
        class FlipObjectsCommand : public DocumentCommand {
        protected:
            Model::EntityList m_entities;
            Model::BrushList m_brushes;
            
            Axis::Type m_axis;
            Vec3f m_center;
            bool m_lockTextures;
            
            bool performDo();
            bool performUndo();
            
            FlipObjectsCommand(Model::MapDocument& document, const Model::EntityList& entities, const Model::BrushList& brushes, const wxString& name, Axis::Type axis, const Vec3f& center, bool lockTextures) :
            DocumentCommand(FlipObjects, document, true, name, true),
            m_entities(entities),
            m_brushes(brushes),
            m_axis(axis),
            m_center(center),
            m_lockTextures(lockTextures) {}
        public:
            static FlipObjectsCommand* flip(Model::MapDocument& document, const Model::EntityList& entities, const Model::BrushList& brushes, Axis::Type axis, const Vec3f& center, bool lockTextures);
        };
    }
}


#endif /* defined(__TrenchBroom__FlipObjectsCommand__) */

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

#ifndef __TrenchBroom__MoveEdgesCommand__
#define __TrenchBroom__MoveEdgesCommand__

#include "Controller/SnapshotCommand.h"
#include "Model/BrushGeometryTypes.h"
#include "Model/BrushTypes.h"
#include "Utility/VecMath.h"

#include <map>

using namespace TrenchBroom::Math;

namespace TrenchBroom {
    namespace Controller {
        class MoveEdgesCommand : public SnapshotCommand {
        protected:
            typedef std::map<Model::Brush*, Model::EdgeInfoList> BrushEdgesMap;
            typedef std::pair<Model::Brush*, Model::EdgeInfoList> BrushEdgesMapEntry;
            typedef std::pair<BrushEdgesMap::iterator, bool> BrushEdgesMapInsertResult;

            Model::BrushList m_brushes;
            Model::EdgeInfoList m_originalEdges;
            Model::EdgeInfoList m_edges;
            BrushEdgesMap m_brushEdges;
            Vec3f m_delta;

            bool performDo();
            bool performUndo();

            MoveEdgesCommand(Model::MapDocument& document, const wxString& name, const Model::VertexToEdgesMap& brushEdges, const Vec3f& delta);
        public:
            static MoveEdgesCommand* moveEdges(Model::MapDocument& document, const Model::VertexToEdgesMap& brushEdges, const Vec3f& delta);

            bool canDo() const;

            inline const Model::BrushList& brushes() const {
                return m_brushes;
            }

            inline const Model::EdgeInfoList& edges() const {
                return m_edges;
            }
        };
    }
}

#endif /* defined(__TrenchBroom__MoveEdgesCommand__) */

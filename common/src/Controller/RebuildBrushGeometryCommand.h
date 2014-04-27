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

#ifndef __TrenchBroom__RebuildBrushGeometryCommand__
#define __TrenchBroom__RebuildBrushGeometryCommand__

#include "StringUtils.h"
#include "SharedPointer.h"
#include "Controller/Command.h"
#include "Controller/BrushVertexHandleCommand.h"
#include "Model/ModelTypes.h"
#include "Model/Snapshot.h"
#include "View/ViewTypes.h"

#include <map>

namespace TrenchBroom {
    namespace Controller {
        class RebuildBrushGeometryCommand : public BrushVertexHandleCommand {
        public:
            static const CommandType Type;
            typedef std::tr1::shared_ptr<RebuildBrushGeometryCommand> Ptr;
        private:
            static const FloatType MaxDistance;
            typedef std::map<Model::Brush*, Vec3::List> BrushVerticesMap;
            
            View::MapDocumentWPtr m_document;
            Model::BrushList m_brushes;
            Vec3::List m_selectedVertexHandles;
            Vec3::List m_selectedEdgeHandles;
            Vec3::List m_selectedFaceHandles;
        public:
            static Ptr rebuildBrushGeometry(View::MapDocumentWPtr document, const Model::BrushList& brushes);
        private:
            RebuildBrushGeometryCommand(View::MapDocumentWPtr document, const Model::BrushList& brushes);
            
            bool doPerformDo();
            
            void doRemoveBrushes(View::VertexHandleManager& manager);
            
            template <typename T, typename O>
            Vec3::List getSelectedHandles(const std::map<Vec3, T, O>& handles) const {
                Vec3::List result;
                result.reserve(handles.size());
                
                typename std::map<Vec3, T, O>::const_iterator it, end;
                for (it = handles.begin(), end = handles.end(); it != end; ++it) {
                    const Vec3& position = it->first;
                    result.push_back(position);
                }
                return result;
            }
            
            void doAddBrushes(View::VertexHandleManager& manager);
            void doSelectNewHandlePositions(View::VertexHandleManager& manager);

            void selectNewVertexHandlePositions(View::VertexHandleManager& manager) const;
            Vec3::List findVertexHandlePositions(const Vec3& original) const;

            void selectNewEdgeHandlePositions(View::VertexHandleManager& manager) const;
            Vec3::List findEdgeHandlePositions(const Vec3& original) const;
            
            void selectNewFaceHandlePositions(View::VertexHandleManager& manager) const;
            Vec3::List findFaceHandlePositions(const Vec3& original) const;
            
            void doSelectOldHandlePositions(View::VertexHandleManager& manager);
        };
    }
}

#endif /* defined(__TrenchBroom__RebuildBrushGeometryCommand__) */

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

#ifndef __TrenchBroom__MoveBrushFacesCommand__
#define __TrenchBroom__MoveBrushFacesCommand__

#include "StringUtils.h"
#include "SharedPointer.h"
#include "Controller/Command.h"
#include "Controller/BrushVertexHandleCommand.h"
#include "Model/ModelTypes.h"
#include "Model/Snapshot.h"
#include "View/ViewTypes.h"

namespace TrenchBroom {
    namespace Controller {
        class MoveBrushFacesCommand : public BrushVertexHandleCommand {
        public:
            static const CommandType Type;
            typedef std::tr1::shared_ptr<MoveBrushFacesCommand> Ptr;
        private:
            typedef std::map<Model::Brush*, Polygon3::List> BrushFacesMap;
            
            View::MapDocumentWPtr m_document;
            
            Model::BrushList m_brushes;
            BrushFacesMap m_brushFaces;
            Polygon3::List m_oldFacePositions;
            Polygon3::List m_newFacePositions;
            Vec3 m_delta;
            
            Model::Snapshot m_snapshot;
        public:
            static Ptr moveFaces(View::MapDocumentWPtr document, const Model::VertexToFacesMap& faces, const Vec3& delta);
        private:
            MoveBrushFacesCommand(View::MapDocumentWPtr document, const Model::VertexToFacesMap& faces, const Vec3& delta);
            
            bool doPerformDo();
            bool canPerformDo(View::MapDocumentSPtr document) const;
            bool doPerformUndo();
            
            void doRemoveBrushes(View::VertexHandleManager& manager) const;
            void doAddBrushes(View::VertexHandleManager& manager) const;
            void doSelectNewHandlePositions(View::VertexHandleManager& manager) const;
            void doSelectOldHandlePositions(View::VertexHandleManager& manager) const;
            
            static String makeName(const Model::VertexToFacesMap& faces);
            void extractFaces(const Model::VertexToFacesMap& faces);
        };
    }
}

#endif /* defined(__TrenchBroom__MoveBrushFacesCommand__) */

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

#ifndef __TrenchBroom__MoveBrushVerticesCommand__
#define __TrenchBroom__MoveBrushVerticesCommand__

#include "StringUtils.h"
#include "SharedPointer.h"
#include "Controller/Command.h"
#include "Controller/BrushVertexHandleCommand.h"
#include "Model/ModelTypes.h"
#include "Model/Snapshot.h"
#include "View/ViewTypes.h"

namespace TrenchBroom {
    namespace Controller {
        class MoveBrushVerticesCommand : public BrushVertexHandleCommand {
        public:
            static const CommandType Type;
            typedef std::tr1::shared_ptr<MoveBrushVerticesCommand> Ptr;
        private:
            typedef std::map<Model::Brush*, Vec3::List> BrushVerticesMap;

            View::MapDocumentWPtr m_document;
            
            Model::BrushList m_brushes;
            BrushVerticesMap m_brushVertices;
            Vec3::List m_oldVertexPositions;
            Vec3::List m_newVertexPositions;
            Vec3 m_delta;
            
            Model::Snapshot m_snapshot;
        public:
            static Ptr moveVertices(View::MapDocumentWPtr document, const Model::VertexToBrushesMap& vertices, const Vec3& delta);
            bool hasRemainingVertices() const;
        private:
            MoveBrushVerticesCommand(View::MapDocumentWPtr document, const Model::VertexToBrushesMap& vertices, const Vec3& delta);
            
            bool doPerformDo();
            bool canPerformDo(View::MapDocumentSPtr document) const;
            bool doPerformUndo();
            bool doCollateWith(Command::Ptr command);

            void doRemoveBrushes(View::VertexHandleManager& manager);
            void doAddBrushes(View::VertexHandleManager& manager);
            void doSelectNewHandlePositions(View::VertexHandleManager& manager);
            void doSelectOldHandlePositions(View::VertexHandleManager& manager);

            static String makeName(const Model::VertexToBrushesMap& vertices);
            void extractVertices(const Model::VertexToBrushesMap& vertices);
        };
    }
}

#endif /* defined(__TrenchBroom__MoveBrushVerticesCommand__) */

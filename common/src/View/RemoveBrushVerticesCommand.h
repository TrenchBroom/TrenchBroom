/*
 Copyright (C) 2010-2017 Kristian Duske
 
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

#ifndef TrenchBroom_RemoveBrushVerticesCommand
#define TrenchBroom_RemoveBrushVerticesCommand

#include "SharedPointer.h"
#include "Model/ModelTypes.h"
#include "View/RemoveBrushElementsCommand.h"

namespace TrenchBroom {
    namespace Model {
        class Snapshot;
    }
    
    namespace View {
        class VertexHandleManagerOld;
        
        class RemoveBrushVerticesCommand : public RemoveBrushElementsCommand {
        public:
            static const CommandType Type;
            typedef std::shared_ptr<RemoveBrushVerticesCommand> Ptr;
        private:
            Vec3::List m_oldVertexPositions;
        public:
            static Ptr remove(const Model::VertexToBrushesMap& vertices);
        private:
            RemoveBrushVerticesCommand(const Model::BrushList& brushes, const Model::BrushVerticesMap& vertices, const Vec3::List& vertexPositions);
            
            void doSelectOldHandlePositions(VertexHandleManager& manager, const Model::BrushList& brushes);

            void doSelectOldHandlePositions(VertexHandleManagerOld& manager, const Model::BrushList& brushes);
        };
    }
}

#endif /* defined(TrenchBroom_RemoveBrushVerticesCommand) */

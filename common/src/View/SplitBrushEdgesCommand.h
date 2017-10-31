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

#ifndef TrenchBroom_SplitBrushEdgesCommand
#define TrenchBroom_SplitBrushEdgesCommand

#include "SharedPointer.h"
#include "Model/ModelTypes.h"
#include "View/VertexCommand.h"

namespace TrenchBroom {
    namespace Model {
        class Snapshot;
    }
    
    namespace View {
        class VertexHandleManagerOld;
        
        class SplitBrushEdgesCommand : public VertexCommand {
        public:
            static const CommandType Type;
            typedef std::shared_ptr<SplitBrushEdgesCommand> Ptr;
        private:
            Model::BrushEdgesMap m_edges;
            Edge3::List m_oldEdgePositions;
            Vec3::List m_newVertexPositions;
            Vec3 m_delta;
        public:
            static Ptr split(const Model::VertexToEdgesMap& edges, const Vec3& delta);
        private:
            SplitBrushEdgesCommand(const Model::BrushList& brushes, const Model::BrushEdgesMap& edges, const Edge3::List& edgePositions, const Vec3& delta);
            
            bool doCanDoVertexOperation(const MapDocument* document) const override;
            bool doVertexOperation(MapDocumentCommandFacade* document) override;
            
            void doSelectNewHandlePositions(VertexHandleManagerOld& manager, const Model::BrushList& brushes) override;
            void doSelectOldHandlePositions(VertexHandleManagerOld& manager, const Model::BrushList& brushes) override;
            
            bool doCollateWith(UndoableCommand::Ptr command) override;
            
            void doSelectNewHandlePositions(VertexHandleManagerBaseT<Vec3>& manager) const override;
            void doSelectOldHandlePositions(VertexHandleManagerBaseT<Vec3>& manager) const override;
        };
    }
}

#endif /* defined(TrenchBroom_SplitBrushEdgesCommand) */

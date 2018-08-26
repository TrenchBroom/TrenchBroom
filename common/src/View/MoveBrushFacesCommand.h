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

#ifndef TrenchBroom_MoveBrushFacesCommand
#define TrenchBroom_MoveBrushFacesCommand

#include "SharedPointer.h"
#include "Model/ModelTypes.h"
#include "View/VertexCommand.h"

namespace TrenchBroom {
    namespace Model {
        class Snapshot;
    }
    
    namespace View {
        class MoveBrushFacesCommand : public VertexCommand {
        public:
            static const CommandType Type;
            typedef std::shared_ptr<MoveBrushFacesCommand> Ptr;
        private:
            Model::BrushFacesMap m_faces;
            Polygon3::List m_oldFacePositions;
            Polygon3::List m_newFacePositions;
            vec3 m_delta;
        public:
            static Ptr move(const Model::FaceToBrushesMap& faces, const vec3& delta);
        private:
            MoveBrushFacesCommand(const Model::BrushList& brushes, const Model::BrushFacesMap& faces, const Polygon3::List& facePositions, const vec3& delta);
            
            bool doCanDoVertexOperation(const MapDocument* document) const override;
            bool doVertexOperation(MapDocumentCommandFacade* document) override;

            bool doCollateWith(UndoableCommand::Ptr command) override;

            void doSelectNewHandlePositions(VertexHandleManagerBaseT<Polygon3>& manager) const override;
            void doSelectOldHandlePositions(VertexHandleManagerBaseT<Polygon3>& manager) const override;
        };
    }
}

#endif /* defined(TrenchBroom_MoveBrushFacesCommand) */

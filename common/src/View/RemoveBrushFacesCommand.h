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

#ifndef TrenchBroom_RemoveBrushFacesCommand
#define TrenchBroom_RemoveBrushFacesCommand

#include "Model/ModelTypes.h"
#include "View/RemoveBrushElementsCommand.h"

#include <vector>

namespace TrenchBroom {
    namespace Model {
        class Snapshot;
    }

    namespace View {
        class RemoveBrushFacesCommand : public RemoveBrushElementsCommand {
        public:
            static const CommandType Type;
            using Ptr = std::shared_ptr<RemoveBrushFacesCommand>;
        private:
            std::vector<vm::polygon3> m_oldFacePositions;
        public:
            static Ptr remove(const FaceToBrushesMap& faces);
        private:
            RemoveBrushFacesCommand(const std::vector<Model::Brush*>& brushes, const BrushVerticesMap& vertices, const std::vector<vm::polygon3>& facePositions);

            void doSelectOldHandlePositions(VertexHandleManagerBaseT<vm::polygon3>& manager) const override;
        };
    }
}

#endif /* defined(TrenchBroom_RemoveBrushFacesCommand) */

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

#pragma once

#include "Macros.h"
#include "View/VertexCommand.h"

#include <memory>
#include <string>
#include <vector>

namespace TrenchBroom {
    namespace View {
        class MapDocument;

        class AddBrushVerticesCommand : public VertexCommand {
        public:
            static const CommandType Type;
        private:
            VertexToBrushesMap m_vertices;
        public:
            static std::unique_ptr<AddBrushVerticesCommand> add(const VertexToBrushesMap& vertices);

            AddBrushVerticesCommand(CommandType type, const std::string& name, const std::vector<Model::BrushNode*>& brushes, const VertexToBrushesMap& vertices);
        private:
            bool doCanDoVertexOperation(const MapDocument* document) const override;
            bool doVertexOperation(MapDocumentCommandFacade* document) override;

            bool doCollateWith(UndoableCommand* command) override;

            deleteCopyAndMove(AddBrushVerticesCommand)
        };
    }
}

#endif /* AddBrushVerticesCommand_h */

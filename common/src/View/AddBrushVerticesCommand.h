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

#ifndef AddBrushVerticesCommand_h
#define AddBrushVerticesCommand_h

#include "Model/Model_Forward.h"
#include "View/VertexCommand.h"

#include <string>
#include <vector>

namespace TrenchBroom {
    namespace View {
        class MapDocument;

        class AddBrushVerticesCommand : public VertexCommand {
        public:
            static const CommandType Type;
            using Ptr = std::shared_ptr<AddBrushVerticesCommand>;
        private:
            VertexToBrushesMap m_vertices;
        public:
            static Ptr add(const VertexToBrushesMap& vertices);
        protected:
            AddBrushVerticesCommand(CommandType type, const std::string& name, const std::vector<Model::Brush*>& brushes, const VertexToBrushesMap& vertices);
        private:
            bool doCanDoVertexOperation(const MapDocument* document) const override;
            bool doVertexOperation(MapDocumentCommandFacade* document) override;

            bool doCollateWith(UndoableCommand::Ptr command) override;
        };
    }
}

#endif /* AddBrushVerticesCommand_h */

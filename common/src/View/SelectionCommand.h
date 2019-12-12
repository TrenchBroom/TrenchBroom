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

#ifndef TrenchBroom_SelectionCommand
#define TrenchBroom_SelectionCommand

#include "Model/Model_Forward.h"
#include "Model/BrushFaceReference.h"
#include "View/UndoableCommand.h"

#include <string>
#include <vector>

namespace TrenchBroom {
    namespace View {
        class SelectionCommand : public UndoableCommand {
        public:
            static const CommandType Type;
            using Ptr = std::shared_ptr<SelectionCommand>;
        private:
            typedef enum {
                Action_SelectNodes,
                Action_SelectFaces,
                Action_SelectAllNodes,
                Action_SelectAllFaces,
                Action_ConvertToFaces,
                Action_DeselectNodes,
                Action_DeselectFaces,
                Action_DeselectAll
            } Action;

            Action m_action;

            std::vector<Model::Node*> m_nodes;
            std::vector<Model::BrushFaceReference> m_faceRefs;

            std::vector<Model::Node*> m_previouslySelectedNodes;
            std::vector<Model::BrushFaceReference> m_previouslySelectedFaceRefs;
        public:
            static Ptr select(const std::vector<Model::Node*>& nodes);
            static Ptr select(const std::vector<Model::BrushFace*>& faces);

            static Ptr convertToFaces();
            static Ptr selectAllNodes();
            static Ptr selectAllFaces();

            static Ptr deselect(const std::vector<Model::Node*>& nodes);
            static Ptr deselect(const std::vector<Model::BrushFace*>& faces);
            static Ptr deselectAll();
        private:
            SelectionCommand(Action action, const std::vector<Model::Node*>& nodes, const std::vector<Model::BrushFace*>& faces);
            static std::string makeName(Action action, const std::vector<Model::Node*>& nodes, const std::vector<Model::BrushFace*>& faces);
        private:
            bool doPerformDo(MapDocumentCommandFacade* document) override;
            bool doPerformUndo(MapDocumentCommandFacade* document) override;

            bool doIsRepeatDelimiter() const override;
            bool doIsRepeatable(MapDocumentCommandFacade* document) const override;

            bool doCollateWith(UndoableCommand::Ptr command) override;
        };
    }
}

#endif /* defined(TrenchBroom_SelectionCommand) */

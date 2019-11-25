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

#ifndef TrenchBroom_DuplicateNodesCommand
#define TrenchBroom_DuplicateNodesCommand

#include "Model/ModelTypes.h"
#include "View/DocumentCommand.h"

#include <map>
#include <vector>

namespace TrenchBroom {
    namespace View {
        class DuplicateNodesCommand : public DocumentCommand {
        public:
            static const CommandType Type;
            using Ptr = std::shared_ptr<DuplicateNodesCommand>;
        private:
            std::vector<Model::Node*> m_previouslySelectedNodes;
            std::vector<Model::Node*> m_nodesToSelect;
            std::map<Model::Node*, std::vector<Model::Node*>> m_addedNodes;
            bool m_firstExecution;
        public:
            static Ptr duplicate();
        private:
            DuplicateNodesCommand();
        public:
            ~DuplicateNodesCommand() override;
        private:
            bool doPerformDo(MapDocumentCommandFacade* document) override;
            bool doPerformUndo(MapDocumentCommandFacade* document) override;

            class CloneParentQuery;
            bool cloneParent(const Model::Node* node) const;

            bool doIsRepeatable(MapDocumentCommandFacade* document) const override;
            UndoableCommand::Ptr doRepeat(MapDocumentCommandFacade* document) const override;

            bool doCollateWith(UndoableCommand::Ptr command) override;
        };
    }
}

#endif /* defined(TrenchBroom_DuplicateNodesCommand) */

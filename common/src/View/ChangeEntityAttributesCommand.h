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

#ifndef TrenchBroom_ChangeEntityAttributesCommand
#define TrenchBroom_ChangeEntityAttributesCommand

#include "Macros.h"
#include "View/DocumentCommand.h"

#include <map>
#include <memory>
#include <string>
#include <vector>

namespace TrenchBroom {
    namespace Model {
        class AttributableNode;
        class EntityAttributeSnapshot;
    }

    namespace View {
        class MapDocumentCommandFacade;

        class ChangeEntityAttributesCommand : public DocumentCommand {
        public:
            static const CommandType Type;
        private:
            enum class Action {
                Set,
                Remove,
                Rename
            };
            enum class Target {
                SelectedNodes,
                NodeList
            };

            Action m_action;
            Target m_target;
            std::string m_oldName;
            std::string m_newName;
            std::string m_newValue;

            std::map<Model::AttributableNode*, std::vector<Model::EntityAttributeSnapshot>> m_snapshots;
            /**
             * Only used if m_target == NodeList
             */
            std::vector<Model::AttributableNode*> m_targetNodes;
        public:
            static std::unique_ptr<ChangeEntityAttributesCommand> set(const std::string& name, const std::string& value);
            static std::unique_ptr<ChangeEntityAttributesCommand> remove(const std::string& name);
            static std::unique_ptr<ChangeEntityAttributesCommand> rename(const std::string& oldName, const std::string& newName);

            static std::unique_ptr<ChangeEntityAttributesCommand>    setForNodes(const std::vector<Model::AttributableNode*>& nodes, const std::string& name, const std::string& value);
            static std::unique_ptr<ChangeEntityAttributesCommand> removeForNodes(const std::vector<Model::AttributableNode*>& nodes, const std::string& name);
            static std::unique_ptr<ChangeEntityAttributesCommand> renameForNodes(const std::vector<Model::AttributableNode*>& nodes, const std::string& oldName, const std::string& newName);
        public:
            ChangeEntityAttributesCommand(Action action, Target target);
            ~ChangeEntityAttributesCommand() override;
        private:
            static std::string makeName(Action action);

            void setName(const std::string& name);
            void setNewName(const std::string& newName);
            void setNewValue(const std::string& newValue);
            void setTargetNodes(const std::vector<Model::AttributableNode*>& nodes);

            std::unique_ptr<CommandResult> doPerformDo(MapDocumentCommandFacade* document) override;
            std::unique_ptr<CommandResult> doPerformUndo(MapDocumentCommandFacade* document) override;

            bool doIsRepeatable(MapDocumentCommandFacade* document) const override;

            bool doCollateWith(UndoableCommand* command) override;

            deleteCopyAndMove(ChangeEntityAttributesCommand)
        };
    }
}

#endif /* defined(TrenchBroom_ChangeEntityAttributesCommand) */

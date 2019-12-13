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
#include "Model/EntityAttributeSnapshot.h"
#include "Model/Model_Forward.h"
#include "View/DocumentCommand.h"
#include "View/View_Forward.h"

#include <map>
#include <memory>
#include <string>
#include <vector>

namespace TrenchBroom {
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

            Action m_action;
            Model::AttributeName m_oldName;
            Model::AttributeName m_newName;
            Model::AttributeValue m_newValue;

            std::map<Model::AttributableNode*, std::vector<Model::EntityAttributeSnapshot>> m_snapshots;
        public:
            static std::unique_ptr<ChangeEntityAttributesCommand> set(const Model::AttributeName& name, const Model::AttributeValue& value);
            static std::unique_ptr<ChangeEntityAttributesCommand> remove(const Model::AttributeName& name);
            static std::unique_ptr<ChangeEntityAttributesCommand> rename(const Model::AttributeName& oldName, const Model::AttributeName& newName);
        public:
            ChangeEntityAttributesCommand(Action action);
        private:
            static std::string makeName(Action action);

            void setName(const Model::AttributeName& name);
            void setNewName(const Model::AttributeName& newName);
            void setNewValue(const Model::AttributeValue& newValue);

            bool doPerformDo(MapDocumentCommandFacade* document) override;
            bool doPerformUndo(MapDocumentCommandFacade* document) override;

            bool doIsRepeatable(MapDocumentCommandFacade* document) const override;

            bool doCollateWith(UndoableCommand* command) override;

            deleteCopyAndMove(ChangeEntityAttributesCommand)
        };
    }
}

#endif /* defined(TrenchBroom_ChangeEntityAttributesCommand) */

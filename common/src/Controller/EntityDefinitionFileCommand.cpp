/*
 Copyright (C) 2010-2014 Kristian Duske
 
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

#include "EntityDefinitionFileCommand.h"

#include "Model/Entity.h"
#include "Model/EntityProperties.h"
#include "View/MapDocument.h"

namespace TrenchBroom {
    namespace Controller {
        const Command::CommandType EntityDefinitionFileCommand::Type = Command::freeType();

        EntityDefinitionFileCommand::Ptr EntityDefinitionFileCommand::setEntityDefinitionFileSpec(View::MapDocumentWPtr document, const Model::EntityDefinitionFileSpec& spec) {
            return Ptr(new EntityDefinitionFileCommand(document, spec));
        }

        EntityDefinitionFileCommand::Ptr EntityDefinitionFileCommand::reloadEntityDefinitionFile(View::MapDocumentWPtr document) {
            return setEntityDefinitionFileSpec(document, lock(document)->entityDefinitionFile());
        }

        EntityDefinitionFileCommand::EntityDefinitionFileCommand(View::MapDocumentWPtr document, const Model::EntityDefinitionFileSpec& spec) :
        DocumentCommand(Type, "Set Entity Definition File", true, document),
        m_newSpec(spec) {}
        
        bool EntityDefinitionFileCommand::doPerformDo() {
            View::MapDocumentSPtr document = lockDocument();
            Model::Entity* worldspawn = document->worldspawn();
            m_oldSpec = document->entityDefinitionFile();
            worldspawn->addOrUpdateProperty(Model::PropertyKeys::EntityDefinitions, m_newSpec.asString());
            document->entityPropertyDidChangeNotifier(worldspawn, Model::PropertyKeys::EntityDefinitions, m_oldSpec.asString(), Model::PropertyKeys::EntityDefinitions, m_newSpec.asString());
            document->entityDefinitionsDidChangeNotifier();
            return true;
        }
        
        bool EntityDefinitionFileCommand::doPerformUndo() {
            View::MapDocumentSPtr document = lockDocument();
            Model::Entity* worldspawn = document->worldspawn();
            worldspawn->addOrUpdateProperty(Model::PropertyKeys::EntityDefinitions, m_oldSpec.asString());
            document->entityPropertyDidChangeNotifier(worldspawn, Model::PropertyKeys::EntityDefinitions, m_newSpec.asString(), Model::PropertyKeys::EntityDefinitions, m_oldSpec.asString());
            document->entityDefinitionsDidChangeNotifier();
            return true;
        }

        bool EntityDefinitionFileCommand::doIsRepeatable(View::MapDocumentSPtr document) const {
            return false;
        }

        bool EntityDefinitionFileCommand::doCollateWith(Command::Ptr command) {
            return false;
        }
    }
}

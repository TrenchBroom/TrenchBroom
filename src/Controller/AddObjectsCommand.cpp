/*
 Copyright (C) 2010-2013 Kristian Duske
 
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
 along with TrenchBroom.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "AddObjectsCommand.h"

#include "CollectionUtils.h"
#include "Model/Entity.h"
#include "Model/ModelUtils.h"
#include "View/MapDocument.h"

namespace TrenchBroom {
    namespace Controller {
        const Command::CommandType AddObjectsCommand::Type = Command::freeType();

        class AddEntity {
        private:
            View::MapDocumentPtr m_document;
        public:
            Model::BrushList addedBrushes;

            AddEntity(View::MapDocumentPtr document) :
            m_document(document) {}
            
            inline void operator()(Model::Entity* entity) {
                m_document->addEntity(entity);
                VectorUtils::concatenate(addedBrushes, entity->brushes());
            }
        };
        
        AddObjectsCommand::AddObjectsCommand(View::MapDocumentPtr document, const Model::EntityList& entities, const Model::BrushList& brushes) :
        Command(Type, "", true),
        m_document(document) {
            // todo: transform entities so that worldspawn brushes get folded into the given brush list and worldspawn is excluded ( deleted? )
        }

        bool AddObjectsCommand::doPerformDo() {
            m_addedEntities = m_entitiesToAdd;
            m_addedBrushes = m_brushesToAdd;
            m_hasAddedBrushes = !m_addedBrushes.empty();
            
            // m_document->addBrushes(m_document->worldspawn(), m_brushesToAdd);
            
            AddEntity addEntity(m_document);
            Model::each(m_entitiesToAdd.begin(), m_entitiesToAdd.end(), addEntity, Model::MatchAll());
            VectorUtils::concatenate(m_addedBrushes, addEntity.addedBrushes);

            return true;
        }
        
        bool AddObjectsCommand::doPerformUndo() {
        }
    }
}

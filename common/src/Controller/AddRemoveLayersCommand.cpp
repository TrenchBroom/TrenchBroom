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

#include "AddRemoveLayersCommand.h"

#include "CollectionUtils.h"
#include "Model/Layer.h"
#include "View/MapDocument.h"

namespace TrenchBroom {
    namespace Controller {
        const Command::CommandType AddRemoveLayersCommand::Type = Command::freeType();

        AddRemoveLayersCommand::~AddRemoveLayersCommand() {
            VectorUtils::clearAndDelete(m_layersToAdd);
        }
        
        AddRemoveLayersCommand::Ptr AddRemoveLayersCommand::addLayers(View::MapDocumentWPtr document, const Model::LayerList& layers) {
            return Ptr(new AddRemoveLayersCommand(document, Action_Add, layers));
        }
        
        AddRemoveLayersCommand::Ptr AddRemoveLayersCommand::removeLayers(View::MapDocumentWPtr document, const Model::LayerList& layers) {
            return Ptr(new AddRemoveLayersCommand(document, Action_Remove, layers));
        }

        AddRemoveLayersCommand::AddRemoveLayersCommand(View::MapDocumentWPtr document, const Action action, const Model::LayerList& layers) :
        DocumentCommand(Type, StringUtils::safePlural(layers.size(), "Add Layer", "Add Layers"), true, document),
        m_action(action) {
            if (m_action == Action_Add)
                m_layersToAdd = layers;
            else
                m_layersToRemove = layers;
        }
        
        bool AddRemoveLayersCommand::doPerformDo() {
            if (m_action == Action_Add)
                addLayers(m_layersToAdd);
            else
                removeLayers(m_layersToRemove);

            using std::swap;
            swap(m_layersToAdd, m_layersToRemove);
            return true;
        }
        
        bool AddRemoveLayersCommand::doPerformUndo() {
            if (m_action == Action_Add)
                removeLayers(m_layersToRemove);
            else
                addLayers(m_layersToAdd);
            
            using std::swap;
            swap(m_layersToAdd, m_layersToRemove);
            return true;
        }
        
        bool AddRemoveLayersCommand::doIsRepeatable(View::MapDocumentSPtr document) const {
            return false;
        }
        
        bool AddRemoveLayersCommand::doCollateWith(Command::Ptr command) {
            return false;
        }
        
        void AddRemoveLayersCommand::addLayers(const Model::LayerList& layers) {
            View::MapDocumentSPtr document = lockDocument();
            
            document->addLayers(layers);
            document->layersWereAddedNotifier(layers);
        }
        
        void AddRemoveLayersCommand::removeLayers(const Model::LayerList& layers) {
            View::MapDocumentSPtr document = lockDocument();
            
            document->layersWillBeRemovedNotifier(layers);
            document->removeLayers(layers);
            document->layersWereRemovedNotifier(layers);
        }
    }
}

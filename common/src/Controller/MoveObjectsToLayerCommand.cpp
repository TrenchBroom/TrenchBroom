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

#include "MoveObjectsToLayerCommand.h"

#include "CollectionUtils.h"
#include "Macros.h"
#include "Model/Layer.h"
#include "Model/Object.h"
#include "View/MapDocument.h"

#include <algorithm>
#include <cassert>

namespace TrenchBroom {
    namespace Controller {
        const Command::CommandType MoveObjectsToLayerCommand::Type = Command::freeType();

        MoveObjectsToLayerCommand::Ptr MoveObjectsToLayerCommand::moveObjects(View::MapDocumentWPtr document, Model::Layer* layer, const Model::ObjectList& objects) {
            return Ptr(new MoveObjectsToLayerCommand(document, layer, objects));
        }

        MoveObjectsToLayerCommand::MoveObjectsToLayerCommand(View::MapDocumentWPtr document, Model::Layer* layer, const Model::ObjectList& objects) :
        DocumentCommand(Type, StringUtils::safePlural(objects.size(), "Move Object to Layer", "Move Objects to Layer"), true, document),
        m_layer(layer),
        m_objects(objects) {
            Model::ObjectList::iterator it = m_objects.begin();
            Model::ObjectList::iterator end = m_objects.end();
            while (it != end) {
                Model::Object* object = *it;
                Model::Layer* originalLayer = object->layer();
                if (originalLayer == m_layer) {
                    --end;
                    std::iter_swap(it, end);
                } else {
                    m_originalLayers[originalLayer].push_back(object);
                    ++it;
                }
            }
            m_objects.erase(end, m_objects.end());
        }
                        
        
        bool MoveObjectsToLayerCommand::doPerformDo() {
            setLayer(m_objects, m_layer);
            return true;
        }
        
        bool MoveObjectsToLayerCommand::doPerformUndo() {
            View::MapDocumentSPtr document = lockDocument();
            LayerObjectsMap::const_iterator mapIt, mapEnd;
            for (mapIt = m_originalLayers.begin(), mapEnd = m_originalLayers.end(); mapIt != mapEnd; ++mapIt) {
                Model::Layer* oldLayer = mapIt->first;
                const Model::ObjectList& objects = mapIt->second;
                setLayer(objects, oldLayer);
            }
            
            return true;
        }
        
        void MoveObjectsToLayerCommand::setLayer(const Model::ObjectList& objects, Model::Layer* layer) {
            Model::ObjectList::const_iterator it, end;
            for (it = objects.begin(), end = objects.end(); it != end; ++it) {
                Model::Object* object = *it;
                object->setLayer(layer);
            }
        }

        bool MoveObjectsToLayerCommand::doIsRepeatable(View::MapDocumentSPtr document) const {
            return false;
        }
        
        bool MoveObjectsToLayerCommand::doCollateWith(Command::Ptr command) {
            return false;
        }
    }
}

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

#include "ReparentBrushesCommand.h"

#include "StringUtils.h"
#include "Model/Brush.h"
#include "Model/Entity.h"
#include "Model/ModelUtils.h"
#include "View/MapDocument.h"

#include <cassert>

namespace TrenchBroom {
    namespace Controller {
        const Command::CommandType ReparentBrushesCommand::Type = Command::freeType();

        ReparentBrushesCommand::Ptr ReparentBrushesCommand::reparent(View::MapDocumentWPtr document, const Model::BrushList& brushes, Model::Entity* newParent) {
            return Ptr(new ReparentBrushesCommand(document, brushes, newParent));
        }

        const Model::EntityList& ReparentBrushesCommand::emptyEntities() const {
            return m_emptyEntities;
        }

        ReparentBrushesCommand::ReparentBrushesCommand(View::MapDocumentWPtr document, const Model::BrushList& brushes, Model::Entity* newParent) :
        DocumentCommand(Type, makeName(brushes, newParent), true, document),
        m_brushes(brushes),
        m_newParent(newParent) {
            assert(!brushes.empty());
            assert(newParent != NULL);
        }
        
        String ReparentBrushesCommand::makeName(const Model::BrushList& brushes, Model::Entity* newParent) {
            StringStream name;
            name << "Move " << (brushes.size() == 1 ? "Brush" : "Brushes") << " to " << newParent->classname("<missing classname>");
            return name.str();
        }
        
        bool ReparentBrushesCommand::doPerformDo() {
            m_oldParents.clear();
            m_oldLayers.clear();
            m_emptyEntities.clear();
            
            Model::ObjectList allChangedParents = Model::makeParentList(m_brushes);
            if (!VectorUtils::contains(allChangedParents, m_newParent))
                allChangedParents.push_back(m_newParent);
            
            const Model::ObjectList allChangedBrushes = Model::makeObjectList(m_brushes);
            
            View::MapDocumentSPtr document = lockDocument();
            document->objectsWillChangeNotifier(allChangedParents);
            document->objectsWillBeRemovedNotifier(allChangedBrushes);
            
            Model::BrushList::const_iterator it, end;
            for (it = m_brushes.begin(), end = m_brushes.end(); it != end; ++it) {
                Model::Brush* brush = *it;
                Model::Entity* oldParent = brush->parent();
                m_oldParents[brush] = oldParent;
                m_oldLayers[brush] = brush->layer();

                Model::Layer* newLayer = m_newParent->worldspawn() ? brush->layer() : m_newParent->layer();
                brush->setLayer(NULL);
                oldParent->removeBrush(brush);
                m_newParent->addBrush(brush);
                brush->setLayer(newLayer);
                
                if (oldParent->brushes().empty() && !oldParent->worldspawn())
                    m_emptyEntities.push_back(oldParent);
            }

            document->objectsWereAddedNotifier(allChangedBrushes);
            document->objectsDidChangeNotifier(allChangedParents);
            
            return true;
        }
        
        bool ReparentBrushesCommand::doPerformUndo() {
            Model::ObjectList allChangedParents = Model::makeParentList(m_brushes);
            Model::BrushEntityMap::const_iterator it, end;
            for (it = m_oldParents.begin(), end = m_oldParents.end(); it != end; ++it)
                allChangedParents.push_back(it->second);
            VectorUtils::sortAndRemoveDuplicates(allChangedParents);
            
            const Model::ObjectList allChangedBrushes = Model::makeObjectList(m_brushes);

            View::MapDocumentSPtr document = lockDocument();
            document->objectsWillChangeNotifier(allChangedParents);
            document->objectsWillBeRemovedNotifier(allChangedBrushes);

            for (it = m_oldParents.begin(), end = m_oldParents.end(); it != end; ++it) {
                Model::Brush* brush = it->first;
                Model::Entity* oldParent = it->second;
                Model::Layer* oldLayer = m_oldLayers[brush];

                m_newParent->removeBrush(brush);
                oldParent->addBrush(brush);
                brush->setLayer(oldLayer);
            }

            document->objectsWereAddedNotifier(allChangedBrushes);
            document->objectsDidChangeNotifier(allChangedParents);
            return true;
        }

        bool ReparentBrushesCommand::doIsRepeatable(View::MapDocumentSPtr document) const {
            return document->hasSelectedBrushes() && !document->hasSelectedEntities();
        }
        
        Command* ReparentBrushesCommand::doRepeat(View::MapDocumentSPtr document) const {
            return new ReparentBrushesCommand(document, document->selectedBrushes(), m_newParent);
        }

        bool ReparentBrushesCommand::doCollateWith(Command::Ptr command) {
            return false;
        }
    }
}

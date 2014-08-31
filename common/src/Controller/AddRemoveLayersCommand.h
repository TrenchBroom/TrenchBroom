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

#ifndef __TrenchBroom__AddRemoveLayersCommand__
#define __TrenchBroom__AddRemoveLayersCommand__

#include "SharedPointer.h"
#include "Controller/DocumentCommand.h"
#include "Model/ModelTypes.h"
#include "View/ViewTypes.h"

namespace TrenchBroom {
    namespace Controller {
        class AddRemoveLayersCommand : public DocumentCommand {
        public:
            static const CommandType Type;
            typedef std::tr1::shared_ptr<AddRemoveLayersCommand> Ptr;
        private:
            typedef enum {
                Action_Add,
                Action_Remove
            } Action;
            
            Action m_action;
            Model::LayerList m_layersToAdd;
            Model::LayerList m_layersToRemove;
        public:
            ~AddRemoveLayersCommand();
            
            static AddRemoveLayersCommand::Ptr addLayers(View::MapDocumentWPtr document, const Model::LayerList& layers);
            static AddRemoveLayersCommand::Ptr removeLayers(View::MapDocumentWPtr document, const Model::LayerList& layers);
        private:
            AddRemoveLayersCommand(View::MapDocumentWPtr document, const Action action, const Model::LayerList& layers);
            
            bool doPerformDo();
            bool doPerformUndo();
            
            bool doIsRepeatable(View::MapDocumentSPtr document) const;
            
            bool doCollateWith(Command::Ptr command);
            
            void addLayers(const Model::LayerList& layers);
            void removeLayers(const Model::LayerList& layers);
        };
    }
}

#endif /* defined(__TrenchBroom__AddRemoveLayersCommand__) */

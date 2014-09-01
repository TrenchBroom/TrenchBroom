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

#ifndef __TrenchBroom__MoveObjectsToLayerCommand__
#define __TrenchBroom__MoveObjectsToLayerCommand__

#include "SharedPointer.h"
#include "Controller/DocumentCommand.h"
#include "Model/ModelTypes.h"
#include "View/ViewTypes.h"

#include <map>

namespace TrenchBroom {
    namespace Controller {
        class MoveObjectsToLayerCommand : public DocumentCommand {
        public:
            static const CommandType Type;
            typedef std::tr1::shared_ptr<MoveObjectsToLayerCommand> Ptr;
        private:
            typedef std::map<Model::Layer*, Model::ObjectList> LayerObjectsMap;
            
            Model::Layer* m_layer;
            Model::ObjectList m_objects;
            LayerObjectsMap m_originalLayers;
        public:
            static Ptr moveObjects(View::MapDocumentWPtr document, Model::Layer* layer, const Model::ObjectList& objects);
        private:
            MoveObjectsToLayerCommand(View::MapDocumentWPtr document, Model::Layer* layer, const Model::ObjectList& objects);
            
            bool doPerformDo();
            bool doPerformUndo();
            void setLayer(const Model::ObjectList& objects, Model::Layer* layer);
            
            bool doIsRepeatable(View::MapDocumentSPtr document) const;
            
            bool doCollateWith(Command::Ptr command);
        };
    }
}

#endif /* defined(__TrenchBroom__MoveObjectsToLayerCommand__) */

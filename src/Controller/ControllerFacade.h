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

#ifndef __TrenchBroom__ControllerFacade__
#define __TrenchBroom__ControllerFacade__

#include "TrenchBroom.h"
#include "VecMath.h"
#include "Controller/CommandProcessor.h"
#include "Controller/CommandListener.h"
#include "Model/ModelTypes.h"
#include "View/ViewTypes.h"

namespace TrenchBroom {
    namespace IO {
        class Path;
    }
    
    namespace Controller {
        class CommandProcessor;
        
        class ControllerFacade {
        private:
            View::MapDocumentPtr m_document;
            CommandProcessor m_commandProcessor;
        public:
            void setDocument(View::MapDocumentPtr document);
            
            void addCommandListener(CommandListener::Ptr listener);
            void removeCommandListener(CommandListener::Ptr listener);

            bool newDocument(const BBox3& worldBounds, Model::GamePtr game);
            bool openDocument(const BBox3& worldBounds, Model::GamePtr game, const IO::Path& path);
            
            bool selectObject(Model::ObjectPtr object);
            bool deselectAllAndSelectObject(Model::ObjectPtr object);
            bool deselectObject(Model::ObjectPtr object);
            bool selectFace(Model::BrushFacePtr face);
            bool deselectAllAndSelectFace(Model::BrushFacePtr face);
            bool deselectFace(Model::BrushFacePtr face);
            bool deselectAll();
        };
    }
}

#endif /* defined(__TrenchBroom__ControllerFacade__) */

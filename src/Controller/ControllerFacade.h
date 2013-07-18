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
#include "Model/BrushFace.h"
#include "Model/Game.h"
#include "Model/Object.h"
#include "View/MapDocument.h"

namespace TrenchBroom {
    namespace IO {
        class Path;
    }
    
    namespace Controller {
        class CommandProcessor;
        
        class ControllerFacade {
        private:
            View::MapDocument::Ptr m_document;
            CommandProcessor m_commandProcessor;
        public:
            void setDocument(View::MapDocument::Ptr document);
            
            void addCommandListener(CommandListener::Ptr listener);
            void removeCommandListener(CommandListener::Ptr listener);

            bool newDocument(const BBox3& worldBounds, Model::Game::Ptr game);
            bool openDocument(const BBox3& worldBounds, Model::Game::Ptr game, const IO::Path& path);
            
            bool selectObject(Model::Object::Ptr object);
            bool deselectAllAndSelectObject(Model::Object::Ptr object);
            bool deselectObject(Model::Object::Ptr object);
            bool selectFace(Model::BrushFace::Ptr face);
            bool deselectAllAndSelectFace(Model::BrushFace::Ptr face);
            bool deselectFace(Model::BrushFace::Ptr face);
            bool deselectAll();
        };
    }
}

#endif /* defined(__TrenchBroom__ControllerFacade__) */

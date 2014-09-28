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

#ifndef __TrenchBroom__MapFrame__
#define __TrenchBroom__MapFrame__

#include "Model/MapFormat.h"
#include "Model/ModelTypes.h"
#include "View/ViewTypes.h"

#include <wx/frame.h>

class wxTimer;
class wxTimerEvent;

namespace TrenchBroom {
    class Logger;

    namespace IO {
        class Path;
    }
    
    namespace View {
        class Autosaver;
        class FrameManager;
        
        class MapFrame : public wxFrame {
        private:
            FrameManager* m_frameManager;
            MapDocumentSPtr m_document;
            
            Autosaver* m_autosaver;
            wxTimer* m_autosaveTimer;
        public:
            MapFrame();
            MapFrame(FrameManager* frameManager, MapDocumentSPtr document);
            void Create(FrameManager* frameManager, MapDocumentSPtr document);
            ~MapFrame();
            
            void positionOnScreen(wxFrame* reference);
        public: // getters and such
            Logger* logger() const;
        public: // document management
            bool newDocument(Model::GamePtr game, Model::MapFormat::Type mapFormat);
            bool openDocument(Model::GamePtr game, const IO::Path& path);
        private:
            bool saveDocument();
            bool saveDocumentAs();

            bool confirmOrDiscardChanges();
        private: // title bar contents
            void updateTitle();
        private: // notification handlers
            void bindObservers();
            void unbindObservers();
            
            void documentWasCleared(View::MapDocument* document);
            void documentDidChange(View::MapDocument* document);
        private: // event handlers
            void OnAutosaveTimer(wxTimerEvent& event);
        };
    }
}

#endif /* defined(__TrenchBroom__MapFrame__) */

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
 along with TrenchBroom. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __TrenchBroom__FrameManager__
#define __TrenchBroom__FrameManager__

#include "View/ViewTypes.h"

#include <vector>

#include <wx/event.h>

namespace TrenchBroom {
    namespace IO {
        class Path;
    }

    namespace View {
        class MapFrame;
        
        typedef std::vector<MapFrame*> FrameList;
        
        class FrameManager {
        private:
            bool m_singleFrame;
            FrameList m_frames;
            MapFrame* m_topFrame;
        public:
            FrameManager(const bool singleFrame);
            ~FrameManager();
            
            MapFrame* newFrame();
            bool closeAllFrames();

            FrameList frames() const;
            bool allFramesClosed() const;
            
            void OnFrameActivate(wxActivateEvent& event);
        private:
            MapFrame* createOrReuseFrame();
            MapFrame* createFrame(MapDocumentPtr document);
            bool closeAllFrames(bool force);
            void removeAndDestroyFrame(MapFrame* frame);
            
            friend class MapFrame;
        };
    }
}

#endif /* defined(__TrenchBroom__FrameManager__) */

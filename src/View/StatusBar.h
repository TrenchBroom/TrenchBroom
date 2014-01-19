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

#ifndef __TrenchBroom__StatusBar__
#define __TrenchBroom__StatusBar__

#include "Logger.h"
#include "StringUtils.h"
#include "View/Console.h"
#include "View/ViewTypes.h"

#include <wx/panel.h>

class wxIdleEvent;
class wxStaticText;
class wxWindow;

namespace TrenchBroom {
    namespace View {
        class Console;
        class ImagePanel;
        
        class StatusBar : public wxPanel {
        private:
            MapDocumentWPtr m_document;
            
            size_t m_gridSize;
            ImagePanel* m_gridIconPanel;
            wxStaticText* m_gridSizeText;
            
            bool m_textureLock;
            wxBitmap m_textureLockOn;
            wxBitmap m_textureLockOff;
            ImagePanel* m_textureLockIconPanel;
            wxStaticText* m_textureLockText;

            size_t m_issueCount;
            ImagePanel* m_issuesIconPanel;
            wxStaticText* m_issuesText;
            
            wxStaticText* m_message;
        public:
            StatusBar(wxWindow* parent, MapDocumentWPtr document, Console* console);
            
            void log(Logger::LogLevel level, const String& message);
            void OnIdle(wxIdleEvent& event);
        };
    }
}

#endif /* defined(__TrenchBroom__StatusBar__) */

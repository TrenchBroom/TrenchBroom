/*
 Copyright (C) 2010-2017 Kristian Duske
 
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

#ifndef CompilationRun_h
#define CompilationRun_h

#include "StringUtils.h"
#include "View/ViewTypes.h"

#include <wx/event.h>
#include <wx/string.h>
#include <wx/thread.h>

class wxTextCtrl;

namespace TrenchBroom {
    class VariableTable;
    
    namespace Model {
        class CompilationProfile;
    }
    
    namespace View {
        class CompilationRunner;
        
        class CompilationRun : public wxEvtHandler {
        private:
            CompilationRunner* m_currentRun;
            mutable wxCriticalSection m_currentRunSection;
        public:
            CompilationRun();
            ~CompilationRun();
            
            bool running() const;
            void run(const Model::CompilationProfile* profile, MapDocumentSPtr document, wxTextCtrl* currentOutput);
            void test(const Model::CompilationProfile* profile, MapDocumentSPtr document, wxTextCtrl* currentOutput);
            void terminate();
        private:
            bool doIsRunning() const;
            void run(const Model::CompilationProfile* profile, MapDocumentSPtr document, wxTextCtrl* currentOutput, bool test);
        private:
            String buildWorkDir(const Model::CompilationProfile* profile, MapDocumentSPtr document);

            void OnCompilationStart(wxEvent& event);
            void OnCompilationEnd(wxEvent& event);
            
            void cleanup();
        };
    }
}

#endif /* CompilationRun_h */

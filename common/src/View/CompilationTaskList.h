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

#ifndef CompilationTaskList_h
#define CompilationTaskList_h

#include "View/ControlListBox.h"
#include "View/ViewTypes.h"

class wxWindow;

namespace TrenchBroom {
    namespace Model {
        class CompilationProfile;
        class CompilationTask;
    }
    
    namespace View {
        class CompilationTaskList : public ControlListBox {
        private:
            template <typename T> class TaskEditor;
            class ExportMapTaskEditor;
            class CopyFilesTaskEditor;
            class RunToolTaskEditor;
        private:
            MapDocumentWPtr m_document;
            Model::CompilationProfile* m_profile;
        public:
            CompilationTaskList(wxWindow* parent, MapDocumentWPtr document);
            ~CompilationTaskList() override;
            
            void setProfile(Model::CompilationProfile* profile);
        private:
            void profileDidChange();
            void refresh();
        private:
            class CompilationTaskEditorFactory;
            Item* createItem(wxWindow* parent, const wxSize& margins, size_t index) override;
        };
    }
}

#endif /* CompilationTaskList_h */

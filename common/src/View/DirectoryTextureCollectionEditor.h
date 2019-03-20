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

#ifndef DirectoryTextureCollectionEditor_h
#define DirectoryTextureCollectionEditor_h

#include "IO/Path.h"
#include "View/ViewTypes.h"

#include <wx/panel.h>

class wxListBox;

namespace TrenchBroom {
    namespace View {
        class DirectoryTextureCollectionEditor : public QWidget {
        private:
            MapDocumentWPtr m_document;

            wxListBox* m_availableCollectionsList;
            wxListBox* m_enabledCollectionsList;
        public:
            DirectoryTextureCollectionEditor(QWidget* parent, MapDocumentWPtr document);
			~DirectoryTextureCollectionEditor();
        private:
            void OnAddTextureCollections(wxCommandEvent& event);
            void OnRemoveTextureCollections(wxCommandEvent& event);
            void OnReloadTextureCollections(wxCommandEvent& event);
            void OnUpdateAddTextureCollections(wxUpdateUIEvent& event);
            void OnUpdateRemoveTextureCollections(wxUpdateUIEvent& event);
            void OnUpdateReloadTextureCollections(wxUpdateUIEvent& event);
        private:
            void createGui();

            void bindObservers();
            void unbindObservers();

            void textureCollectionsDidChange();
            void modsDidChange();
            void preferenceDidChange(const IO::Path& path);

            void update();
            void updateAvailableTextureCollections();
            void updateEnabledTextureCollections();
            void updateListBox(wxListBox* box, const IO::Path::List& paths);

            IO::Path::List availableTextureCollections() const;
            IO::Path::List enabledTextureCollections() const;
        };
    }
}

#endif /* DirectoryTextureCollectionEditor_h */

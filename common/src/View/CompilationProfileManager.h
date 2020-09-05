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

#ifndef CompilationProfileManager_h
#define CompilationProfileManager_h

#include <memory>

#include <QWidget>

class QAbstractButton;
class QPoint;

namespace TrenchBroom {
    namespace Model {
        class CompilationConfig;
        class CompilationProfile;
    }

    namespace View {
        class CompilationProfileListBox;
        class CompilationProfileEditor;
        class MapDocument;

        class CompilationProfileManager : public QWidget {
            Q_OBJECT
        private:
            Model::CompilationConfig& m_config;
            CompilationProfileListBox* m_profileList;
            CompilationProfileEditor* m_profileEditor;
            QAbstractButton* m_removeProfileButton;
        public:
            CompilationProfileManager(std::weak_ptr<MapDocument> document, Model::CompilationConfig& config, QWidget* parent = nullptr);

            const Model::CompilationProfile* selectedProfile() const;
        private slots:
            void addProfile();
            void removeProfile();
            void removeProfile(size_t index);
            void removeProfile(Model::CompilationProfile* profile);
            void duplicateProfile(Model::CompilationProfile* profile);
            void profileContextMenuRequested(const QPoint& globalPos, Model::CompilationProfile* profile);
            void profileSelectionChanged();
        signals:
            void selectedProfileChanged();
            void profileChanged();
        };
    }
}

#endif /* CompilationProfileManager_h */

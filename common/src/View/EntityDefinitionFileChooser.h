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

#pragma once

#include <memory>

#include <QListWidget>

class QPushButton;
class QListWidget;
class QLabel;

namespace TrenchBroom {
    namespace View {
        class MapDocument;

        class SingleSelectionListWidget : public QListWidget {
            Q_OBJECT
        private:
            bool m_allowDeselectAll;
        public:
            explicit SingleSelectionListWidget(QWidget* parent = nullptr);
            void setAllowDeselectAll(bool allow);
            bool allowDeselectAll() const;
        protected: // QAbstractItemView overrides
            void selectionChanged(const QItemSelection& selected, const QItemSelection& deselected) override;
            //QItemSelectionModel::SelectionFlags selectionCommand(const QModelIndex& index, const QEvent* event) const override;
        };

        class EntityDefinitionFileChooser : public QWidget {
            Q_OBJECT
        private:
            std::weak_ptr<MapDocument> m_document;

            SingleSelectionListWidget* m_builtin;
            QLabel* m_external;
            QPushButton* m_chooseExternal;
            QPushButton* m_reloadExternal;
        public:
            explicit EntityDefinitionFileChooser(std::weak_ptr<MapDocument> document, QWidget* parent = nullptr);
            ~EntityDefinitionFileChooser() override;
        private:
            void createGui();
            void bindEvents();

            void bindObservers();
            void unbindObservers();

            void documentWasNewed(MapDocument* document);
            void documentWasLoaded(MapDocument* document);
            void entityDefinitionsDidChange();

            void updateControls();

            void builtinSelectionChanged();
            void chooseExternalClicked();
            void reloadExternalClicked();
        };
    }
}

#endif /* defined(TrenchBroom_EntityDefinitionFileChooser) */

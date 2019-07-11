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

#ifndef TrenchBroom_EntityDefinitionFileChooser
#define TrenchBroom_EntityDefinitionFileChooser

#include "View/ViewTypes.h"

#include <QWidget>
#include <QListWidget>

class QPushButton;
class QListWidget;
class QLabel;

namespace TrenchBroom {
    namespace View {
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
            MapDocumentWPtr m_document;

            SingleSelectionListWidget* m_builtin;
            QLabel* m_external;
            QPushButton* m_chooseExternal;
            QPushButton* m_reloadExternal;
        public:
            EntityDefinitionFileChooser(MapDocumentWPtr document, QWidget* parent = nullptr);
            ~EntityDefinitionFileChooser() override;

            void OnBuiltinSelectionChanged();
            void OnChooseExternalClicked();
            void OnReloadExternalClicked();
        private:
            void createGui();
            void bindEvents();

            void bindObservers();
            void unbindObservers();

            void documentWasNewed(MapDocument* document);
            void documentWasLoaded(MapDocument* document);
            void entityDefinitionsDidChange();

            void updateControls();
        };
    }
}

#endif /* defined(TrenchBroom_EntityDefinitionFileChooser) */

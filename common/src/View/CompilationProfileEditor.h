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

#ifndef CompilationProfileEditor_h
#define CompilationProfileEditor_h

#include <QWidget>

#include <memory>

class QAbstractButton;
class QLineEdit;
class QStackedWidget;

namespace TrenchBroom {
    namespace Model {
        class CompilationProfile;
    }

    namespace View {
        class CompilationTaskListBox;
        class MapDocument;
        class MultiCompletionLineEdit;

        /**
         * Editor UI for a single compilation profile
         */
        class CompilationProfileEditor : public QWidget {
            Q_OBJECT
        private:
            std::weak_ptr<MapDocument> m_document;
            Model::CompilationProfile* m_profile;
            QStackedWidget* m_stackedWidget;
            QLineEdit* m_nameTxt;
            MultiCompletionLineEdit* m_workDirTxt;
            CompilationTaskListBox* m_taskList;
            QAbstractButton* m_addTaskButton;
            QAbstractButton* m_removeTaskButton;
            QAbstractButton* m_moveTaskUpButton;
            QAbstractButton* m_moveTaskDownButton;
        public:
            explicit CompilationProfileEditor(std::weak_ptr<MapDocument> document, QWidget* parent = nullptr);
            ~CompilationProfileEditor() override;
        private:
            QWidget* createEditorPage(QWidget* parent);

        private slots:
            void nameChanged(const QString& text);
            void workDirChanged(const QString& text);

            void addTask();
            void removeTask();
            void moveTaskUp();
            void moveTaskDown();

            void taskSelectionChanged();
        public:
            void setProfile(Model::CompilationProfile* profile);
        private:
            void profileWillBeRemoved();
            void profileDidChange();
            void refresh();
        signals:
            void profileChanged();
        };
    }
}

#endif /* CompilationProfileEditor_h */

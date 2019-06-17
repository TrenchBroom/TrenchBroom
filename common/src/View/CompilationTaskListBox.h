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

class QCompleter;
class QLineEdit;
class QWidget;

namespace TrenchBroom {
    namespace Model {
        class CompilationProfile;
        class CompilationTask;
        class CompilationExportMap;
        class CompilationCopyFiles;
        class CompilationRunTool;
    }

    namespace View {
        class TitledPanel;

        class CompilationTaskEditorBase : public ControlListBoxItemRenderer {
            Q_OBJECT
        protected:
            const QString m_title;
            MapDocumentWPtr m_document;
            Model::CompilationProfile* m_profile;
            Model::CompilationTask* m_task;
            TitledPanel* m_panel;

            using Completers = std::list<QCompleter*>;
            Completers m_completers;
        protected:
            CompilationTaskEditorBase(const QString& title, MapDocumentWPtr document, Model::CompilationProfile& profile, Model::CompilationTask& task, QWidget* parent);
        public:
            ~CompilationTaskEditorBase() override;
        protected:
            void setupCompleter(QLineEdit* lineEdit);
        private:
            void updateCompleter(QCompleter* completer);
        private:
            void addProfileObservers();
            void removeProfileObservers();
            void addTaskObservers();
            void removeTaskObservers();

            void profileWillBeRemoved();
            void profileDidChange();

            void taskWillBeRemoved();
            void taskDidChange();
        private:
            void update(size_t index) override;
        private:
            virtual void updateTask() = 0;
        };

        class CompilationExportMapTaskEditor : public CompilationTaskEditorBase {
            Q_OBJECT
        private:
            QLineEdit* m_targetEditor;
        public:
            CompilationExportMapTaskEditor(MapDocumentWPtr document, Model::CompilationProfile& profile, Model::CompilationExportMap& task, QWidget* parent = nullptr);
        private:
            void updateTask() override;
            Model::CompilationExportMap& task();
        private slots:
            void targetSpecChanged(const QString& text);
        };

        class CompilationCopyFilesTaskEditor : public CompilationTaskEditorBase {
            Q_OBJECT
        private:
            QLineEdit* m_sourceEditor;
            QLineEdit* m_targetEditor;
        public:
            CompilationCopyFilesTaskEditor(MapDocumentWPtr document, Model::CompilationProfile& profile, Model::CompilationCopyFiles& task, QWidget* parent = nullptr);
        private:
            void updateTask() override;
            Model::CompilationCopyFiles& task();
        private slots:
            void sourceSpecChanged(const QString& text);
            void targetSpecChanged(const QString& text);
        };

        class CompilationRunToolTaskEditor : public CompilationTaskEditorBase {
            Q_OBJECT
        private:
            QLineEdit* m_toolEditor;
            QLineEdit* m_parametersEditor;
        public:
            CompilationRunToolTaskEditor(MapDocumentWPtr document, Model::CompilationProfile& profile, Model::CompilationRunTool& task, QWidget* parent = nullptr);
        private:
            void updateTask() override;
            Model::CompilationRunTool& task();
        private slots:
            void browseTool();
            void toolSpecChanged(const QString& text);
            void parameterSpecChanged(const QString& text);
        };

        class CompilationTaskListBox : public ControlListBox {
            Q_OBJECT
        private:
            MapDocumentWPtr m_document;
            Model::CompilationProfile* m_profile;
        public:
            explicit CompilationTaskListBox(MapDocumentWPtr document, QWidget* parent = nullptr);
            ~CompilationTaskListBox() override;

            void setProfile(Model::CompilationProfile* profile);
        private:
            void profileDidChange();
        private:
            class CompilationTaskEditorFactory;
            size_t itemCount() const override;
            ControlListBoxItemRenderer* createItemRenderer(QWidget* parent, size_t index) override;
        };
    }
}

#endif /* CompilationTaskList_h */

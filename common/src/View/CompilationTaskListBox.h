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

#include <memory>
#include <vector>

class QCompleter;
class QLineEdit;
class QWidget;

namespace TrenchBroom {
    namespace Model {
        class CompilationCopyFiles;
        class CompilationExportMap;
        class CompilationProfile;
        class CompilationRunTool;
        class CompilationTask;
    }

    namespace View {
        class MapDocument;
        class MultiCompletionLineEdit;
        class TitledPanel;

        class CompilationTaskEditorBase : public ControlListBoxItemRenderer {
            Q_OBJECT
        protected:
            const QString m_title;
            std::weak_ptr<MapDocument> m_document;
            Model::CompilationProfile* m_profile;
            Model::CompilationTask* m_task;
            TitledPanel* m_panel;

            using Completers = std::vector<QCompleter*>;
            Completers m_completers;
        protected:
            CompilationTaskEditorBase(const QString& title, std::weak_ptr<MapDocument> document, Model::CompilationProfile& profile, Model::CompilationTask& task, QWidget* parent);
        protected:
            void setupCompleter(MultiCompletionLineEdit* lineEdit);
        private:
            void updateCompleter(QCompleter* completer);
        };

        class CompilationExportMapTaskEditor : public CompilationTaskEditorBase {
            Q_OBJECT
        private:
            MultiCompletionLineEdit* m_targetEditor;
        public:
            CompilationExportMapTaskEditor(std::weak_ptr<MapDocument> document, Model::CompilationProfile& profile, Model::CompilationExportMap& task, QWidget* parent = nullptr);
        private:
            void updateItem() override;
            Model::CompilationExportMap& task();
        private slots:
            void targetSpecChanged(const QString& text);
        };

        class CompilationCopyFilesTaskEditor : public CompilationTaskEditorBase {
            Q_OBJECT
        private:
            MultiCompletionLineEdit* m_sourceEditor;
            MultiCompletionLineEdit* m_targetEditor;
        public:
            CompilationCopyFilesTaskEditor(std::weak_ptr<MapDocument> document, Model::CompilationProfile& profile, Model::CompilationCopyFiles& task, QWidget* parent = nullptr);
        private:
            void updateItem() override;
            Model::CompilationCopyFiles& task();
        private slots:
            void sourceSpecChanged(const QString& text);
            void targetSpecChanged(const QString& text);
        };

        class CompilationRunToolTaskEditor : public CompilationTaskEditorBase {
            Q_OBJECT
        private:
            MultiCompletionLineEdit* m_toolEditor;
            MultiCompletionLineEdit* m_parametersEditor;
        public:
            CompilationRunToolTaskEditor(std::weak_ptr<MapDocument> document, Model::CompilationProfile& profile, Model::CompilationRunTool& task, QWidget* parent = nullptr);
        private:
            void updateItem() override;
            Model::CompilationRunTool& task();
        private slots:
            void browseTool();
            void toolSpecChanged(const QString& text);
            void parameterSpecChanged(const QString& text);
        };

        class CompilationTaskListBox : public ControlListBox {
            Q_OBJECT
        private:
            std::weak_ptr<MapDocument> m_document;
            Model::CompilationProfile* m_profile;
        public:
            explicit CompilationTaskListBox(std::weak_ptr<MapDocument> document, QWidget* parent = nullptr);

            void setProfile(Model::CompilationProfile* profile);
        public:
            void reloadTasks();
        private:
            class CompilationTaskEditorFactory;
            size_t itemCount() const override;
            ControlListBoxItemRenderer* createItemRenderer(QWidget* parent, size_t index) override;
        signals:
            void taskContextMenuRequested(const QPoint& globalPos, Model::CompilationTask* task);
        };
    }
}

#endif /* CompilationTaskList_h */

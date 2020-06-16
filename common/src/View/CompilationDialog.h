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

#ifndef CompilationDialog_h
#define CompilationDialog_h

#include "View/CompilationRun.h"

#include <QDialog>

class QLabel;
class QPushButton;
class QTextEdit;

namespace TrenchBroom {
    namespace View {
        class CompilationProfileManager;
        class MapFrame;

        class CompilationDialog : public QDialog {
            Q_OBJECT
        private:
            MapFrame* m_mapFrame;
            CompilationProfileManager* m_profileManager;
            QPushButton* m_launchButton;
            QPushButton* m_compileButton;
            QPushButton* m_testCompileButton;
            QPushButton* m_stopCompileButton;
            QPushButton* m_closeButton;
            QLabel* m_currentRunLabel;
            QTextEdit* m_output;
            CompilationRun m_run;
        public:
            explicit CompilationDialog(MapFrame* mapFrame);
        private:
            void createGui();

            void keyPressEvent(QKeyEvent* event) override;

            void updateCompileButtons();
            void startCompilation(bool test);
            void stopCompilation();
            void closeEvent(QCloseEvent* event) override;
        private slots:
            void compilationStarted();
            void compilationEnded();

            void selectedProfileChanged();
        };
    }
}

#endif /* CompilationDialog_h */

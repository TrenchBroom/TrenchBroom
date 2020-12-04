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

#include <QObject>

#include <memory>
#include <string>

class QTextEdit;

namespace TrenchBroom {
    class VariableTable;

    namespace Model {
        class CompilationProfile;
    }

    namespace View {
        class CompilationRunner;
        class MapDocument;

        class CompilationRun : public QObject {
            Q_OBJECT
        private:
            CompilationRunner* m_currentRun;
        public:
            CompilationRun();
            ~CompilationRun() override;

            bool running() const;
            void run(const Model::CompilationProfile* profile, std::shared_ptr<MapDocument> document, QTextEdit* currentOutput);
            void test(const Model::CompilationProfile* profile, std::shared_ptr<MapDocument> document, QTextEdit* currentOutput);
            void terminate();
        private:
            bool doIsRunning() const;
            void run(const Model::CompilationProfile* profile, std::shared_ptr<MapDocument> document, QTextEdit* currentOutput, bool test);
        private:
            std::string buildWorkDir(const Model::CompilationProfile* profile, std::shared_ptr<MapDocument> document);
            void cleanup();
        signals:
            void compilationStarted();
            void compilationEnded();
        };
    }
}



/*
Copyright (C) 2020 Kristian Duske

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

#include <catch2/catch.hpp>

#include "GTestCompat.h"

#include "View/MapDocumentTest.h"

#include "EL/VariableStore.h"
#include "Model/CompilationTask.h"
#include "View/CompilationContext.h"
#include "View/CompilationRunner.h"
#include "View/TextOutputAdapter.h"

#include <QEventLoop>
#include <QObject>
#include <QTextEdit>
#include <QTimer>

namespace TrenchBroom {
    namespace View {
        class CompilationTaskRunnerTest : public MapDocumentTest {};
        
        class ExecuteTask {
        private:
            CompilationTaskRunner& m_runner;
        public:
            bool started = false;
            bool errored = false;
            bool ended = false;
            
            ExecuteTask(CompilationTaskRunner& runner)
            : m_runner(runner) {
                QObject::connect(&m_runner, &CompilationTaskRunner::start, [&]() { started = true; });
                QObject::connect(&m_runner, &CompilationTaskRunner::error, [&]() { errored = true; });
                QObject::connect(&m_runner, &CompilationTaskRunner::end, [&]()   { ended = true; });
            }
            
            void executeAndWait(const int timeout) {
                QTimer timer;
                timer.setSingleShot(true);
                
                QEventLoop loop;
                QObject::connect(&m_runner, &CompilationTaskRunner::end, &loop, &QEventLoop::quit);
                QObject::connect(&m_runner, &CompilationTaskRunner::error, &loop, &QEventLoop::quit);
                QObject::connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
                
                m_runner.execute();
                timer.start(timeout);
                loop.exec();
            }
        };
        
        TEST_CASE_METHOD(CompilationTaskRunnerTest, "CompilationTaskRunnerTest.runMissingTool") {
            EL::NullVariableStore variables;
            QTextEdit output;
            TextOutputAdapter outputAdapter(&output);
            
            CompilationContext context(document, variables, outputAdapter, false);
            
            Model::CompilationRunTool task(true, "", "");
            CompilationRunToolTaskRunner runner(context, task);
            
            ExecuteTask exec(runner);
            exec.executeAndWait(500);
            
            ASSERT_TRUE(exec.started);
            ASSERT_TRUE(exec.errored);
            ASSERT_FALSE(exec.ended);
        }
    }
}

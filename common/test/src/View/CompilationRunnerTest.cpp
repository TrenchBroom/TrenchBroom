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

#include "MapDocumentTest.h"
#include "EL/VariableStore.h"
#include "Model/CompilationTask.h"
#include "View/CompilationContext.h"
#include "View/CompilationRunner.h"
#include "View/TextOutputAdapter.h"

#include <QObject>
#include <QTextEdit>

#include <condition_variable>
#include <chrono>
#include <mutex>
#include <thread>

#include "Catch2.h"

namespace TrenchBroom {
    namespace View {
        class ExecuteTask {
        private:
            CompilationTaskRunner& m_runner;
            std::mutex m_mutex;
            std::condition_variable m_condition;
        public:
            bool started{false};
            bool errored{false};
            bool ended{false};
            
            ExecuteTask(CompilationTaskRunner& runner)
            : m_runner{runner} {
                QObject::connect(&m_runner, &CompilationTaskRunner::start, [&]() { started = true; auto lock = std::unique_lock<std::mutex>{m_mutex}; m_condition.notify_all(); });
                QObject::connect(&m_runner, &CompilationTaskRunner::error, [&]() { errored = true; auto lock = std::unique_lock<std::mutex>{m_mutex}; m_condition.notify_all(); });
                QObject::connect(&m_runner, &CompilationTaskRunner::end, [&]()   { ended = true;   auto lock = std::unique_lock<std::mutex>{m_mutex}; m_condition.notify_all(); });
            }
            
            void executeAndWait(const int timeout) {
                m_runner.execute();
                
                auto lock = std::unique_lock<std::mutex>{m_mutex};
                m_condition.wait_for(lock, std::chrono::milliseconds{timeout}, [&]() { return errored || ended; });
            }
        };
        
        TEST_CASE_METHOD(MapDocumentTest, "CompilationRunToolTaskRunner.runMissingTool") {
            auto variables = EL::NullVariableStore{};
            auto output = QTextEdit{};
            auto outputAdapter = TextOutputAdapter{&output};
            
            auto context = CompilationContext{document, variables, outputAdapter, false};
            
            auto task = Model::CompilationRunTool{true, "", ""};
            auto runner = CompilationRunToolTaskRunner{context, task};
            
            auto exec = ExecuteTask{runner};
            exec.executeAndWait(500);
            
            CHECK(exec.started);
            CHECK(exec.errored);
            CHECK_FALSE(exec.ended);
        }
    }
}

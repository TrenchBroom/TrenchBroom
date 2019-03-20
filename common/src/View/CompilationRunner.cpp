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

#include "CompilationRunner.h"

#include "CollectionUtils.h"
#include "Exceptions.h"
#include "IO/DiskIO.h"
#include "IO/Path.h"
#include "Model/CompilationProfile.h"
#include "Model/CompilationTask.h"
#include "View/CompilationContext.h"
#include "View/CompilationVariables.h"
#include "View/MapDocument.h"

#include <wx/process.h>
#include <wx/sstream.h>
#include <wx/timer.h>

wxDECLARE_EVENT(wxEVT_TASK_START, wxNotifyEvent);
wxDECLARE_EVENT(wxEVT_TASK_ERROR, wxNotifyEvent);
wxDECLARE_EVENT(wxEVT_TASK_END, wxNotifyEvent);

wxDEFINE_EVENT(wxEVT_TASK_START, wxNotifyEvent);
wxDEFINE_EVENT(wxEVT_TASK_ERROR, wxNotifyEvent);
wxDEFINE_EVENT(wxEVT_TASK_END, wxNotifyEvent);

wxDEFINE_EVENT(wxEVT_COMPILATION_START, wxNotifyEvent);
wxDEFINE_EVENT(wxEVT_COMPILATION_END, wxNotifyEvent);

namespace TrenchBroom {
    namespace View {
        class CompilationRunner::TaskRunner : public wxEvtHandler {
        protected:
            CompilationContext& m_context;
        protected:
            TaskRunner(CompilationContext& context) :
            m_context(context) {}
        public:
            ~TaskRunner() override {}

            void execute() {
                doExecute();
            }

            void terminate() {
                doTerminate();
            }
        protected:
            void notifyStart() {
                QueueEvent(new wxNotifyEvent(wxEVT_TASK_START));
            }

            void notifyError() {
                QueueEvent(new wxNotifyEvent(wxEVT_TASK_ERROR));
            }

            void notifyEnd() {
                QueueEvent(new wxNotifyEvent(wxEVT_TASK_END));
            }

            String interpolate(const String& spec) {
                try {
                    return m_context.interpolate(spec);
                } catch (const Exception& e) {
                    m_context << "#### Could not interpolate expression '" << spec << "': " << e.what() << "\n";
                    throw;
                }
            }
        private:
            virtual void doExecute() = 0;
            virtual void doTerminate() = 0;
        private:
            TaskRunner(const TaskRunner& other);
            TaskRunner& operator=(const TaskRunner& other);
        };

        class CompilationRunner::ExportMapRunner : public TaskRunner {
        private:
            const Model::CompilationExportMap* m_task;
        public:
            ExportMapRunner(CompilationContext& context, const Model::CompilationExportMap* task) :
            TaskRunner(context),
            m_task(static_cast<const Model::CompilationExportMap*>(task->clone())) {}

            ~ExportMapRunner() override {
                delete m_task;
            }
        private:
            void doExecute() override {
                notifyStart();

                try {
                    const IO::Path targetPath(interpolate(m_task->targetSpec()));
                    try {
                        m_context << "#### Exporting map file '" << targetPath.asString() << "'\n";

                        if (!m_context.test()) {
                            const IO::Path directoryPath = targetPath.deleteLastComponent();
                            if (!IO::Disk::directoryExists(directoryPath))
                                IO::Disk::createDirectory(directoryPath);

                            const MapDocumentSPtr document = m_context.document();
                            document->saveDocumentTo(targetPath);
                        }
                        notifyEnd();
                    } catch (const Exception& e) {
                        m_context << "#### Could not export map file '" << targetPath.asString() << "': " << e.what() << "\n";
                        throw;
                    }
                } catch (const Exception&) {
                    notifyError();
                }

            }

            void doTerminate() override {}
        private:
            ExportMapRunner(const ExportMapRunner& other);
            ExportMapRunner& operator=(const ExportMapRunner& other);
        };

        class CompilationRunner::CopyFilesRunner : public TaskRunner {
        private:
            const Model::CompilationCopyFiles* m_task;
        public:
            CopyFilesRunner(CompilationContext& context, const Model::CompilationCopyFiles* task) :
            TaskRunner(context),
            m_task(static_cast<const Model::CompilationCopyFiles*>(task->clone())) {}

            ~CopyFilesRunner() override {
                delete m_task;
            }
        private:
            void doExecute() override {
                notifyStart();

                try {
                    const IO::Path sourcePath(interpolate(m_task->sourceSpec()));
                    const IO::Path targetPath(interpolate(m_task->targetSpec()));

                    const IO::Path sourceDirPath = sourcePath.deleteLastComponent();
                    const String sourcePattern = sourcePath.lastComponent().asString();

                    try {
                        m_context << "#### Copying '" << sourcePath.asString() << "' to '" << targetPath.asString() << "'\n";
                        if (!m_context.test())
                            IO::Disk::copyFiles(sourceDirPath, IO::FileNameMatcher(sourcePattern), targetPath, true);
                        notifyEnd();
                    } catch (const Exception& e) {
                        m_context << "#### Could not copy '" << sourcePath.asString() << "' to '" << targetPath.asString() << "': " << e.what() << "\n";
                        throw;
                    }
                } catch (const Exception&) {
                    notifyError();
                }
            }

            void doTerminate() override {}
        private:
            CopyFilesRunner(const CopyFilesRunner& other);
            CopyFilesRunner& operator=(const CopyFilesRunner& other);
        };

        class CompilationRunner::RunToolRunner : public TaskRunner {
        private:
            const Model::CompilationRunTool* m_task;
            wxProcess* m_process;
            wxCriticalSection m_processSection;
            wxTimer* m_timer;
            bool m_terminated;
        public:
            RunToolRunner(CompilationContext& context, const Model::CompilationRunTool* task) :
            TaskRunner(context),
            m_task(static_cast<const Model::CompilationRunTool*>(task->clone())),
            m_process(nullptr),
            m_timer(nullptr),
            m_terminated(false) {}

            ~RunToolRunner() override {
                assert(m_process == nullptr);
                assert(m_timer == nullptr);
                delete m_task;
            }
        private:
            void doExecute() override {
                wxCriticalSectionLocker lockProcess(m_processSection);
                start();
            }

            void doTerminate() override {
                wxCriticalSectionLocker lockProcess(m_processSection);
                if (m_process != nullptr) {
                    readRemainingOutput();
                    m_process->Unbind(wxEVT_END_PROCESS, &RunToolRunner::OnEndProcessAsync, this);
                    m_process->Detach();
                    const int pid = static_cast<int>(m_process->GetPid());
                    end();
                    ::wxKill(pid, wxSIGKILL);
                    m_context << "\n\n#### Terminated\n";
                }
            }
        private:
            void OnTimer(wxTimerEvent& event) {
                wxCriticalSectionLocker lockProcess(m_processSection);
                if (m_process != nullptr)
                    readOutput();
            }

            void OnEndProcessAsync(wxProcessEvent& event) {
                QueueEvent(event.Clone());
            }

            void OnEndProcessSync(wxProcessEvent& event) {
                wxCriticalSectionLocker lockProcess(m_processSection);
                if (m_process != nullptr) {
                    readRemainingOutput();
                    m_context << "#### Finished with exit status " << event.GetExitCode() << "\n\n";
                    end();
                    delete m_process;
                }
            }
        private:
            void start() {
                assert(m_process == nullptr);
                assert(m_timer == nullptr);

                try {
                    const IO::Path toolPath(interpolate(m_task->toolSpec()));
                    const String parameters(interpolate(m_task->parameterSpec()));
                    const String cmd = toolPath.asString() + " " + parameters;

                    m_context << "#### Executing '" << cmd << "'\n";

                    if (!m_context.test()) {
                        m_process = new wxProcess(this);
                        m_process->Redirect();
                        m_process->Bind(wxEVT_END_PROCESS, &RunToolRunner::OnEndProcessAsync, this);
                        Bind(wxEVT_END_PROCESS, &RunToolRunner::OnEndProcessSync, this);

                        m_timer = new wxTimer();
                        m_timer->Bind(wxEVT_TIMER, &RunToolRunner::OnTimer, this);

                        wxExecuteEnv* env = new wxExecuteEnv();
                        env->cwd = m_context.variableValue(CompilationVariableNames::WORK_DIR_PATH);;

                        ::wxExecute(cmd, wxEXEC_ASYNC, m_process, env);
                        m_timer->Start(50);
                    } else {
                        notifyEnd();
                    }
                } catch (const Exception&) {
                    notifyError();
                }
            }

            void end() {
                ensure(m_process != nullptr, "process is null");
                ensure(m_timer != nullptr, "timer is null");

                delete m_timer;
                m_timer = nullptr;
                m_process = nullptr; // process will be deleted by the library

                notifyEnd();
            }
        private:
            void readRemainingOutput() {
                bool hasOutput = true;
                bool hasTrailingNewline = true;
                while (hasOutput) {
                    hasOutput = false;
                    if (m_process->IsInputAvailable()) {
                        const QString output = readStream(m_process->GetInputStream());
                        if (!output.IsEmpty()) {
                            m_context << output;
                            hasOutput = true;
                            hasTrailingNewline = output.Last() == '\n';
                        }
                    }
                    if (m_process->IsErrorAvailable()) {
                        const QString output = readStream(m_process->GetErrorStream());
                        if (!output.IsEmpty()) {
                            m_context << output;
                            hasOutput = true;
                            hasTrailingNewline = output.Last() == '\n';
                        }
                    }
                }

                if (!hasTrailingNewline)
                    m_context << '\n';
            }

            bool readOutput() {
                bool hasOutput = false;
                if (m_process->IsInputAvailable()) {
                    const QString output = readStream(m_process->GetInputStream());
                    if (!output.IsEmpty()) {
                        m_context << output;
                        hasOutput = true;
                    }
                }
                if (m_process->IsErrorAvailable()) {
                    const QString output = readStream(m_process->GetErrorStream());
                    if (!output.IsEmpty()) {
                        m_context << output;
                        hasOutput = true;
                    }
                }
                return hasOutput;
            }

            QString readStream(wxInputStream* stream) {
                ensure(stream != nullptr, "stream is null");
                wxStringOutputStream out;
                if (stream->CanRead()) {
                    static const size_t BUF_SIZE = 8192;
                    char buffer[BUF_SIZE];
                    stream->Read(buffer, BUF_SIZE);
                    out.Write(buffer, stream->LastRead());
                }
                const QString result = out.GetString();
                return result;
            }
        private:
            RunToolRunner(const RunToolRunner& other);
            RunToolRunner& operator=(const RunToolRunner& other);
        };

        CompilationRunner::CompilationRunner(CompilationContext* context, const Model::CompilationProfile* profile) :
        m_context(context),
        m_taskRunners(createTaskRunners(*m_context, profile)),
        m_currentTask(std::end(m_taskRunners)) {}

        CompilationRunner::~CompilationRunner() {
            ListUtils::clearAndDelete(m_taskRunners);
            delete m_context;
        }

        class CompilationRunner::CreateTaskRunnerVisitor : public Model::ConstCompilationTaskVisitor {
        private:
            CompilationContext& m_context;
            TaskRunnerList m_runners;
        public:
            CreateTaskRunnerVisitor(CompilationContext& context) :
            m_context(context) {}

            const TaskRunnerList& runners() {
                return m_runners;
            }

            void visit(const Model::CompilationExportMap* task) override {
                appendRunner(new ExportMapRunner(m_context, task));
            }

            void visit(const Model::CompilationCopyFiles* task) override {
                appendRunner(new CopyFilesRunner(m_context, task));
            }

            void visit(const Model::CompilationRunTool* task) override {
                appendRunner(new RunToolRunner(m_context, task));
            }

        private:
            void appendRunner(TaskRunner* runner) {
                m_runners.push_back(runner);
            }
        };

        CompilationRunner::TaskRunnerList CompilationRunner::createTaskRunners(CompilationContext& context, const Model::CompilationProfile* profile) {
            CreateTaskRunnerVisitor visitor(context);
            profile->accept(visitor);
            return visitor.runners();
        }

        void CompilationRunner::execute() {
            assert(!running());
            m_currentTask = std::begin(m_taskRunners);
            bindEvents(*m_currentTask);
            (*m_currentTask)->execute();

            wxNotifyEvent event(wxEVT_COMPILATION_START);
            ProcessEvent(event);
        }

        void CompilationRunner::terminate() {
            assert(running());
            unbindEvents(*m_currentTask);
            (*m_currentTask)->terminate();
            m_currentTask = std::end(m_taskRunners);

            wxNotifyEvent event(wxEVT_COMPILATION_END);
            ProcessEvent(event);
        }

        bool CompilationRunner::running() const {
            return m_currentTask != std::end(m_taskRunners);
        }

        void CompilationRunner::OnTaskError(wxEvent& event) {
            if (running()) {
                unbindEvents(*m_currentTask);
                m_currentTask = std::end(m_taskRunners);
                wxNotifyEvent endEvent(wxEVT_COMPILATION_END);
                ProcessEvent(endEvent);
            }
        }

        void CompilationRunner::OnTaskEnd(wxEvent& event) {
            if (running()) {
                unbindEvents(*m_currentTask);
                ++m_currentTask;
                if (m_currentTask != std::end(m_taskRunners)) {
                    bindEvents(*m_currentTask);
                    (*m_currentTask)->execute();
                } else {
                    wxNotifyEvent endEvent(wxEVT_COMPILATION_END);
                    ProcessEvent(endEvent);
                }
            }
        }

        void CompilationRunner::bindEvents(TaskRunner* runner) {
            runner->Bind(wxEVT_TASK_ERROR, &CompilationRunner::OnTaskError, this);
            runner->Bind(wxEVT_TASK_END, &CompilationRunner::OnTaskEnd, this);
        }

        void CompilationRunner::unbindEvents(TaskRunner* runner) {
            runner->Unbind(wxEVT_TASK_ERROR, &CompilationRunner::OnTaskError, this);
            runner->Unbind(wxEVT_TASK_END, &CompilationRunner::OnTaskEnd, this);
        }
    }
}

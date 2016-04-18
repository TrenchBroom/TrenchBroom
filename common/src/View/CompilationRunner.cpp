/*
 Copyright (C) 2010-2016 Kristian Duske
 
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
wxDECLARE_EVENT(wxEVT_TASK_END, wxNotifyEvent);

wxDEFINE_EVENT(wxEVT_TASK_START, wxNotifyEvent);
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
            virtual ~TaskRunner() {}
            
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
            
            void notifyEnd() {
                QueueEvent(new wxNotifyEvent(wxEVT_TASK_END));
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
            
            ~ExportMapRunner() {
                delete m_task;
            }
        private:
            void doExecute() {
                notifyStart();
                
                const IO::Path targetPath(m_context.translateVariables(m_task->targetSpec()));
                try {
                    m_context << "#### Exporting map file '" << targetPath.asString() << "'\n";
                    
                    const IO::Path directoryPath = targetPath.deleteLastComponent();
                    if (!IO::Disk::directoryExists(directoryPath))
                        IO::Disk::createDirectory(directoryPath);
                    
                    const MapDocumentSPtr document = m_context.document();
                    document->saveDocumentTo(targetPath);
                } catch (const Exception& e) {
                    m_context << "#### Could export map file '" << targetPath.asString() << "': " << e.what() << "\n";
                }
                
                notifyEnd();
            }
            
            void doTerminate() {}
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
            
            ~CopyFilesRunner() {
                delete m_task;
            }
        private:
            void doExecute() {
                notifyStart();
                
                const IO::Path sourcePath(m_context.translateVariables(m_task->sourceSpec()));
                const IO::Path targetPath(m_context.translateVariables(m_task->targetSpec()));
                
                const IO::Path sourceDirPath = sourcePath.deleteLastComponent();
                const String sourcePattern = sourcePath.lastComponent().asString();
                
                try {
                    m_context << "#### Copying '" << sourcePath.asString() << "'\nTo '" << targetPath.asString() << "'\n";
                    IO::Disk::copyFiles(sourceDirPath, IO::FileNameMatcher(sourcePattern), targetPath, true);
                } catch (const Exception& e) {
                    m_context << "#### Could not copy '" << sourcePath.asString() << "' to '" << targetPath.asString() << "': " << e.what() << "\n";
                }
                
                notifyEnd();
            }
            
            void doTerminate() {}
        private:
            CopyFilesRunner(const CopyFilesRunner& other);
            CopyFilesRunner& operator=(const CopyFilesRunner& other);
        };
        
        class CompilationRunner::RunToolRunner : public TaskRunner {
        private:
            const Model::CompilationRunTool* m_task;
            wxProcess* m_process;
            wxTimer* m_timer;
            bool m_terminated;
        public:
            RunToolRunner(CompilationContext& context, const Model::CompilationRunTool* task) :
            TaskRunner(context),
            m_task(static_cast<const Model::CompilationRunTool*>(task->clone())),
            m_process(NULL),
            m_timer(NULL),
            m_terminated(false) {}
            
            ~RunToolRunner() {
                assert(m_process == NULL);
                assert(m_timer == NULL);
                delete m_task;
            }
        private:
            void doExecute() {
                start();
            }
            
            void doTerminate() {
                if (m_process != NULL) {
                    readRemainingOutput();
                    m_process->Unbind(wxEVT_END_PROCESS, &RunToolRunner::OnEndProcessAsync, this);
                    m_process->Detach();
                    const int pid = static_cast<int>(m_process->GetPid());
                    end();
                    ::wxKill(pid);
                    m_context << "#### Terminated\n";
                }
            }
        private:
            void OnTimer(wxTimerEvent& event) {
                readOutput();
            }
            
            void OnEndProcessAsync(wxProcessEvent& event) {
                QueueEvent(event.Clone());
            }
            
            void OnEndProcessSync(wxProcessEvent& event) {
                readRemainingOutput();
                end();
                delete m_process;
                m_context << "#### Finished with exit status " << event.GetPid() << "\n";
            }
        private:
            void start() {
                assert(m_process == NULL);
                assert(m_timer == NULL);
                
                const IO::Path toolPath(m_context.translateVariables(m_task->toolSpec()));
                const String parameters(m_context.translateVariables(m_task->parameterSpec()));
                const String cmd = toolPath.asString() + " " + parameters;

                m_process = new wxProcess(this);
                m_process->Redirect();
                m_process->Bind(wxEVT_END_PROCESS, &RunToolRunner::OnEndProcessAsync, this);
                Bind(wxEVT_END_PROCESS, &RunToolRunner::OnEndProcessSync, this);
                
                m_timer = new wxTimer();
                m_timer->Bind(wxEVT_TIMER, &RunToolRunner::OnTimer, this);
                
                wxExecuteEnv* env = new wxExecuteEnv();
                env->cwd = m_context.variableValue(CompilationVariableNames::WORK_DIR_PATH);;
                
                m_context << "#### Executing '" << cmd << "'\n";
                ::wxExecute(cmd, wxEXEC_ASYNC, m_process, env);
                m_timer->Start(50);
            }
            
            void end() {
                assert(m_process != NULL);
                assert(m_timer != NULL);

                delete m_timer;
                m_timer = NULL;
                m_process = NULL; // process will be deleted by the library
                
                notifyEnd();
            }
        private:
            void readRemainingOutput() {
                bool hasOutput = true;
                bool hasTrailingNewline = true;
                while (hasOutput) {
                    hasOutput = false;
                    if (m_process->IsInputAvailable()) {
                        const wxString output = readStream(m_process->GetInputStream());
                        if (!output.IsEmpty()) {
                            m_context << output;
                            hasOutput = true;
                            hasTrailingNewline = output.Last() == '\n';
                        }
                    }
                    if (m_process->IsErrorAvailable()) {
                        const wxString output = readStream(m_process->GetErrorStream());
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
                    const wxString output = readStream(m_process->GetInputStream());
                    if (!output.IsEmpty()) {
                        m_context << output;
                        hasOutput = true;
                    }
                }
                if (m_process->IsErrorAvailable()) {
                    const wxString output = readStream(m_process->GetErrorStream());
                    if (!output.IsEmpty()) {
                        m_context << output;
                        hasOutput = true;
                    }
                }
                return hasOutput;
            }
            
            wxString readStream(wxInputStream* stream) {
                assert(stream != NULL);
                wxStringOutputStream out;
                if (stream->CanRead()) {
                    static const size_t BUF_SIZE = 8192;
                    char buffer[BUF_SIZE];
                    stream->Read(buffer, BUF_SIZE);
                    out.Write(buffer, stream->LastRead());
                }
                const wxString result = out.GetString();
                return result;
            }
        private:
            RunToolRunner(const RunToolRunner& other);
            RunToolRunner& operator=(const RunToolRunner& other);
        };

        CompilationRunner::CompilationRunner(CompilationContext* context, const Model::CompilationProfile* profile) :
        m_context(context),
        m_taskRunners(createTaskRunners(*m_context, profile)),
        m_currentTask(m_taskRunners.end()) {}
        
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
            
            void visit(const Model::CompilationExportMap* task) {
                appendRunner(new ExportMapRunner(m_context, task));
            }
            
            void visit(const Model::CompilationCopyFiles* task) {
                appendRunner(new CopyFilesRunner(m_context, task));
            }
            
            void visit(const Model::CompilationRunTool* task) {
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
            m_currentTask = m_taskRunners.begin();
            (*m_currentTask)->Bind(wxEVT_TASK_END, &CompilationRunner::OnTaskEnded, this);
            (*m_currentTask)->execute();
        }
        
        void CompilationRunner::terminate() {
            assert(running());
            (*m_currentTask)->Unbind(wxEVT_TASK_END, &CompilationRunner::OnTaskEnded, this);
            (*m_currentTask)->terminate();
            m_currentTask = m_taskRunners.end();
        }
        
        bool CompilationRunner::running() const {
            return m_currentTask != m_taskRunners.end();
        }
        
        void CompilationRunner::OnTaskEnded(wxEvent& event) {
            if (running()) {
                (*m_currentTask)->Unbind(wxEVT_TASK_END, &CompilationRunner::OnTaskEnded, this);
                ++m_currentTask;
                if (m_currentTask != m_taskRunners.end()) {
                    (*m_currentTask)->Bind(wxEVT_TASK_END, &CompilationRunner::OnTaskEnded, this);
                    (*m_currentTask)->execute();
                }
            }
        }
    }
}

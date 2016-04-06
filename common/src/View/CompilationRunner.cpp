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

#include "Exceptions.h"
#include "IO/DiskIO.h"
#include "IO/Path.h"
#include "Model/CompilationContext.h"
#include "Model/CompilationProfile.h"
#include "Model/CompilationTask.h"

#include <wx/event.h>
#include <wx/process.h>
#include <wx/sstream.h>
#include <wx/thread.h>
#include <wx/timer.h>

namespace TrenchBroom {
    namespace View {
        class CompilationRunner::TaskRunner {
        protected:
            Model::CompilationContext& m_context;
        private:
            TaskRunner* m_next;
        public:
            TaskRunner(Model::CompilationContext& context) :
            m_context(context),
            m_next(NULL) {}
            
            virtual ~TaskRunner() {
                delete m_next;
            }
            
            void setNext(TaskRunner* next) {
                m_next = next;
            }
            
            void execute() {
                doExecute();
            }
            
            void terminate() {
                doTerminate();
                if (m_next != NULL)
                    m_next->terminate();
            }
        protected:
            void executeNext() {
                if (m_next != NULL)
                    m_next->execute();
            }
        private:
            virtual void doExecute() = 0;
            virtual void doTerminate() = 0;
        private:
            TaskRunner(const TaskRunner& other);
            TaskRunner& operator=(const TaskRunner& other);
        };

        class CompilationRunner::CopyFilesRunner : public TaskRunner {
        private:
            IO::Path m_sourcePath;
            IO::Path m_targetPath;
        public:
            CopyFilesRunner(Model::CompilationContext& context, const Model::CompilationCopyFiles& task) :
            TaskRunner(context),
            m_sourcePath(m_context.translateVariables(task.sourceSpec())),
            m_targetPath(m_context.translateVariables(task.targetSpec())) {}
        private:
            void doExecute() {
                const IO::Path sourceDirPath = m_sourcePath.deleteLastComponent();
                const String sourcePattern = m_sourcePath.lastComponent().asString();
                
                try {
                    StringStream str;
                    str << "Copying '" << m_sourcePath.asString() << "' to '" << m_targetPath.asString() << "'\n";
                    m_context.appendOutput(str.str());
                    
                    IO::Disk::copyFiles(sourceDirPath, IO::FileNameMatcher(sourcePattern), m_targetPath, true);
                    executeNext();
                } catch (const Exception& e) {
                    StringStream str;
                    str << "Could not copy '" << m_sourcePath.asString() << "' to '" << m_targetPath.asString() << "': " << e.what() << "\n";
                    m_context.appendOutput(str.str());
                }
            }
            
            void doTerminate() {}
        private:
            CopyFilesRunner(const CopyFilesRunner& other);
            CopyFilesRunner& operator=(const CopyFilesRunner& other);
        };

        class CompilationRunner::RunToolRunner : public wxEvtHandler, public TaskRunner {
        private:
            IO::Path m_toolPath;
            String m_parameters;
            wxProcess* m_process;
            wxCriticalSection m_processSection;
            wxTimer* m_processTimer;
        public:
            RunToolRunner(Model::CompilationContext& context, const Model::CompilationRunTool& task) :
            TaskRunner(context),
            m_toolPath(m_context.translateVariables(task.toolSpec())),
            m_parameters(m_context.translateVariables(task.parameterSpec())),
            m_process(NULL),
            m_processTimer(NULL) {}
            
            ~RunToolRunner() {
                terminate();
            }
        private:
            void doExecute() {
                wxCriticalSectionLocker lockProcess(m_processSection);
                
                const String cmd = m_toolPath.asString() + " " + m_parameters;
                createProcess();
                startProcess(cmd);
            }
            
            void doTerminate() {
                wxCriticalSectionLocker lockProcess(m_processSection);
                if (m_process != NULL) {
                    wxProcess::Kill(static_cast<int>(m_process->GetPid()));
                    deleteProcess();
                }
            }
        private:
            void OnTerminateProcess(wxProcessEvent& event) {
                wxCriticalSectionLocker lockProcess(m_processSection);
                if (m_process != NULL) {
                    assert(m_process->GetPid() == event.GetPid());
                    
                    StringStream str;
                    str << "Finished with exit status " << event.GetExitCode() << "\n";
                    m_context.appendOutput(str.str());
                    
                    deleteProcess();
                    executeNext();
                }
            }
            
            void OnProcessTimer(wxTimerEvent& event) {
                wxCriticalSectionLocker lockProcess(m_processSection);
                
                if (m_process != NULL) {
                    if (m_process->IsInputAvailable())
                        m_context.appendOutput(readStream(m_process->GetInputStream()));
                }
            }
            
            String readStream(wxInputStream* stream) {
                assert(stream != NULL);
                wxStringOutputStream out;
                stream->Read(out);
                return out.GetString().ToStdString();
            }
            
            void createProcess() {
                assert(m_process == NULL);
                m_process = new wxProcess(this);
                m_processTimer = new wxTimer(this);
                
                m_process->Bind(wxEVT_END_PROCESS, &RunToolRunner::OnTerminateProcess, this);
                m_processTimer->Bind(wxEVT_TIMER, &RunToolRunner::OnProcessTimer, this);
            }
            
            void startProcess(const String& cmd) {
                assert(m_process != NULL);
                assert(m_processTimer != NULL);
                
                m_context.appendOutput("Executing " + cmd + "\n");
                wxExecute(cmd, wxEXEC_ASYNC, m_process);
                m_processTimer->Start(20);
            }
            
            void deleteProcess() {
                if (m_processTimer != NULL) {
                    delete m_processTimer;
                    m_processTimer = NULL;
                }
                delete m_process;
                m_process = NULL;
            }
        private:
            RunToolRunner(const RunToolRunner& other);
            RunToolRunner& operator=(const RunToolRunner& other);
        };
    
        CompilationRunner::CompilationRunner(Model::CompilationContext& context, const Model::CompilationProfile& profile) :
        m_runnerChain(createRunnerChain(context, profile)) {}
        
        CompilationRunner::~CompilationRunner() {
            if (m_runnerChain != NULL) {
                m_runnerChain->terminate();
                delete m_runnerChain;
            }
        }

        class CompilationRunner::CreateTaskRunnerVisitor : public Model::ConstCompilationTaskVisitor {
        private:
            Model::CompilationContext& m_context;
            TaskRunner* m_runnerChain;
        public:
            CreateTaskRunnerVisitor(Model::CompilationContext& context) :
            m_context(context),
            m_runnerChain(NULL) {}
            
            TaskRunner* runnerChain() {
                return m_runnerChain;
            }
            
            void visit(const Model::CompilationCopyFiles& task) {
                appendRunner(new CopyFilesRunner(m_context, task));
            }
            
            void visit(const Model::CompilationRunTool& task) {
                appendRunner(new RunToolRunner(m_context, task));
            }
            
        private:
            void appendRunner(TaskRunner* runner) {
                if (m_runnerChain == NULL)
                    m_runnerChain = runner;
                else
                    m_runnerChain->setNext(runner);
            }
        };

        CompilationRunner::TaskRunner* CompilationRunner::createRunnerChain(Model::CompilationContext& context, const Model::CompilationProfile& profile) {
            CreateTaskRunnerVisitor visitor(context);
            profile.accept(visitor);
            return visitor.runnerChain();
        }

        void CompilationRunner::execute() {
            if (m_runnerChain != NULL)
                m_runnerChain->execute();
        }
        
        void CompilationRunner::terminate() {
            if (m_runnerChain != NULL)
                m_runnerChain->terminate();
        }
    }
}

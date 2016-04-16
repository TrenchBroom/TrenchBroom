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
#include "Model/CompilationProfile.h"
#include "Model/CompilationTask.h"
#include "View/CompilationContext.h"
#include "View/CompilationVariables.h"
#include "View/MapDocument.h"

#include <wx/event.h>
#include <wx/filefn.h>
#include <wx/process.h>
#include <wx/sstream.h>
#include <wx/thread.h>
#include <wx/timer.h>

namespace TrenchBroom {
    namespace View {
        class CompilationRunner::TaskRunner {
        protected:
            CompilationContext& m_context;
        private:
            TaskRunner* m_next;
        public:
            TaskRunner(CompilationContext& context) :
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
        
        class CompilationRunner::ExportMapRunner : public TaskRunner {
        private:
            String m_targetSpec;
        public:
            ExportMapRunner(CompilationContext& context, const Model::CompilationExportMap* task) :
            TaskRunner(context),
            m_targetSpec(task->targetSpec()) {}
        private:
            void doExecute() {
                const IO::Path targetPath(m_context.translateVariables(m_targetSpec));
                try {
                    m_context << "Exporting map file '" << targetPath.asString() << "'\n";

                    const IO::Path directoryPath = targetPath.deleteLastComponent();
                    if (!IO::Disk::directoryExists(directoryPath))
                        IO::Disk::createDirectory(directoryPath);
                    
                    const MapDocumentSPtr document = m_context.document();
                    document->saveDocumentTo(targetPath);
                    
                    executeNext();
                } catch (const Exception& e) {
                    m_context << "Could export map file '" << targetPath.asString() << "': " << e.what() << "\n";
                }
            }
            
            void doTerminate() {}
        private:
            ExportMapRunner(const CopyFilesRunner& other);
            ExportMapRunner& operator=(const CopyFilesRunner& other);
        };

        class CompilationRunner::CopyFilesRunner : public TaskRunner {
        private:
            String m_sourceSpec;
            String m_targetSpec;
        public:
            CopyFilesRunner(CompilationContext& context, const Model::CompilationCopyFiles* task) :
            TaskRunner(context),
            m_sourceSpec(task->sourceSpec()),
            m_targetSpec(task->targetSpec()) {}
        private:
            void doExecute() {
                const IO::Path sourcePath(m_context.translateVariables(m_sourceSpec));
                const IO::Path targetPath(m_context.translateVariables(m_targetSpec));
                
                const IO::Path sourceDirPath = sourcePath.deleteLastComponent();
                const String sourcePattern = sourcePath.lastComponent().asString();
                
                try {
                    m_context << "Copying '" << sourcePath.asString() << "'\nTo '" << targetPath.asString() << "'\n";
                    
                    IO::Disk::copyFiles(sourceDirPath, IO::FileNameMatcher(sourcePattern), targetPath, true);
                    executeNext();
                } catch (const Exception& e) {
                    m_context << "Could not copy '" << sourcePath.asString() << "' to '" << targetPath.asString() << "': " << e.what() << "\n";
                }
            }
            
            void doTerminate() {}
        private:
            CopyFilesRunner(const CopyFilesRunner& other);
            CopyFilesRunner& operator=(const CopyFilesRunner& other);
        };

        class CompilationRunner::RunToolRunner : public wxEvtHandler, public TaskRunner {
        private:
            String m_toolSpec;
            String m_parameterSpec;
            wxProcess* m_process;
            wxCriticalSection m_processSection;
            wxTimer* m_processTimer;
        public:
            RunToolRunner(CompilationContext& context, const Model::CompilationRunTool* task) :
            TaskRunner(context),
            m_toolSpec(task->toolSpec()),
            m_parameterSpec(task->parameterSpec()),
            m_process(NULL),
            m_processTimer(NULL) {}
            
            ~RunToolRunner() {
                terminate();
            }
        private:
            void doExecute() {
                wxCriticalSectionLocker lockProcess(m_processSection);
                
                const IO::Path toolPath(m_context.translateVariables(m_toolSpec));
                const String parameters(m_context.translateVariables(m_parameterSpec));
                const String cmd = toolPath.asString() + " " + parameters;
                
                createProcess();
                startProcess(cmd);
            }
            
            void doTerminate() {
                wxCriticalSectionLocker lockProcess(m_processSection);
                if (m_process != NULL) {
                    while (readOutput());
                    wxProcess::Kill(static_cast<int>(m_process->GetPid()));
                    deleteProcess();
                }
            }
        private:
            void OnTerminateProcess(wxProcessEvent& event) {
                wxCriticalSectionLocker lockProcess(m_processSection);
                if (m_process != NULL) {
                    assert(m_process->GetPid() == event.GetPid());
                    
                    while (readOutput());
                    
                    m_context << "Finished with exit status " << event.GetExitCode() << "\n";
                    
                    deleteProcess();
                    executeNext();
                }
            }
            
            void OnProcessTimer(wxTimerEvent& event) {
                wxCriticalSectionLocker lockProcess(m_processSection);
                if (m_process != NULL)
                    readOutput();
            }
            
            bool readOutput() {
                bool hasOutput = false;
                if (m_process->IsInputAvailable()) {
                    m_context << readStream(m_process->GetInputStream());
                    hasOutput = true;
                }
                if (m_process->IsErrorAvailable()) {
                    m_context << readStream(m_process->GetErrorStream());
                    hasOutput = true;
                }
                return hasOutput;
            }
            
            wxString readStream(wxInputStream* stream) {
                assert(stream != NULL);
                wxStringOutputStream out;
                if (stream->CanRead()) {
                    static const size_t BUF_SIZE = 1024;
                    char buffer[BUF_SIZE];
                    stream->Read(buffer, BUF_SIZE);
                    out.Write(buffer, stream->LastRead());
                }
                const wxString result = out.GetString();
                return result;
            }
            
            void createProcess() {
                assert(m_process == NULL);
                m_process = new wxProcess(this);
                m_processTimer = new wxTimer();
                
                m_process->Redirect();
                m_process->Bind(wxEVT_END_PROCESS, &RunToolRunner::OnTerminateProcess, this);
                m_processTimer->Bind(wxEVT_TIMER, &RunToolRunner::OnProcessTimer, this);
            }
            
            void startProcess(const String& cmd) {
                assert(m_process != NULL);
                assert(m_processTimer != NULL);

                wxExecuteEnv* env = new wxExecuteEnv();
                env->cwd = m_context.variableValue(CompilationVariableNames::WORK_DIR_PATH);;
                
                m_context << "Executing '" << cmd << "'\n";
                m_processTimer->Start(10);
                
                // At least on OSX, setting the working directory using env is broken. Instead of using the
                // directory provided in env->cwd, it inherits the one from the parent process, so we
                // temporarily change our working directory and change it back immediately after.
                // const wxString oldCwd = ::wxGetCwd();
                // ::wxSetWorkingDirectory(env->cwd);
                ::wxExecute(cmd, wxEXEC_ASYNC, m_process, env);
                // ::wxSetWorkingDirectory(oldCwd);
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
    
        CompilationRunner::CompilationRunner(CompilationContext* context, const Model::CompilationProfile* profile) :
        m_context(context),
        m_runnerChain(createRunnerChain(*m_context, profile)) {}
        
        CompilationRunner::~CompilationRunner() {
            if (m_runnerChain != NULL) {
                m_runnerChain->terminate();
                delete m_runnerChain;
            }
            if (m_context != NULL)
                delete m_context;
        }

        class CompilationRunner::CreateTaskRunnerVisitor : public Model::ConstCompilationTaskVisitor {
        private:
            CompilationContext& m_context;
            TaskRunner* m_runnerChain;
        public:
            CreateTaskRunnerVisitor(CompilationContext& context) :
            m_context(context),
            m_runnerChain(NULL) {}
            
            TaskRunner* runnerChain() {
                return m_runnerChain;
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
                if (m_runnerChain == NULL)
                    m_runnerChain = runner;
                else
                    m_runnerChain->setNext(runner);
            }
        };

        CompilationRunner::TaskRunner* CompilationRunner::createRunnerChain(CompilationContext& context, const Model::CompilationProfile* profile) {
            CreateTaskRunnerVisitor visitor(context);
            profile->accept(visitor);
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

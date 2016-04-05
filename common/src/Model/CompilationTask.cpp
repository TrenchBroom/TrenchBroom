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

#include "CompilationTask.h"

#include "Exceptions.h"
#include "IO/DiskIO.h"
#include "Model/CompilationContext.h"

#include <wx/sstream.h>
#include <wx/timer.h>

namespace TrenchBroom {
    namespace Model {
        CompilationTask::TaskRunner::TaskRunner(CompilationContext& context, TaskRunner* next) :
        m_context(context),
        m_next(next) {}
        
        CompilationTask::TaskRunner::~TaskRunner() {
            if (m_next != NULL)
                delete m_next;
        }

        void CompilationTask::TaskRunner::execute() {
            doExecute();
        }

        void CompilationTask::TaskRunner::terminate() {
            doTerminate();
            if (m_next != NULL)
                m_next->terminate();
        }

        void CompilationTask::TaskRunner::executeNext() {
            if (m_next != NULL)
                m_next->execute();
        }

        CompilationTask::CompilationTask(const Type type) :
        m_type(type) {}
    
        CompilationTask::~CompilationTask() {}

        CompilationTask::Type CompilationTask::type() const {
            return m_type;
        }

        CompilationTask* CompilationTask::clone() const {
            return doClone();
        }

        CompilationTask::TaskRunner* CompilationTask::createTaskRunner(CompilationContext& context, TaskRunner* next) const {
            return doCreateTaskRunner(context, next);
        }
    
        CompilationCopyFiles::Runner::Runner(CompilationContext& context, TaskRunner* next, const String& sourceSpec, const String& targetSpec) :
        CompilationTask::TaskRunner(context, next),
        m_sourcePath(m_context.translateVariables(sourceSpec)),
        m_targetPath(m_context.translateVariables(targetSpec)) {}

        void CompilationCopyFiles::Runner::doExecute() {
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
        
        void CompilationCopyFiles::Runner::doTerminate() {}

        CompilationCopyFiles::CompilationCopyFiles(const String& sourceSpec, const String& targetSpec) :
        CompilationTask(Type_Copy),
        m_sourceSpec(sourceSpec),
        m_targetSpec(targetSpec) {}
        
        const String& CompilationCopyFiles::sourceSpec() const {
            return m_sourceSpec;
        }
        
        const String& CompilationCopyFiles::targetSpec() const {
            return m_targetSpec;
        }

        CompilationTask* CompilationCopyFiles::doClone() const {
            return new CompilationCopyFiles(m_sourceSpec, m_targetSpec);
        }

        CompilationTask::TaskRunner* CompilationCopyFiles::doCreateTaskRunner(CompilationContext& context, TaskRunner* next) const {
            return new Runner(context, next, m_sourceSpec, m_targetSpec);
        }

        CompilationRunTool::Runner::Runner(CompilationContext& context, TaskRunner* next, const String& toolSpec, const String& parameterSpec) :
        TaskRunner(context, next),
        m_toolPath(m_context.translateVariables(toolSpec)),
        m_parameters(m_context.translateVariables(parameterSpec)),
        m_process(NULL),
        m_processTimer(NULL) {}

        CompilationRunTool::Runner::~Runner() {
            terminate();
        }
        
        void CompilationRunTool::Runner::doExecute() {
            wxCriticalSectionLocker lockProcess(m_processSection);

            const String cmd = m_toolPath.asString() + " " + m_parameters;
            createProcess();
            startProcess(cmd);
        }
        
        void CompilationRunTool::Runner::doTerminate() {
            wxCriticalSectionLocker lockProcess(m_processSection);
            if (m_process != NULL) {
                wxProcess::Kill(static_cast<int>(m_process->GetPid()));
                deleteProcess();
            }
        }

        void CompilationRunTool::Runner::OnTerminateProcess(wxProcessEvent& event) {
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

        void CompilationRunTool::Runner::OnProcessTimer(wxTimerEvent& event) {
            wxCriticalSectionLocker lockProcess(m_processSection);
            
            if (m_process != NULL) {
                if (m_process->IsInputAvailable())
                    m_context.appendOutput(readStream(m_process->GetInputStream()));
            }
        }

        String CompilationRunTool::Runner::readStream(wxInputStream* stream) {
            assert(stream != NULL);
            wxStringOutputStream out;
            stream->Read(out);
            return out.GetString().ToStdString();
        }

        void CompilationRunTool::Runner::createProcess() {
            assert(m_process == NULL);
            m_process = new wxProcess(this);
            m_processTimer = new wxTimer(this);
            
            m_process->Bind(wxEVT_END_PROCESS, &CompilationRunTool::Runner::OnTerminateProcess, this);
            m_processTimer->Bind(wxEVT_TIMER, &CompilationRunTool::Runner::OnProcessTimer, this);
        }
        
        void CompilationRunTool::Runner::startProcess(const String& cmd) {
            assert(m_process != NULL);
            assert(m_processTimer != NULL);

            m_context.appendOutput("Executing " + cmd + "\n");
            wxExecute(cmd, wxEXEC_ASYNC, m_process);
            m_processTimer->Start(20);
        }

        void CompilationRunTool::Runner::deleteProcess() {
            if (m_processTimer != NULL) {
                delete m_processTimer;
                m_processTimer = NULL;
            }
            delete m_process;
            m_process = NULL;
        }

        CompilationRunTool::CompilationRunTool(const String& toolSpec, const String& parameterSpec) :
        CompilationTask(Type_Tool),
        m_toolSpec(toolSpec),
        m_parameterSpec(parameterSpec) {}

        const String& CompilationRunTool::toolSpec() const {
            return m_toolSpec;
        }
        
        const String& CompilationRunTool::parameterSpec() const {
            return m_parameterSpec;
        }

        CompilationTask* CompilationRunTool::doClone() const {
            return new CompilationRunTool(m_toolSpec, m_parameterSpec);
        }

        CompilationTask::TaskRunner* CompilationRunTool::doCreateTaskRunner(CompilationContext& context, TaskRunner* next) const {
            return new Runner(context, next, m_toolSpec, m_parameterSpec);
        }
    }
}

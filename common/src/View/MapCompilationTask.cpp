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

#include "MapCompilationTask.h"

#include "Exceptions.h"
#include "IO/DiskIO.h"
#include "View/MapCompilationContext.h"

#include <wx/sstream.h>
#include <wx/timer.h>

namespace TrenchBroom {
    namespace View {
        MapCompilationTask::TaskRunner::TaskRunner(MapCompilationContext& context, TaskRunner* next) :
        m_context(context),
        m_next(next) {}
        
        MapCompilationTask::TaskRunner::~TaskRunner() {
            if (m_next != NULL)
                delete m_next;
        }

        void MapCompilationTask::TaskRunner::execute() {
            doExecute();
        }

        void MapCompilationTask::TaskRunner::terminate() {
            doTerminate();
            if (m_next != NULL)
                m_next->terminate();
        }

        void MapCompilationTask::TaskRunner::executeNext() {
            if (m_next != NULL)
                m_next->execute();
        }

        MapCompilationTask::MapCompilationTask() {}
    
        MapCompilationTask::~MapCompilationTask() {}

        MapCompilationTask::TaskRunner* MapCompilationTask::createTaskRunner(MapCompilationContext& context, TaskRunner* next) const {
            return doCreateTaskRunner(context, next);
        }
    
        MapCompilationCopyFiles::Runner::Runner(MapCompilationContext& context, TaskRunner* next, const String& sourceSpec, const String& targetSpec) :
        MapCompilationTask::TaskRunner(context, next),
        m_sourcePath(m_context.translateVariables(sourceSpec)),
        m_targetPath(m_context.translateVariables(targetSpec)) {}

        void MapCompilationCopyFiles::Runner::doExecute() {
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
        
        void MapCompilationCopyFiles::Runner::doTerminate() {}

        MapCompilationCopyFiles::MapCompilationCopyFiles(const String& sourceSpec, const String& targetSpec) :
        m_sourceSpec(sourceSpec),
        m_targetSpec(targetSpec) {}
        
        MapCompilationTask::TaskRunner* MapCompilationCopyFiles::doCreateTaskRunner(MapCompilationContext& context, TaskRunner* next) const {
            return new Runner(context, next, m_sourceSpec, m_targetSpec);
        }

        MapCompilationRunTool::Runner::Runner(MapCompilationContext& context, TaskRunner* next, const String& toolSpec, const String& parameterSpec) :
        TaskRunner(context, next),
        m_toolPath(m_context.translateVariables(toolSpec)),
        m_parameters(m_context.translateVariables(parameterSpec)),
        m_process(NULL),
        m_processTimer(NULL) {}

        MapCompilationRunTool::Runner::~Runner() {
            terminate();
        }
        
        void MapCompilationRunTool::Runner::doExecute() {
            wxCriticalSectionLocker lockProcess(m_processSection);

            const String cmd = m_toolPath.asString() + " " + m_parameters;
            createProcess();
            startProcess(cmd);
        }
        
        void MapCompilationRunTool::Runner::doTerminate() {
            wxCriticalSectionLocker lockProcess(m_processSection);
            if (m_process != NULL) {
                wxProcess::Kill(static_cast<int>(m_process->GetPid()));
                deleteProcess();
            }
        }

        void MapCompilationRunTool::Runner::OnTerminateProcess(wxProcessEvent& event) {
            wxCriticalSectionLocker lockProcess(m_processSection);
            if (m_process != NULL) {
                assert(m_process->GetPid() == event.GetPid());
                if (event.GetExitCode() == 0) {
                    executeNext();
                } else {
                    StringStream str;
                    str << "Finished with exit status " << event.GetExitCode() << ", aborting\n";
                    m_context.appendOutput(str.str());
                }
                deleteProcess();
            }
        }

        void MapCompilationRunTool::Runner::OnProcessTimer(wxTimerEvent& event) {
            wxCriticalSectionLocker lockProcess(m_processSection);
            
            if (m_process != NULL) {
                if (m_process->IsInputAvailable())
                    m_context.appendOutput(readStream(m_process->GetInputStream()));
                if (m_process->IsErrorAvailable())
                    m_context.appendOutput(readStream(m_process->GetErrorStream()));
            }
        }

        String MapCompilationRunTool::Runner::readStream(wxInputStream* stream) {
            assert(stream != NULL);
            wxStringOutputStream out;
            stream->Read(out);
            return out.GetString().ToStdString();
        }

        void MapCompilationRunTool::Runner::createProcess() {
            assert(m_process == NULL);
            m_process = new wxProcess(this);
            m_processTimer = new wxTimer(this);
            
            m_process->Bind(wxEVT_END_PROCESS, &MapCompilationRunTool::Runner::OnTerminateProcess, this);
            m_processTimer->Bind(wxEVT_TIMER, &MapCompilationRunTool::Runner::OnProcessTimer, this);
        }
        
        void MapCompilationRunTool::Runner::startProcess(const String& cmd) {
            assert(m_process != NULL);
            assert(m_processTimer != NULL);

            m_context.appendOutput("Executing " + cmd + "\n");
            wxExecute(cmd, wxEXEC_ASYNC, m_process);
            m_processTimer->Start(20);
        }

        void MapCompilationRunTool::Runner::deleteProcess() {
            if (m_processTimer != NULL) {
                delete m_processTimer;
                m_processTimer = NULL;
            }
            delete m_process;
            m_process = NULL;
        }

        MapCompilationRunTool::MapCompilationRunTool(const String& toolSpec, const String& parameterSpec) :
        m_toolSpec(toolSpec),
        m_parameterSpec(parameterSpec) {}

        MapCompilationTask::TaskRunner* MapCompilationRunTool::doCreateTaskRunner(MapCompilationContext& context, TaskRunner* next) const {
            return new Runner(context, next, m_toolSpec, m_parameterSpec);
        }
    }
}

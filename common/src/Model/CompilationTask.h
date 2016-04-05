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

#ifndef CompilationTask_h
#define CompilationTask_h

#include "StringUtils.h"
#include "IO/Path.h"

#include <wx/event.h>
#include <wx/process.h>
#include <wx/thread.h>

#include <vector>

class wxTimer;
class wxTimerEvent;

namespace TrenchBroom {
    namespace Model {
        class CompilationContext;
        
        class CompilationTask {
        public:
            typedef std::vector<CompilationTask*> List;
            
            typedef enum {
                Type_Copy,
                Type_Tool
            } Type;
            
            class TaskRunner {
            protected:
                CompilationContext& m_context;
            private:
                TaskRunner* m_next;
            public:
                TaskRunner(CompilationContext& context, TaskRunner* next);
                virtual ~TaskRunner();
                
                void execute();
                void terminate();
            protected:
                void executeNext();
            private:
                virtual void doExecute() = 0;
                virtual void doTerminate() = 0;
            private:
                TaskRunner(const TaskRunner& other);
                TaskRunner& operator=(const TaskRunner& other);
            };
        private:
            Type m_type;
        protected:
            CompilationTask(Type type);
        public:
            virtual ~CompilationTask();

            Type type() const;
            
            CompilationTask* clone() const;
            
            TaskRunner* createTaskRunner(CompilationContext& context, TaskRunner* next = NULL) const;
        private:
            virtual CompilationTask* doClone() const = 0;
            virtual TaskRunner* doCreateTaskRunner(CompilationContext& context, TaskRunner* next) const = 0;
        private:
            CompilationTask(const CompilationTask& other);
            CompilationTask& operator=(const CompilationTask& other);
        };
        
        class CompilationCopyFiles : public CompilationTask {
        private:
            class Runner : public TaskRunner {
            private:
                IO::Path m_sourcePath;
                IO::Path m_targetPath;
            public:
                Runner(CompilationContext& context, TaskRunner* next, const String& sourceSpec, const String& targetSpec);
            private:
                void doExecute();
                void doTerminate();
            private:
                Runner(const Runner& other);
                Runner& operator=(const Runner& other);
            };
            
            String m_sourceSpec;
            String m_targetSpec;
        public:
            CompilationCopyFiles(const String& sourceSpec, const String& targetSpec);
            
            const String& sourceSpec() const;
            const String& targetSpec() const;
        private:
            CompilationTask* doClone() const;
            TaskRunner* doCreateTaskRunner(CompilationContext& context, TaskRunner* next) const;
        private:
            CompilationCopyFiles(const CompilationCopyFiles& other);
            CompilationCopyFiles& operator=(const CompilationCopyFiles& other);
        };

        class CompilationRunTool : public CompilationTask {
        private:
            class Runner : public wxEvtHandler, public TaskRunner {
            private:
                IO::Path m_toolPath;
                String m_parameters;
                wxProcess* m_process;
                wxCriticalSection m_processSection;
                wxTimer* m_processTimer;
            public:
                Runner(CompilationContext& context, TaskRunner* next, const String& toolSpec, const String& parameterSpec);
                ~Runner();
            private:
                void doExecute();
                void doTerminate();
            private:
                void OnTerminateProcess(wxProcessEvent& event);
                void OnProcessTimer(wxTimerEvent& event);
                String readStream(wxInputStream* stream);
                
                void createProcess();
                void startProcess(const String& cmd);
                void deleteProcess();
            private:
                Runner(const Runner& other);
                Runner& operator=(const Runner& other);
            };
            
            String m_toolSpec;
            String m_parameterSpec;
        public:
            CompilationRunTool(const String& toolSpec, const String& parameterSpec);
            
            const String& toolSpec() const;
            const String& parameterSpec() const;
        private:
            CompilationTask* doClone() const;
            TaskRunner* doCreateTaskRunner(CompilationContext& context, TaskRunner* next) const;
        private:
            CompilationRunTool(const CompilationRunTool& other);
            CompilationRunTool& operator=(const CompilationRunTool& other);
        };
    }
}

#endif /* CompilationTask_h */

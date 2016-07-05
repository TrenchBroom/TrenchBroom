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

#ifndef CompilationRunner_h
#define CompilationRunner_h

#include <wx/event.h>

#include <list>

wxDECLARE_EVENT(wxEVT_COMPILATION_START, wxNotifyEvent);
wxDECLARE_EVENT(wxEVT_COMPILATION_END, wxNotifyEvent);

namespace TrenchBroom {
    namespace Model {
        class CompilationProfile;
    }
    
    namespace View {
        class CompilationContext;
        
        class CompilationRunner : public wxEvtHandler {
        private:
            class TaskRunner;
            class ExportMapRunner;
            class CopyFilesRunner;
            class RunToolRunner;
            
            typedef std::list<TaskRunner*> TaskRunnerList;
            
            CompilationContext* m_context;
            TaskRunnerList m_taskRunners;
            TaskRunnerList::iterator m_currentTask;
        public:
            CompilationRunner(CompilationContext* context, const Model::CompilationProfile* profile);
            ~CompilationRunner();
        private:
            class CreateTaskRunnerVisitor;
            static TaskRunnerList createTaskRunners(CompilationContext& context, const Model::CompilationProfile* profile);
        public:
            void execute();
            void terminate();
            bool running() const;
        private:
            void OnTaskError(wxEvent& event);
            void OnTaskEnd(wxEvent& event);

            void bindEvents(TaskRunner* runner);
            void unbindEvents(TaskRunner* runner);
        private:
            CompilationRunner(const CompilationRunner& other);
            CompilationRunner& operator=(const CompilationRunner& other);
        };
    }
}

#endif /* CompilationRunner_h */

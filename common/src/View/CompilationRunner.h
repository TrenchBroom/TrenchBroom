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

#include "View/CompilationContext.h"

namespace TrenchBroom {
    namespace Model {
        class CompilationProfile;
    }
    
    namespace View {
        class CompilationRunner {
        private:
            class TaskRunner;
            class ExportMapRunner;
            class CopyFilesRunner;
            class RunToolRunner;
            
            CompilationContext* m_context;
            TaskRunner* m_runnerChain;
        public:
            CompilationRunner(CompilationContext* context, const Model::CompilationProfile* profile);
            ~CompilationRunner();
        private:
            class CreateTaskRunnerVisitor;
            static TaskRunner* createRunnerChain(CompilationContext& context, const Model::CompilationProfile* profile);
        public:
            void execute();
            void terminate();
            bool running() const;
        private:
            CompilationRunner(const CompilationRunner& other);
            CompilationRunner& operator=(const CompilationRunner& other);
        };
    }
}

#endif /* CompilationRunner_h */

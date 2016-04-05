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

#ifndef CompilationProfile_h
#define CompilationProfile_h

#include "StringUtils.h"
#include "Model/CompilationTask.h"

#include <vector>

namespace TrenchBroom {
    namespace Model {
        class CompilationContext;

        class CompilationProfileRunner {
        private:
            CompilationTask::TaskRunner* m_tasks;
        public:
            CompilationProfileRunner(CompilationContext& context, const CompilationTask::List& tasks);
            ~CompilationProfileRunner();
            
            void execute();
            void terminate();
        private:
            CompilationProfileRunner(const CompilationProfileRunner& other);
            CompilationProfileRunner& operator=(const CompilationProfileRunner& other);
        };
        
        class CompilationProfile {
        public:
            typedef std::vector<CompilationProfile> List;
        private:
            String m_name;
            CompilationTask::List m_tasks;
        public:
            CompilationProfile(const String& name);
            CompilationProfile(const String& name, const CompilationTask::List& tasks);
            CompilationProfile(const CompilationProfile& other);
            ~CompilationProfile();

            CompilationProfile& operator=(CompilationProfile other);
            friend void swap(CompilationProfile& lhs, CompilationProfile& rhs);

            const String& name() const;
            size_t taskCount() const;
            const CompilationTask& task(size_t index) const;
            
            CompilationProfileRunner* createRunner(CompilationContext& context) const;
        };
    }
}

#endif /* CompilationProfile_h */

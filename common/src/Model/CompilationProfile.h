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

#ifndef CompilationProfile_h
#define CompilationProfile_h

#include "Notifier.h"
#include "StringType.h"
#include "Model/CompilationTask.h"

#include <vector>

namespace TrenchBroom {
    namespace Model {
        class CompilationProfile {
        public:
            using List = std::vector<CompilationProfile*>;

            Notifier<> profileWillBeRemoved;
            Notifier<> profileDidChange;
        private:
            String m_name;
            String m_workDirSpec;
            CompilationTask::List m_tasks;
        public:
            CompilationProfile(const String& name, const String& workDirSpec);
            CompilationProfile(const String& name, const String& workDirSpec, const CompilationTask::List& tasks);
            ~CompilationProfile();

            CompilationProfile* clone() const;

            const String& name() const;
            void setName(const String& name);

            const String& workDirSpec() const;
            void setWorkDirSpec(const String& workDirSpec);

            size_t taskCount() const;
            CompilationTask* task(size_t index) const;

            void addTask(CompilationTask* task);
            void insertTask(size_t index, CompilationTask* task);
            void removeTask(size_t index);

            void moveTaskUp(size_t index);
            void moveTaskDown(size_t index);

            void accept(CompilationTaskVisitor& visitor);
            void accept(ConstCompilationTaskVisitor& visitor) const;
            void accept(const CompilationTaskConstVisitor& visitor);
            void accept(const ConstCompilationTaskConstVisitor& visitor) const;

            deleteCopyAndMove(CompilationProfile)
        };
    }
}

#endif /* CompilationProfile_h */

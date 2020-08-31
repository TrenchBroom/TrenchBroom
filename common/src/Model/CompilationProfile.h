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

#include "Macros.h"
#include "Notifier.h"

#include <memory>
#include <string>
#include <vector>

namespace TrenchBroom {
    namespace Model {
        class CompilationConfig;
        class CompilationTask;
        class CompilationTaskConstVisitor;
        class CompilationTaskVisitor;
        class ConstCompilationTaskVisitor;
        class ConstCompilationTaskConstVisitor;

        class CompilationProfile {
        public:
            Notifier<> profileWillBeRemoved;
            /**
             * Called when name or workDirSpec changed, or tasks are added/removed
             * (not when tasks are changed).
             */
            Notifier<> profileDidChange;
        private:
            std::string m_name;
            std::string m_workDirSpec;
            std::vector<std::unique_ptr<CompilationTask>> m_tasks;
            CompilationConfig* m_parent;
        public:
            CompilationProfile(const std::string& name, const std::string& workDirSpec);
            CompilationProfile(const std::string& name, const std::string& workDirSpec, std::vector<std::unique_ptr<CompilationTask>> tasks);
            ~CompilationProfile();

            std::unique_ptr<CompilationProfile> clone() const;

            CompilationConfig* parent() const;
            void setParent(CompilationConfig* parent);

            const std::string& name() const;
            void setName(const std::string& name);

            const std::string& workDirSpec() const;
            void setWorkDirSpec(const std::string& workDirSpec);

            size_t taskCount() const;
            CompilationTask* task(size_t index) const;

            void addTask(std::unique_ptr<CompilationTask> task);
            void insertTask(size_t index, std::unique_ptr<CompilationTask> task);
            void removeTask(size_t index);

            void moveTaskUp(size_t index);
            void moveTaskDown(size_t index);

            void accept(CompilationTaskVisitor& visitor);
            void accept(ConstCompilationTaskVisitor& visitor) const;
            void accept(const CompilationTaskConstVisitor& visitor);
            void accept(const ConstCompilationTaskConstVisitor& visitor) const;
        private:
            void sendDidChangeNotifications();
        public:
            void taskDidChange(CompilationTask* task);

            deleteCopyAndMove(CompilationProfile)
        };
    }
}

#endif /* CompilationProfile_h */

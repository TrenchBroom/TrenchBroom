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

#include "CompilationProfile.h"

#include "CollectionUtils.h"
#include "Model/CompilationTask.h"

namespace TrenchBroom {
    namespace Model {
        CompilationProfileRunner::CompilationProfileRunner(CompilationContext& context, const CompilationTask::List& tasks) :
        m_tasks(NULL) {
            if (!tasks.empty()) {
                CompilationTask::List::const_reverse_iterator it = tasks.rbegin();
                CompilationTask::TaskRunner* runner = (*it)->createTaskRunner(context);
                ++it;
                
                while (it != tasks.rend()) {
                    runner = (*it)->createTaskRunner(context, runner);
                    ++it;
                }
                
                m_tasks = runner;
            }
        }
        
        CompilationProfileRunner::~CompilationProfileRunner() {
            if (m_tasks != NULL) {
                m_tasks->terminate();
                delete m_tasks;
            }
        }

        void CompilationProfileRunner::execute() {
            if (m_tasks != NULL)
                m_tasks->execute();
        }
        
        void CompilationProfileRunner::terminate() {
            if (m_tasks != NULL)
                m_tasks->terminate();
        }

        CompilationProfile::CompilationProfile(const String& name) :
        m_name(name) {}
        
        CompilationProfile::CompilationProfile(const String& name, const CompilationTask::List& tasks) :
        m_name(name),
        m_tasks(tasks) {}

        CompilationProfile::CompilationProfile(const CompilationProfile& other) :
        m_name(other.m_name) {
            CompilationTask::List::const_iterator it, end;
            for (it = other.m_tasks.begin(), end = other.m_tasks.end(); it != end; ++it) {
                const CompilationTask* original = *it;
                m_tasks.push_back(original->clone());
            }
        }

        CompilationProfile::~CompilationProfile() {
            VectorUtils::clearAndDelete(m_tasks);
        }

        CompilationProfile& CompilationProfile::operator=(CompilationProfile other) {
            using std::swap;
            swap(*this, other);
            return *this;
        }
        
        void swap(CompilationProfile& lhs, CompilationProfile& rhs) {
            using std::swap;
            swap(lhs.m_name, rhs.m_name);
            swap(lhs.m_tasks, rhs.m_tasks);
        }

        const String& CompilationProfile::name() const  {
            return m_name;
        }
        
        size_t CompilationProfile::taskCount() const {
            return m_tasks.size();
        }

        const CompilationTask& CompilationProfile::task(const size_t index) const {
            assert(index < taskCount());
            return *m_tasks[index];
        }
        
        CompilationProfileRunner* CompilationProfile::createRunner(CompilationContext& context) const {
            return new CompilationProfileRunner(context, m_tasks);
        }
    }
}

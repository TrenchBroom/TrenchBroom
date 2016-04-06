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
        
        void CompilationProfile::setName(const String& name) {
            m_name = name;
            profileDidChange();
        }

        size_t CompilationProfile::taskCount() const {
            return m_tasks.size();
        }
        
        CompilationTask& CompilationProfile::task(const size_t index) {
            assert(index < taskCount());
            return *m_tasks[index];
        }

        const CompilationTask& CompilationProfile::task(const size_t index) const {
            assert(index < taskCount());
            return *m_tasks[index];
        }
        
        void CompilationProfile::addTask(const CompilationTask& task) {
            m_tasks.push_back(task.clone());
            profileDidChange();
        }
        
        void CompilationProfile::removeTask(const size_t index) {
            assert(index < taskCount());
            delete m_tasks[index];
            VectorUtils::erase(m_tasks, index);
            profileDidChange();
        }
        
        void CompilationProfile::moveTaskUp(const size_t index) {
            assert(index > 0);
            assert(index < taskCount());
            
            CompilationTask::List::iterator it = m_tasks.begin();
            std::advance(it, index);
            
            CompilationTask::List::iterator pr = m_tasks.begin();
            std::advance(pr, index - 1);
            
            std::iter_swap(it, pr);
            profileDidChange();
        }
        
        void CompilationProfile::moveTaskDown(const size_t index) {
            assert(index < taskCount() - 1);
            
            CompilationTask::List::iterator it = m_tasks.begin();
            std::advance(it, index);
            
            CompilationTask::List::iterator nx = m_tasks.begin();
            std::advance(nx, index - 1);
            
            std::iter_swap(it, nx);
            profileDidChange();
        }

        void CompilationProfile::accept(CompilationTaskVisitor& visitor) {
            CompilationTask::List::iterator it, end;
            for (it = m_tasks.begin(), end = m_tasks.end(); it != end; ++it) {
                CompilationTask* task = *it;
                task->accept(visitor);
            }
        }
        
        void CompilationProfile::accept(ConstCompilationTaskVisitor& visitor) const {
            CompilationTask::List::const_iterator it, end;
            for (it = m_tasks.begin(), end = m_tasks.end(); it != end; ++it) {
                CompilationTask* task = *it;
                task->accept(visitor);
            }
        }
        
        void CompilationProfile::accept(const CompilationTaskConstVisitor& visitor) {
            CompilationTask::List::iterator it, end;
            for (it = m_tasks.begin(), end = m_tasks.end(); it != end; ++it) {
                CompilationTask* task = *it;
                task->accept(visitor);
            }
        }
        
        void CompilationProfile::accept(const ConstCompilationTaskConstVisitor& visitor) const {
            CompilationTask::List::const_iterator it, end;
            for (it = m_tasks.begin(), end = m_tasks.end(); it != end; ++it) {
                CompilationTask* task = *it;
                task->accept(visitor);
            }
        }
    }
}

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
        CompilationProfile::CompilationProfile(const String& name, const String& workDirSpec) :
        m_name(name),
        m_workDirSpec(workDirSpec) {}

        CompilationProfile::CompilationProfile(const String& name, const String& workDirSpec, const CompilationTask::Array& tasks) :
        m_name(name),
        m_workDirSpec(workDirSpec),
        m_tasks(tasks) {
            for (CompilationTask* task : m_tasks)
                task->taskDidChange.addObserver(taskDidChange);
        }

        CompilationProfile::~CompilationProfile() {
            VectorUtils::clearAndDelete(m_tasks);
        }

        CompilationProfile* CompilationProfile::clone() const {
            CompilationTask::Array clones;
            clones.reserve(m_tasks.size());

            for (const CompilationTask* original : m_tasks)
                clones.push_back(original->clone());
            
            return new CompilationProfile(m_name, m_workDirSpec, clones);
        }

        const String& CompilationProfile::name() const  {
            return m_name;
        }
        
        void CompilationProfile::setName(const String& name) {
            m_name = name;
            profileDidChange();
        }

        const String& CompilationProfile::workDirSpec() const {
            return m_workDirSpec;
        }
        
        void CompilationProfile::setWorkDirSpec(const String& workDirSpec) {
            m_workDirSpec = workDirSpec;
            profileDidChange();
        }

        
        size_t CompilationProfile::taskCount() const {
            return m_tasks.size();
        }
        
        CompilationTask* CompilationProfile::task(const size_t index) const {
            assert(index < taskCount());
            return m_tasks[index];
        }
        
        void CompilationProfile::addTask(CompilationTask* task) {
            insertTask(m_tasks.size(), task);
        }
        
        void CompilationProfile::insertTask(const size_t index, CompilationTask* task) {
            assert(index <= m_tasks.size());
            ensure(task != NULL, "task is null");
            
            if (index == m_tasks.size()) {
                m_tasks.push_back(task);
            } else {
                CompilationTask::Array::iterator it = std::begin(m_tasks);
                std::advance(it, static_cast<int>(index));
                m_tasks.insert(it, task);
                
            }
            task->taskDidChange.addObserver(taskDidChange);
            profileDidChange();
        }

        void CompilationProfile::removeTask(const size_t index) {
            assert(index < taskCount());
            m_tasks[index]->taskWillBeRemoved();
            delete m_tasks[index];
            VectorUtils::erase(m_tasks, index);
            profileDidChange();
        }
        
        void CompilationProfile::moveTaskUp(const size_t index) {
            assert(index > 0);
            assert(index < taskCount());
            
            CompilationTask::Array::iterator it = std::begin(m_tasks);
            std::advance(it, static_cast<int>(index));
            
            CompilationTask::Array::iterator pr = std::begin(m_tasks);
            std::advance(pr, static_cast<int>(index) - 1);
            
            std::iter_swap(it, pr);
            profileDidChange();
        }
        
        void CompilationProfile::moveTaskDown(const size_t index) {
            assert(index < taskCount() - 1);
            
            CompilationTask::Array::iterator it = std::begin(m_tasks);
            std::advance(it, static_cast<int>(index));
            
            CompilationTask::Array::iterator nx = std::begin(m_tasks);
            std::advance(nx, static_cast<int>(index) + 1);
            
            std::iter_swap(it, nx);
            profileDidChange();
        }

        void CompilationProfile::accept(CompilationTaskVisitor& visitor) {
            for (CompilationTask* task : m_tasks)
                task->accept(visitor);
        }
        
        void CompilationProfile::accept(ConstCompilationTaskVisitor& visitor) const {
            for (CompilationTask* task : m_tasks)
                task->accept(visitor);
        }
        
        void CompilationProfile::accept(const CompilationTaskConstVisitor& visitor) {
            for (CompilationTask* task : m_tasks)
                task->accept(visitor);
        }
        
        void CompilationProfile::accept(const ConstCompilationTaskConstVisitor& visitor) const {
            for (CompilationTask* task : m_tasks)
                task->accept(visitor);
        }
    }
}

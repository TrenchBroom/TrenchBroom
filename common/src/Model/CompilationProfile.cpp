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

#include "CompilationProfile.h"

#include "Ensure.h"
#include "Model/CompilationTask.h"

#include <kdl/vector_utils.h>

#include <string>

namespace TrenchBroom {
    namespace Model {
        CompilationProfile::CompilationProfile(const std::string& name, const std::string& workDirSpec) :
        m_name(name),
        m_workDirSpec(workDirSpec) {}

        CompilationProfile::CompilationProfile(const std::string& name, const std::string& workDirSpec, std::vector<std::unique_ptr<CompilationTask>> tasks) :
        m_name(name),
        m_workDirSpec(workDirSpec),
        m_tasks(std::move(tasks)) {}

        CompilationProfile::~CompilationProfile() = default;

        std::unique_ptr<CompilationProfile> CompilationProfile::clone() const {
            std::vector<std::unique_ptr<CompilationTask>> clones;
            clones.reserve(m_tasks.size());

            for (const auto& original : m_tasks) {
                clones.push_back(std::unique_ptr<CompilationTask>(original->clone()));
            }

            return std::make_unique<CompilationProfile>(m_name, m_workDirSpec, std::move(clones));
        }

        const std::string& CompilationProfile::name() const  {
            return m_name;
        }

        void CompilationProfile::setName(const std::string& name) {
            m_name = name;
            profileDidChange();
        }

        const std::string& CompilationProfile::workDirSpec() const {
            return m_workDirSpec;
        }

        void CompilationProfile::setWorkDirSpec(const std::string& workDirSpec) {
            m_workDirSpec = workDirSpec;
            profileDidChange();
        }


        size_t CompilationProfile::taskCount() const {
            return m_tasks.size();
        }

        CompilationTask* CompilationProfile::task(const size_t index) const {
            assert(index < taskCount());
            return m_tasks[index].get();
        }

        void CompilationProfile::addTask(std::unique_ptr<CompilationTask> task) {
            insertTask(m_tasks.size(), std::move(task));
        }

        void CompilationProfile::insertTask(const size_t index, std::unique_ptr<CompilationTask> task) {
            assert(index <= m_tasks.size());
            ensure(task != nullptr, "task is null");

            if (index == m_tasks.size()) {
                m_tasks.push_back(std::move(task));
            } else {
                auto it = std::begin(m_tasks);
                std::advance(it, static_cast<int>(index));
                m_tasks.insert(it, std::move(task));

            }
            profileDidChange();
        }

        void CompilationProfile::removeTask(const size_t index) {
            assert(index < taskCount());
            m_tasks[index]->taskWillBeRemoved();
            kdl::vec_erase_at(m_tasks, index);
            profileDidChange();
        }

        void CompilationProfile::moveTaskUp(const size_t index) {
            assert(index > 0);
            assert(index < taskCount());

            auto it = std::begin(m_tasks);
            std::advance(it, static_cast<int>(index));

            auto pr = std::begin(m_tasks);
            std::advance(pr, static_cast<int>(index) - 1);

            std::iter_swap(it, pr);
            profileDidChange();
        }

        void CompilationProfile::moveTaskDown(const size_t index) {
            assert(index < taskCount() - 1);

            auto it = std::begin(m_tasks);
            std::advance(it, static_cast<int>(index));

            auto nx = std::begin(m_tasks);
            std::advance(nx, static_cast<int>(index) + 1);

            std::iter_swap(it, nx);
            profileDidChange();
        }

        void CompilationProfile::accept(CompilationTaskVisitor& visitor) {
            for (auto& task : m_tasks) {
                task->accept(visitor);
            }
        }

        void CompilationProfile::accept(ConstCompilationTaskVisitor& visitor) const {
            for (auto& task : m_tasks) {
                task->accept(visitor);
            }
        }

        void CompilationProfile::accept(const CompilationTaskConstVisitor& visitor) {
            for (auto& task : m_tasks) {
                task->accept(visitor);
            }
        }

        void CompilationProfile::accept(const ConstCompilationTaskConstVisitor& visitor) const {
            for (auto& task : m_tasks) {
                task->accept(visitor);
            }
        }
    }
}

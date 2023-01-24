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

#include <kdl/deref_iterator.h>
#include <kdl/struct_io.h>
#include <kdl/vector_utils.h>

#include <ostream>
#include <string>

namespace TrenchBroom
{
namespace Model
{
CompilationProfile::CompilationProfile(std::string name, std::string workDirSpec)
  : CompilationProfile{std::move(name), std::move(workDirSpec), {}}
{
}

CompilationProfile::CompilationProfile(
  std::string name,
  std::string workDirSpec,
  std::vector<std::unique_ptr<CompilationTask>> tasks)
  : m_name{std::move(name)}
  , m_workDirSpec{std::move(workDirSpec)}
  , m_tasks{std::move(tasks)}
{
}

CompilationProfile::~CompilationProfile() = default;

std::unique_ptr<CompilationProfile> CompilationProfile::clone() const
{
  auto clones = std::vector<std::unique_ptr<CompilationTask>>{};
  clones.reserve(m_tasks.size());

  for (const auto& original : m_tasks)
  {
    clones.emplace_back(original->clone());
  }

  return std::make_unique<CompilationProfile>(m_name, m_workDirSpec, std::move(clones));
}

bool operator==(const CompilationProfile& lhs, const CompilationProfile& rhs)
{
  return lhs.m_name == rhs.m_name && lhs.m_workDirSpec == rhs.m_workDirSpec
         && kdl::const_deref_range{lhs.m_tasks} == kdl::const_deref_range{rhs.m_tasks};
}

bool operator!=(const CompilationProfile& lhs, const CompilationProfile& rhs)
{
  return !(lhs == rhs);
}

std::ostream& operator<<(std::ostream& str, const CompilationProfile& profile)
{
  kdl::struct_stream{str} << "CompilationProfile"
                          << "m_name" << profile.m_name << "m_workDirSpec"
                          << profile.m_workDirSpec << "m_tasks"
                          << kdl::const_deref_range{profile.m_tasks};
  return str;
}

const std::string& CompilationProfile::name() const
{
  return m_name;
}

void CompilationProfile::setName(std::string name)
{
  m_name = std::move(name);
}

const std::string& CompilationProfile::workDirSpec() const
{
  return m_workDirSpec;
}

void CompilationProfile::setWorkDirSpec(std::string workDirSpec)
{
  m_workDirSpec = std::move(workDirSpec);
}

size_t CompilationProfile::taskCount() const
{
  return m_tasks.size();
}

CompilationTask* CompilationProfile::task(const size_t index) const
{
  assert(index < taskCount());
  return m_tasks[index].get();
}

size_t CompilationProfile::indexOfTask(CompilationTask* task) const
{
  auto result =
    kdl::vec_index_of(m_tasks, [=](const auto& ptr) { return ptr.get() == task; });
  return *result;
}

void CompilationProfile::addTask(std::unique_ptr<CompilationTask> task)
{
  insertTask(m_tasks.size(), std::move(task));
}

void CompilationProfile::insertTask(
  const size_t index, std::unique_ptr<CompilationTask> task)
{
  assert(index <= m_tasks.size());
  ensure(task != nullptr, "task is null");

  if (index == m_tasks.size())
  {
    m_tasks.push_back(std::move(task));
  }
  else
  {
    auto it = std::begin(m_tasks);
    std::advance(it, static_cast<int>(index));
    m_tasks.insert(it, std::move(task));
  }
}

void CompilationProfile::removeTask(const size_t index)
{
  assert(index < taskCount());
  m_tasks = kdl::vec_erase_at(std::move(m_tasks), index);
}

void CompilationProfile::moveTaskUp(const size_t index)
{
  assert(index > 0);
  assert(index < taskCount());

  auto it = std::begin(m_tasks);
  std::advance(it, static_cast<int>(index));

  auto pr = std::begin(m_tasks);
  std::advance(pr, static_cast<int>(index) - 1);

  std::iter_swap(it, pr);
}

void CompilationProfile::moveTaskDown(const size_t index)
{
  assert(index < taskCount() - 1);

  auto it = std::begin(m_tasks);
  std::advance(it, static_cast<int>(index));

  auto nx = std::begin(m_tasks);
  std::advance(nx, static_cast<int>(index) + 1);

  std::iter_swap(it, nx);
}

void CompilationProfile::accept(CompilationTaskVisitor& visitor)
{
  for (auto& task : m_tasks)
  {
    task->accept(visitor);
  }
}

void CompilationProfile::accept(ConstCompilationTaskVisitor& visitor) const
{
  for (auto& task : m_tasks)
  {
    task->accept(visitor);
  }
}

void CompilationProfile::accept(const CompilationTaskConstVisitor& visitor)
{
  for (auto& task : m_tasks)
  {
    task->accept(visitor);
  }
}

void CompilationProfile::accept(const ConstCompilationTaskConstVisitor& visitor) const
{
  for (auto& task : m_tasks)
  {
    task->accept(visitor);
  }
}
} // namespace Model
} // namespace TrenchBroom

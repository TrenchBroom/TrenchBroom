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

#pragma once

#include "Macros.h"

#include <iosfwd>
#include <memory>
#include <string>
#include <vector>

namespace TrenchBroom
{
namespace Model
{
class CompilationTask;
class CompilationTaskConstVisitor;
class CompilationTaskVisitor;
class ConstCompilationTaskVisitor;
class ConstCompilationTaskConstVisitor;

class CompilationProfile
{
private:
  std::string m_name;
  std::string m_workDirSpec;
  std::vector<std::unique_ptr<CompilationTask>> m_tasks;

public:
  CompilationProfile(std::string name, std::string workDirSpec);
  CompilationProfile(
    std::string name,
    std::string workDirSpec,
    std::vector<std::unique_ptr<CompilationTask>> tasks);
  ~CompilationProfile();

  std::unique_ptr<CompilationProfile> clone() const;
  friend bool operator==(const CompilationProfile& lhs, const CompilationProfile& rhs);
  friend bool operator!=(const CompilationProfile& lhs, const CompilationProfile& rhs);
  friend std::ostream& operator<<(std::ostream& str, const CompilationProfile& profile);

  const std::string& name() const;
  void setName(std::string name);

  const std::string& workDirSpec() const;
  void setWorkDirSpec(std::string workDirSpec);

  size_t taskCount() const;
  CompilationTask* task(size_t index) const;
  size_t indexOfTask(CompilationTask* task) const;

  void addTask(std::unique_ptr<CompilationTask> task);
  void insertTask(size_t index, std::unique_ptr<CompilationTask> task);
  void removeTask(size_t index);

  void moveTaskUp(size_t index);
  void moveTaskDown(size_t index);

  void accept(CompilationTaskVisitor& visitor);
  void accept(ConstCompilationTaskVisitor& visitor) const;
  void accept(const CompilationTaskConstVisitor& visitor);
  void accept(const ConstCompilationTaskConstVisitor& visitor) const;

  deleteCopyAndMove(CompilationProfile);
};
} // namespace Model
} // namespace TrenchBroom

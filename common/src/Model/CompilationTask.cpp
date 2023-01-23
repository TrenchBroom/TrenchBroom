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

#include "CompilationTask.h"

#include <kdl/struct_io.h>

#include <string>

namespace TrenchBroom
{
namespace Model
{
// CompilationTask

CompilationTask::CompilationTask(const bool enabled)
  : m_enabled{enabled}
{
}

CompilationTask::~CompilationTask() = default;

bool CompilationTask::enabled() const
{
  return m_enabled;
}

void CompilationTask::setEnabled(const bool enabled)
{
  m_enabled = enabled;
}

bool CompilationTask::operator!=(const CompilationTask& other) const
{
  return !(*this == other);
}

std::ostream& operator<<(std::ostream& str, const CompilationTask& task)
{
  task.appendToStream(str);
  return str;
}

// CompilationExportMap

CompilationExportMap::CompilationExportMap(const bool enabled, std::string targetSpec)
  : CompilationTask{enabled}
  , m_targetSpec{std::move(targetSpec)}
{
}

void CompilationExportMap::accept(CompilationTaskVisitor& visitor)
{
  visitor.visit(*this);
}

void CompilationExportMap::accept(ConstCompilationTaskVisitor& visitor) const
{
  visitor.visit(*this);
}

void CompilationExportMap::accept(const CompilationTaskConstVisitor& visitor)
{
  visitor.visit(*this);
}

void CompilationExportMap::accept(const ConstCompilationTaskConstVisitor& visitor) const
{
  visitor.visit(*this);
}

const std::string& CompilationExportMap::targetSpec() const
{
  return m_targetSpec;
}

void CompilationExportMap::setTargetSpec(std::string targetSpec)
{
  m_targetSpec = std::move(targetSpec);
}

CompilationExportMap* CompilationExportMap::clone() const
{
  return new CompilationExportMap{enabled(), m_targetSpec};
}

bool CompilationExportMap::operator==(const CompilationTask& other) const
{
  auto* otherCasted = dynamic_cast<const CompilationExportMap*>(&other);
  return otherCasted && m_enabled == otherCasted->m_enabled
         && m_targetSpec == otherCasted->m_targetSpec;
}

void CompilationExportMap::appendToStream(std::ostream& str) const
{
  kdl::struct_stream{str} << "CompilationExportMap"
                          << "m_enabled" << m_enabled << "m_targetSpec" << m_targetSpec;
}

// CompilationCopyFiles

CompilationCopyFiles::CompilationCopyFiles(
  const bool enabled, std::string sourceSpec, std::string targetSpec)
  : CompilationTask{enabled}
  , m_sourceSpec{std::move(sourceSpec)}
  , m_targetSpec{std::move(targetSpec)}
{
}

void CompilationCopyFiles::accept(CompilationTaskVisitor& visitor)
{
  visitor.visit(*this);
}

void CompilationCopyFiles::accept(ConstCompilationTaskVisitor& visitor) const
{
  visitor.visit(*this);
}

void CompilationCopyFiles::accept(const CompilationTaskConstVisitor& visitor)
{
  visitor.visit(*this);
}

void CompilationCopyFiles::accept(const ConstCompilationTaskConstVisitor& visitor) const
{
  visitor.visit(*this);
}

const std::string& CompilationCopyFiles::sourceSpec() const
{
  return m_sourceSpec;
}

const std::string& CompilationCopyFiles::targetSpec() const
{
  return m_targetSpec;
}

void CompilationCopyFiles::setSourceSpec(std::string sourceSpec)
{
  m_sourceSpec = std::move(sourceSpec);
}

void CompilationCopyFiles::setTargetSpec(std::string targetSpec)
{
  m_targetSpec = std::move(targetSpec);
}

CompilationCopyFiles* CompilationCopyFiles::clone() const
{
  return new CompilationCopyFiles{enabled(), m_sourceSpec, m_targetSpec};
}

bool CompilationCopyFiles::operator==(const CompilationTask& other) const
{
  auto* otherCasted = dynamic_cast<const CompilationCopyFiles*>(&other);
  return otherCasted && m_enabled == otherCasted->m_enabled
         && m_sourceSpec == otherCasted->m_sourceSpec
         && m_targetSpec == otherCasted->m_targetSpec;
}

void CompilationCopyFiles::appendToStream(std::ostream& str) const
{
  kdl::struct_stream{str} << "CompilationCopyFiles"
                          << "m_enabled" << m_enabled << "m_sourceSpec" << m_sourceSpec
                          << "m_targetSpec" << m_targetSpec;
}

// CompilationRunTool

CompilationRunTool::CompilationRunTool(
  const bool enabled, std::string toolSpec, std::string parameterSpec)
  : CompilationTask{enabled}
  , m_toolSpec{std::move(toolSpec)}
  , m_parameterSpec{std::move(parameterSpec)}
{
}

void CompilationRunTool::accept(CompilationTaskVisitor& visitor)
{
  visitor.visit(*this);
}

void CompilationRunTool::accept(ConstCompilationTaskVisitor& visitor) const
{
  visitor.visit(*this);
}

void CompilationRunTool::accept(const CompilationTaskConstVisitor& visitor)
{
  visitor.visit(*this);
}

void CompilationRunTool::accept(const ConstCompilationTaskConstVisitor& visitor) const
{
  visitor.visit(*this);
}

const std::string& CompilationRunTool::toolSpec() const
{
  return m_toolSpec;
}

const std::string& CompilationRunTool::parameterSpec() const
{
  return m_parameterSpec;
}

void CompilationRunTool::setToolSpec(std::string toolSpec)
{
  m_toolSpec = std::move(toolSpec);
}

void CompilationRunTool::setParameterSpec(std::string parameterSpec)
{
  m_parameterSpec = std::move(parameterSpec);
}

CompilationRunTool* CompilationRunTool::clone() const
{
  return new CompilationRunTool{enabled(), m_toolSpec, m_parameterSpec};
}

bool CompilationRunTool::operator==(const CompilationTask& other) const
{
  auto* otherCasted = dynamic_cast<const CompilationRunTool*>(&other);
  return otherCasted && m_enabled == otherCasted->m_enabled
         && m_toolSpec == otherCasted->m_toolSpec
         && m_parameterSpec == otherCasted->m_parameterSpec;
}

void CompilationRunTool::appendToStream(std::ostream& str) const
{
  kdl::struct_stream{str} << "CompilationRunTool"
                          << "m_enabled" << m_enabled << "m_toolSpec" << m_toolSpec
                          << "m_parameterSpec" << m_parameterSpec;
}

CompilationTaskVisitor::~CompilationTaskVisitor() = default;
ConstCompilationTaskVisitor::~ConstCompilationTaskVisitor() = default;
CompilationTaskConstVisitor::~CompilationTaskConstVisitor() = default;
ConstCompilationTaskConstVisitor::~ConstCompilationTaskConstVisitor() = default;
} // namespace Model
} // namespace TrenchBroom

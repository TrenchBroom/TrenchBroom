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
  : m_enabled(enabled)
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

CompilationExportMap::CompilationExportMap(
  const bool enabled, const std::string& targetSpec)
  : CompilationTask(enabled)
  , m_targetSpec(targetSpec)
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

void CompilationExportMap::setTargetSpec(const std::string& targetSpec)
{
  m_targetSpec = targetSpec;
}

CompilationExportMap* CompilationExportMap::clone() const
{
  return new CompilationExportMap(enabled(), m_targetSpec);
}

bool CompilationExportMap::operator==(const CompilationTask& other) const
{
  auto* otherCasted = dynamic_cast<const CompilationExportMap*>(&other);
  if (otherCasted == nullptr)
  {
    return false;
  }
  if (m_enabled != otherCasted->m_enabled)
  {
    return false;
  }
  if (m_targetSpec != otherCasted->m_targetSpec)
  {
    return false;
  }
  return true;
}

void CompilationExportMap::appendToStream(std::ostream& str) const
{
  kdl::struct_stream{str} << "CompilationExportMap"
                          << "m_enabled" << m_enabled << "m_targetSpec" << m_targetSpec;
}

// CompilationCopyFiles

CompilationCopyFiles::CompilationCopyFiles(
  const bool enabled,
  const bool targetIsFileSpec,
  const std::string& sourceSpec,
  const std::string& targetSpec)
  : CompilationTask(enabled)
  , m_targetIsFileSpec(targetIsFileSpec)
  , m_sourceSpec(sourceSpec)
  , m_targetSpec(targetSpec)
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

bool CompilationCopyFiles::targetIsFileSpec() const
{
  return m_targetIsFileSpec;
}

const std::string& CompilationCopyFiles::sourceSpec() const
{
  return m_sourceSpec;
}

const std::string& CompilationCopyFiles::targetSpec() const
{
  return m_targetSpec;
}

void CompilationCopyFiles::setTargetIsFileSpec(const bool targetIsFileSpec)
{
  m_targetIsFileSpec = targetIsFileSpec;
}

void CompilationCopyFiles::setSourceSpec(const std::string& sourceSpec)
{
  m_sourceSpec = sourceSpec;
}

void CompilationCopyFiles::setTargetSpec(const std::string& targetSpec)
{
  m_targetSpec = targetSpec;
}

CompilationCopyFiles* CompilationCopyFiles::clone() const
{
  return new CompilationCopyFiles(
    enabled(), m_targetIsFileSpec, m_sourceSpec, m_targetSpec);
}

bool CompilationCopyFiles::operator==(const CompilationTask& other) const
{
  auto* otherCasted = dynamic_cast<const CompilationCopyFiles*>(&other);
  if (otherCasted == nullptr)
  {
    return false;
  }
  if (m_enabled != otherCasted->m_enabled)
  {
    return false;
  }
  if (m_targetIsFileSpec != otherCasted->m_targetIsFileSpec)
  {
    return false;
  }
  if (m_sourceSpec != otherCasted->m_sourceSpec)
  {
    return false;
  }
  if (m_targetSpec != otherCasted->m_targetSpec)
  {
    return false;
  }
  return true;
}

void CompilationCopyFiles::appendToStream(std::ostream& str) const
{
  kdl::struct_stream{str} << "CompilationCopyFiles"
                          << "m_enabled" << m_enabled << "m_targetIsFileSpec"
                          << m_targetIsFileSpec << "m_sourceSpec" << m_sourceSpec
                          << "m_targetSpec" << m_targetSpec;
}

// CompilationRunTool

CompilationRunTool::CompilationRunTool(
  const bool enabled, const std::string& toolSpec, const std::string& parameterSpec)
  : CompilationTask(enabled)
  , m_toolSpec(toolSpec)
  , m_parameterSpec(parameterSpec)
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

void CompilationRunTool::setToolSpec(const std::string& toolSpec)
{
  m_toolSpec = toolSpec;
}

void CompilationRunTool::setParameterSpec(const std::string& parameterSpec)
{
  m_parameterSpec = parameterSpec;
}

CompilationRunTool* CompilationRunTool::clone() const
{
  return new CompilationRunTool(enabled(), m_toolSpec, m_parameterSpec);
}

bool CompilationRunTool::operator==(const CompilationTask& other) const
{
  auto* otherCasted = dynamic_cast<const CompilationRunTool*>(&other);
  if (otherCasted == nullptr)
  {
    return false;
  }
  if (m_enabled != otherCasted->m_enabled)
  {
    return false;
  }
  if (m_toolSpec != otherCasted->m_toolSpec)
  {
    return false;
  }
  if (m_parameterSpec != otherCasted->m_parameterSpec)
  {
    return false;
  }
  return true;
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

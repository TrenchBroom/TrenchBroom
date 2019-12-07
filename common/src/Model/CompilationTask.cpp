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

#include <string>

namespace TrenchBroom {
    namespace Model {
        CompilationTask::CompilationTask() = default;

        CompilationTask::~CompilationTask() = default;

        CompilationExportMap::CompilationExportMap(const std::string& targetSpec) :
        m_targetSpec(targetSpec) {}

        void CompilationExportMap::accept(CompilationTaskVisitor& visitor) {
            visitor.visit(*this);
        }

        void CompilationExportMap::accept(ConstCompilationTaskVisitor& visitor) const {
            visitor.visit(*this);
        }

        void CompilationExportMap::accept(const CompilationTaskConstVisitor& visitor) {
            visitor.visit(*this);
        }

        void CompilationExportMap::accept(const ConstCompilationTaskConstVisitor& visitor) const {
            visitor.visit(*this);
        }

        const std::string& CompilationExportMap::targetSpec() const {
            return m_targetSpec;
        }

        void CompilationExportMap::setTargetSpec(const std::string& targetSpec) {
            m_targetSpec = targetSpec;
            taskDidChange();
        }

        CompilationExportMap* CompilationExportMap::clone() const {
            return new CompilationExportMap(m_targetSpec);
        }

        CompilationCopyFiles::CompilationCopyFiles(const std::string& sourceSpec, const std::string& targetSpec) :
        CompilationTask(),
        m_sourceSpec(sourceSpec),
        m_targetSpec(targetSpec) {}

        void CompilationCopyFiles::accept(CompilationTaskVisitor& visitor) {
            visitor.visit(*this);
        }

        void CompilationCopyFiles::accept(ConstCompilationTaskVisitor& visitor) const {
            visitor.visit(*this);
        }

        void CompilationCopyFiles::accept(const CompilationTaskConstVisitor& visitor) {
            visitor.visit(*this);
        }

        void CompilationCopyFiles::accept(const ConstCompilationTaskConstVisitor& visitor) const {
            visitor.visit(*this);
        }

        const std::string& CompilationCopyFiles::sourceSpec() const {
            return m_sourceSpec;
        }

        const std::string& CompilationCopyFiles::targetSpec() const {
            return m_targetSpec;
        }

        void CompilationCopyFiles::setSourceSpec(const std::string& sourceSpec) {
            m_sourceSpec = sourceSpec;
            taskDidChange();
        }

        void CompilationCopyFiles::setTargetSpec(const std::string& targetSpec) {
            m_targetSpec = targetSpec;
            taskDidChange();
        }

        CompilationCopyFiles* CompilationCopyFiles::clone() const {
            return new CompilationCopyFiles(m_sourceSpec, m_targetSpec);
        }

        CompilationRunTool::CompilationRunTool(const std::string& toolSpec, const std::string& parameterSpec) :
        CompilationTask(),
        m_toolSpec(toolSpec),
        m_parameterSpec(parameterSpec) {}

        void CompilationRunTool::accept(CompilationTaskVisitor& visitor) {
            visitor.visit(*this);
        }

        void CompilationRunTool::accept(ConstCompilationTaskVisitor& visitor) const {
            visitor.visit(*this);
        }

        void CompilationRunTool::accept(const CompilationTaskConstVisitor& visitor) {
            visitor.visit(*this);
        }

        void CompilationRunTool::accept(const ConstCompilationTaskConstVisitor& visitor) const {
            visitor.visit(*this);
        }

        const std::string& CompilationRunTool::toolSpec() const {
            return m_toolSpec;
        }

        const std::string& CompilationRunTool::parameterSpec() const {
            return m_parameterSpec;
        }

        void CompilationRunTool::setToolSpec(const std::string& toolSpec) {
            m_toolSpec = toolSpec;
            taskDidChange();
        }

        void CompilationRunTool::setParameterSpec(const std::string& parameterSpec) {
            m_parameterSpec = parameterSpec;
            taskDidChange();
        }

        CompilationRunTool* CompilationRunTool::clone() const {
            return new CompilationRunTool(m_toolSpec, m_parameterSpec);
        }

        CompilationTaskVisitor::~CompilationTaskVisitor() = default;
        ConstCompilationTaskVisitor::~ConstCompilationTaskVisitor() = default;
        CompilationTaskConstVisitor::~CompilationTaskConstVisitor() = default;
        ConstCompilationTaskConstVisitor::~ConstCompilationTaskConstVisitor() = default;
    }
}

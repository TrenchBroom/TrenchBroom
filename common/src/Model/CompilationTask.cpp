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

#include "CompilationTask.h"

namespace TrenchBroom {
    namespace Model {
        CompilationTask::CompilationTask() {}
    
        CompilationTask::~CompilationTask() {}

        CompilationTask* CompilationTask::clone() const {
            return doClone();
        }

        CompilationExportMap::CompilationExportMap(const String& targetSpec) :
        m_targetSpec(targetSpec) {}
        
        void CompilationExportMap::accept(CompilationTaskVisitor& visitor) {
            visitor.visit(this);
        }
        
        void CompilationExportMap::accept(ConstCompilationTaskVisitor& visitor) const {
            visitor.visit(this);
        }
        
        void CompilationExportMap::accept(const CompilationTaskConstVisitor& visitor) {
            visitor.visit(this);
        }
        
        void CompilationExportMap::accept(const ConstCompilationTaskConstVisitor& visitor) const {
            visitor.visit(this);
        }
        
        const String& CompilationExportMap::targetSpec() const {
            return m_targetSpec;
        }
        
        void CompilationExportMap::setTargetSpec(const String& targetSpec) {
            m_targetSpec = targetSpec;
            taskDidChange();
        }
    
        CompilationTask* CompilationExportMap::doClone() const {
            return new CompilationExportMap(m_targetSpec);
        }

        CompilationCopyFiles::CompilationCopyFiles(const String& sourceSpec, const String& targetSpec) :
        CompilationTask(),
        m_sourceSpec(sourceSpec),
        m_targetSpec(targetSpec) {}
        
        void CompilationCopyFiles::accept(CompilationTaskVisitor& visitor) {
            visitor.visit(this);
        }
        
        void CompilationCopyFiles::accept(ConstCompilationTaskVisitor& visitor) const {
            visitor.visit(this);
        }
        
        void CompilationCopyFiles::accept(const CompilationTaskConstVisitor& visitor) {
            visitor.visit(this);
        }
        
        void CompilationCopyFiles::accept(const ConstCompilationTaskConstVisitor& visitor) const {
            visitor.visit(this);
        }

        const String& CompilationCopyFiles::sourceSpec() const {
            return m_sourceSpec;
        }
        
        const String& CompilationCopyFiles::targetSpec() const {
            return m_targetSpec;
        }

        void CompilationCopyFiles::setSourceSpec(const String& sourceSpec) {
            m_sourceSpec = sourceSpec;
            taskDidChange();
        }
        
        void CompilationCopyFiles::setTargetSpec(const String& targetSpec) {
            m_targetSpec = targetSpec;
            taskDidChange();
        }

        CompilationTask* CompilationCopyFiles::doClone() const {
            return new CompilationCopyFiles(m_sourceSpec, m_targetSpec);
        }

        CompilationRunTool::CompilationRunTool(const String& toolSpec, const String& parameterSpec) :
        CompilationTask(),
        m_toolSpec(toolSpec),
        m_parameterSpec(parameterSpec) {}

        void CompilationRunTool::accept(CompilationTaskVisitor& visitor) {
            visitor.visit(this);
        }
        
        void CompilationRunTool::accept(ConstCompilationTaskVisitor& visitor) const {
            visitor.visit(this);
        }
        
        void CompilationRunTool::accept(const CompilationTaskConstVisitor& visitor) {
            visitor.visit(this);
        }
        
        void CompilationRunTool::accept(const ConstCompilationTaskConstVisitor& visitor) const {
            visitor.visit(this);
        }
        
        const String& CompilationRunTool::toolSpec() const {
            return m_toolSpec;
        }
        
        const String& CompilationRunTool::parameterSpec() const {
            return m_parameterSpec;
        }

        void CompilationRunTool::setToolSpec(const String& toolSpec) {
            m_toolSpec = toolSpec;
            taskDidChange();
        }
        
        void CompilationRunTool::setParameterSpec(const String& parameterSpec) {
            m_parameterSpec = parameterSpec;
            taskDidChange();
        }

        CompilationTask* CompilationRunTool::doClone() const {
            return new CompilationRunTool(m_toolSpec, m_parameterSpec);
        }

        CompilationTaskVisitor::~CompilationTaskVisitor() {}
        ConstCompilationTaskVisitor::~ConstCompilationTaskVisitor() {}
        CompilationTaskConstVisitor::~CompilationTaskConstVisitor() {}
        ConstCompilationTaskConstVisitor::~ConstCompilationTaskConstVisitor() {}
    }
}

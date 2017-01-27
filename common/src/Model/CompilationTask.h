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

#ifndef CompilationTask_h
#define CompilationTask_h

#include "Notifier.h"
#include "StringUtils.h"
#include "IO/Path.h"

#include <vector>

class wxTimer;
class wxTimerEvent;

namespace TrenchBroom {
    namespace Model {
        class CompilationContext;
        class CompilationTaskVisitor;
        class ConstCompilationTaskVisitor;
        class CompilationTaskConstVisitor;
        class ConstCompilationTaskConstVisitor;
        
        class CompilationTask {
        public:
            typedef std::vector<CompilationTask*> List;
            
            Notifier0 taskWillBeRemoved;
            Notifier0 taskDidChange;
        protected:
            CompilationTask();
        public:
            virtual ~CompilationTask();

            virtual void accept(CompilationTaskVisitor& visitor) = 0;
            virtual void accept(ConstCompilationTaskVisitor& visitor) const = 0;
            virtual void accept(const CompilationTaskConstVisitor& visitor) = 0;
            virtual void accept(const ConstCompilationTaskConstVisitor& visitor) const = 0;
            
            CompilationTask* clone() const;
        private:
            virtual CompilationTask* doClone() const = 0;
        private:
            CompilationTask(const CompilationTask& other);
            CompilationTask& operator=(const CompilationTask& other);
        };
        
        class CompilationExportMap : public CompilationTask {
        private:
            String m_targetSpec;
        public:
            CompilationExportMap(const String& targetSpec);
            
            void accept(CompilationTaskVisitor& visitor);
            void accept(ConstCompilationTaskVisitor& visitor) const;
            void accept(const CompilationTaskConstVisitor& visitor);
            void accept(const ConstCompilationTaskConstVisitor& visitor) const;
            
            const String& targetSpec() const;
            
            void setTargetSpec(const String& targetSpec);
        private:
            CompilationTask* doClone() const;
        private:
            CompilationExportMap(const CompilationExportMap& other);
            CompilationExportMap& operator=(const CompilationExportMap& other);
        };
        
        class CompilationCopyFiles : public CompilationTask {
        private:
            String m_sourceSpec;
            String m_targetSpec;
        public:
            CompilationCopyFiles(const String& sourceSpec, const String& targetSpec);
            
            void accept(CompilationTaskVisitor& visitor);
            void accept(ConstCompilationTaskVisitor& visitor) const;
            void accept(const CompilationTaskConstVisitor& visitor);
            void accept(const ConstCompilationTaskConstVisitor& visitor) const;

            const String& sourceSpec() const;
            const String& targetSpec() const;
            
            void setSourceSpec(const String& sourceSpec);
            void setTargetSpec(const String& targetSpec);
        private:
            CompilationTask* doClone() const;
        private:
            CompilationCopyFiles(const CompilationCopyFiles& other);
            CompilationCopyFiles& operator=(const CompilationCopyFiles& other);
        };

        class CompilationRunTool : public CompilationTask {
        private:
            String m_toolSpec;
            String m_parameterSpec;
        public:
            CompilationRunTool(const String& toolSpec, const String& parameterSpec);
            
            void accept(CompilationTaskVisitor& visitor);
            void accept(ConstCompilationTaskVisitor& visitor) const;
            void accept(const CompilationTaskConstVisitor& visitor);
            void accept(const ConstCompilationTaskConstVisitor& visitor) const;

            const String& toolSpec() const;
            const String& parameterSpec() const;
            
            void setToolSpec(const String& toolSpec);
            void setParameterSpec(const String& parameterSpec);
        private:
            CompilationTask* doClone() const;
        private:
            CompilationRunTool(const CompilationRunTool& other);
            CompilationRunTool& operator=(const CompilationRunTool& other);
        };
        
        class CompilationTaskVisitor {
        public:
            virtual ~CompilationTaskVisitor();
            
            virtual void visit(CompilationExportMap* task) = 0;
            virtual void visit(CompilationCopyFiles* task) = 0;
            virtual void visit(CompilationRunTool* task) = 0;
        };
        
        class ConstCompilationTaskVisitor {
        public:
            virtual ~ConstCompilationTaskVisitor();
            
            virtual void visit(const CompilationExportMap* task) = 0;
            virtual void visit(const CompilationCopyFiles* task) = 0;
            virtual void visit(const CompilationRunTool* task) = 0;
        };
        
        class CompilationTaskConstVisitor {
        public:
            virtual ~CompilationTaskConstVisitor();
            
            virtual void visit(CompilationExportMap* task) const = 0;
            virtual void visit(CompilationCopyFiles* task) const = 0;
            virtual void visit(CompilationRunTool* task) const = 0;
        };
        
        class ConstCompilationTaskConstVisitor {
        public:
            virtual ~ConstCompilationTaskConstVisitor();
            
            virtual void visit(const CompilationExportMap* task) const = 0;
            virtual void visit(const CompilationCopyFiles* task) const = 0;
            virtual void visit(const CompilationRunTool* task) const = 0;
        };
    }
}

#endif /* CompilationTask_h */

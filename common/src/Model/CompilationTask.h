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

#include "Macros.h"
#include "Notifier.h"

#include <string>

namespace TrenchBroom {
    namespace Model {
        class CompilationTaskConstVisitor;
        class CompilationTaskVisitor;
        class ConstCompilationTaskConstVisitor;
        class ConstCompilationTaskVisitor;

        class CompilationTask {
        public:
            Notifier<> taskWillBeRemoved;
            Notifier<> taskDidChange;
        protected:
            CompilationTask();
        public:
            virtual ~CompilationTask();

            virtual void accept(CompilationTaskVisitor& visitor) = 0;
            virtual void accept(ConstCompilationTaskVisitor& visitor) const = 0;
            virtual void accept(const CompilationTaskConstVisitor& visitor) = 0;
            virtual void accept(const ConstCompilationTaskConstVisitor& visitor) const = 0;

            virtual CompilationTask* clone() const = 0;

            deleteCopyAndMove(CompilationTask)
        };

        class CompilationExportMap : public CompilationTask {
        private:
            std::string m_targetSpec;
        public:
            explicit CompilationExportMap(const std::string& targetSpec);

            void accept(CompilationTaskVisitor& visitor) override;
            void accept(ConstCompilationTaskVisitor& visitor) const override;
            void accept(const CompilationTaskConstVisitor& visitor) override;
            void accept(const ConstCompilationTaskConstVisitor& visitor) const override;

            const std::string& targetSpec() const;

            void setTargetSpec(const std::string& targetSpec);

            CompilationExportMap* clone() const override;

            deleteCopyAndMove(CompilationExportMap)
        };

        class CompilationCopyFiles : public CompilationTask {
        private:
            std::string m_sourceSpec;
            std::string m_targetSpec;
        public:
            CompilationCopyFiles(const std::string& sourceSpec, const std::string& targetSpec);

            void accept(CompilationTaskVisitor& visitor) override;
            void accept(ConstCompilationTaskVisitor& visitor) const override;
            void accept(const CompilationTaskConstVisitor& visitor) override;
            void accept(const ConstCompilationTaskConstVisitor& visitor) const override;

            const std::string& sourceSpec() const;
            const std::string& targetSpec() const;

            void setSourceSpec(const std::string& sourceSpec);
            void setTargetSpec(const std::string& targetSpec);

            CompilationCopyFiles* clone() const override;

            deleteCopyAndMove(CompilationCopyFiles)
        };

        class CompilationRunTool : public CompilationTask {
        private:
            std::string m_toolSpec;
            std::string m_parameterSpec;
        public:
            CompilationRunTool(const std::string& toolSpec, const std::string& parameterSpec);

            void accept(CompilationTaskVisitor& visitor) override;
            void accept(ConstCompilationTaskVisitor& visitor) const override;
            void accept(const CompilationTaskConstVisitor& visitor) override;
            void accept(const ConstCompilationTaskConstVisitor& visitor) const override;

            const std::string& toolSpec() const;
            const std::string& parameterSpec() const;

            void setToolSpec(const std::string& toolSpec);
            void setParameterSpec(const std::string& parameterSpec);

            CompilationRunTool* clone() const override;

            deleteCopyAndMove(CompilationRunTool)
        };

        class CompilationTaskVisitor {
        public:
            virtual ~CompilationTaskVisitor();

            virtual void visit(CompilationExportMap& task) = 0;
            virtual void visit(CompilationCopyFiles& task) = 0;
            virtual void visit(CompilationRunTool& task) = 0;
        };

        class ConstCompilationTaskVisitor {
        public:
            virtual ~ConstCompilationTaskVisitor();

            virtual void visit(const CompilationExportMap& task) = 0;
            virtual void visit(const CompilationCopyFiles& task) = 0;
            virtual void visit(const CompilationRunTool& task) = 0;
        };

        class CompilationTaskConstVisitor {
        public:
            virtual ~CompilationTaskConstVisitor();

            virtual void visit(CompilationExportMap& task) const = 0;
            virtual void visit(CompilationCopyFiles& task) const = 0;
            virtual void visit(CompilationRunTool& task) const = 0;
        };

        class ConstCompilationTaskConstVisitor {
        public:
            virtual ~ConstCompilationTaskConstVisitor();

            virtual void visit(const CompilationExportMap& task) const = 0;
            virtual void visit(const CompilationCopyFiles& task) const = 0;
            virtual void visit(const CompilationRunTool& task) const = 0;
        };
    }
}

#endif /* CompilationTask_h */

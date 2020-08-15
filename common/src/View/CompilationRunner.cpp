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

#include "CompilationRunner.h"

#include "Exceptions.h"
#include "IO/DiskIO.h"
#include "IO/FileMatcher.h"
#include "IO/Path.h"
#include "Model/CompilationProfile.h"
#include "Model/CompilationTask.h"
#include "Model/ExportFormat.h"
#include "View/CompilationContext.h"
#include "View/CompilationVariables.h"
#include "View/MapDocument.h"

#include <string>

#include <QtGlobal>
#include <QProcess>

namespace TrenchBroom {
    namespace View {
        CompilationTaskRunner::CompilationTaskRunner(CompilationContext& context) :
        m_context(context) {}

        CompilationTaskRunner::~CompilationTaskRunner() = default;

        void CompilationTaskRunner::execute() {
            doExecute();
        }

        void CompilationTaskRunner::terminate() {
            doTerminate();
        }

        std::string CompilationTaskRunner::interpolate(const std::string& spec) {
            try {
                return m_context.interpolate(spec);
            } catch (const Exception& e) {
                m_context << "#### Could not interpolate expression '" << spec << "': " << e.what() << "\n";
                throw;
            }
        }

        CompilationExportMapTaskRunner::CompilationExportMapTaskRunner(CompilationContext& context, const Model::CompilationExportMap& task) :
        CompilationTaskRunner(context),
        m_task(task.clone()) {}

        CompilationExportMapTaskRunner::~CompilationExportMapTaskRunner() = default;

        void CompilationExportMapTaskRunner::doExecute() {
            emit start();

            try {
                const IO::Path targetPath(interpolate(m_task->targetSpec()));
                try {
                    m_context << "#### Exporting map file '" << targetPath.asString() << "'\n";

                    if (!m_context.test()) {
                        const IO::Path directoryPath = targetPath.deleteLastComponent();
                        if (!IO::Disk::directoryExists(directoryPath)) {
                            IO::Disk::createDirectory(directoryPath);
                        }

                        const auto document = m_context.document();
                        document->exportDocumentAs(Model::ExportFormat::Map, targetPath);
                    }
                    emit end();
                } catch (const Exception& e) {
                    m_context << "#### Could not export map file '" << targetPath.asString() << "': " << e.what() << "\n";
                    throw;
                }
            } catch (const Exception&) {
                emit error();
            }

        }

        void CompilationExportMapTaskRunner::doTerminate() {}

        CompilationCopyFilesTaskRunner::CompilationCopyFilesTaskRunner(CompilationContext& context, const Model::CompilationCopyFiles& task) :
        CompilationTaskRunner(context),
        m_task(task.clone()) {}

        CompilationCopyFilesTaskRunner::~CompilationCopyFilesTaskRunner() = default;

        void CompilationCopyFilesTaskRunner::doExecute() {
            emit start();

            try {
                const IO::Path sourcePath(interpolate(m_task->sourceSpec()));
                const IO::Path targetPath(interpolate(m_task->targetSpec()));

                const IO::Path sourceDirPath = sourcePath.deleteLastComponent();
                const std::string sourcePattern = sourcePath.lastComponent().asString();

                try {
                    m_context << "#### Copying '" << sourcePath.asString() << "' to '" << targetPath.asString() << "'\n";
                    if (!m_context.test()) {
                        IO::Disk::copyFiles(sourceDirPath, IO::FileNameMatcher(sourcePattern), targetPath, true);
                    }
                    emit end();
                } catch (const Exception& e) {
                    m_context << "#### Could not copy '" << sourcePath.asString() << "' to '" << targetPath.asString() << "': " << e.what() << "\n";
                    throw;
                }
            } catch (const Exception&) {
                emit error();
            }
        }

        void CompilationCopyFilesTaskRunner::doTerminate() {}

        CompilationRunToolTaskRunner::CompilationRunToolTaskRunner(CompilationContext& context, const Model::CompilationRunTool& task) :
        CompilationTaskRunner(context),
        m_task(task.clone()),
        m_process(nullptr),
        m_terminated(false) {}

        CompilationRunToolTaskRunner::~CompilationRunToolTaskRunner() = default;

        void CompilationRunToolTaskRunner::doExecute() {
            startProcess();
        }

        void CompilationRunToolTaskRunner::doTerminate() {
            if (m_process != nullptr) {
                disconnect(m_process, &QProcess::errorOccurred, this, &CompilationRunToolTaskRunner::processErrorOccurred);
                disconnect(m_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, &CompilationRunToolTaskRunner::processFinished);
                m_process->kill();
                m_context << "\n\n#### Terminated\n";
            }
        }

        void CompilationRunToolTaskRunner::startProcess() {
            assert(m_process == nullptr);

            emit start();
            try {
                const auto workDir = m_context.variableValue(CompilationVariableNames::WORK_DIR_PATH);
                const auto cmd = this->cmd();

                m_context << "#### Executing '" << cmd << "'\n";

                if (!m_context.test()) {
                    m_process = new QProcess(this);
                    connect(m_process, &QProcess::errorOccurred, this, &CompilationRunToolTaskRunner::processErrorOccurred);
                    connect(m_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, &CompilationRunToolTaskRunner::processFinished);
                    connect(m_process, &QProcess::readyReadStandardError, this, &CompilationRunToolTaskRunner::processReadyReadStandardError);
                    connect(m_process, &QProcess::readyReadStandardOutput, this, &CompilationRunToolTaskRunner::processReadyReadStandardOutput);

                    m_process->setWorkingDirectory(QString::fromStdString(workDir));
                    m_process->start(QString::fromStdString(cmd));
                    if (!m_process->waitForStarted()) {
                        emit error();
                    }
                } else {
                    emit end();
                }
            } catch (const Exception&) {
                emit error();
            }
        }

        std::string CompilationRunToolTaskRunner::cmd() {
            const auto toolPath = IO::Path(interpolate(m_task->toolSpec()));
            const auto parameters = interpolate(m_task->parameterSpec());
            if (parameters.empty()) {
                return std::string("\"") + toolPath.asString() + "\"";
            } else if (toolPath.isEmpty()) {
                return "";
            } else {
                return std::string("\"") + toolPath.asString() + "\" " + parameters;
            }
        }

        void CompilationRunToolTaskRunner::processErrorOccurred(const QProcess::ProcessError processError) {
            m_context << "#### Error " << processError << " occurred when communicating with process\n\n";
            emit error();
        }

        void CompilationRunToolTaskRunner::processFinished(const int exitCode, const QProcess::ExitStatus /* exitStatus */) {
            m_context << "#### Finished with exit status " << exitCode << "\n\n";
            emit end();
        }

        void CompilationRunToolTaskRunner::processReadyReadStandardError() {
            if (m_process != nullptr) {
                const QByteArray bytes = m_process->readAllStandardError();
                m_context << bytes.toStdString();
            }
        }

        void CompilationRunToolTaskRunner::processReadyReadStandardOutput() {
            if (m_process != nullptr) {
                const QByteArray bytes = m_process->readAllStandardOutput();
                m_context << bytes.toStdString();
            }
        }

        CompilationRunner::CompilationRunner(std::unique_ptr<CompilationContext> context, const Model::CompilationProfile* profile) :
        m_context(std::move(context)),
        m_taskRunners(createTaskRunners(*m_context, profile)),
        m_currentTask(std::end(m_taskRunners)) {}

        CompilationRunner::~CompilationRunner() = default;

        class CompilationRunner::CreateTaskRunnerVisitor : public Model::ConstCompilationTaskVisitor {
        private:
            CompilationContext& m_context;
            TaskRunnerList m_runners;
        public:
            explicit CreateTaskRunnerVisitor(CompilationContext& context) :
            m_context(context) {}

            TaskRunnerList runners() {
                return std::move(m_runners);
            }

            void visit(const Model::CompilationExportMap& task) override {
                if (task.enabled()) {
                    appendRunner(std::make_unique<CompilationExportMapTaskRunner>(m_context, task));
                }
            }

            void visit(const Model::CompilationCopyFiles& task) override {
                if (task.enabled()) {
                    appendRunner(std::make_unique<CompilationCopyFilesTaskRunner>(m_context, task));
                }
            }

            void visit(const Model::CompilationRunTool& task) override {
                if (task.enabled()) {
                    appendRunner(std::make_unique<CompilationRunToolTaskRunner>(m_context, task));
                }
            }

        private:
            void appendRunner(std::unique_ptr<CompilationTaskRunner> runner) {
                m_runners.emplace_back(std::move(runner));
            }
        };

        CompilationRunner::TaskRunnerList CompilationRunner::createTaskRunners(CompilationContext& context, const Model::CompilationProfile* profile) {
            CreateTaskRunnerVisitor visitor(context);
            profile->accept(visitor);
            return visitor.runners();
        }

        void CompilationRunner::execute() {
            assert(!running());

            m_currentTask = std::begin(m_taskRunners);
            if (m_currentTask == std::end(m_taskRunners)) {
                emit compilationEnded();
                return;
            }
            bindEvents(m_currentTask->get());

            emit compilationStarted();
            m_currentTask->get()->execute();
        }

        void CompilationRunner::terminate() {
            assert(running());
            unbindEvents(m_currentTask->get());
            m_currentTask->get()->terminate();
            m_currentTask = std::end(m_taskRunners);

            emit compilationEnded();
        }

        bool CompilationRunner::running() const {
            return m_currentTask != std::end(m_taskRunners);
        }

        void CompilationRunner::bindEvents(CompilationTaskRunner* runner) {
            connect(runner, &CompilationTaskRunner::error, this, &CompilationRunner::taskError);
            connect(runner, &CompilationTaskRunner::end, this, &CompilationRunner::taskEnd);
        }

        void CompilationRunner::unbindEvents(CompilationTaskRunner* runner) {
            runner->disconnect(this);
        }

        void CompilationRunner::taskError() {
            if (running()) {
                unbindEvents(m_currentTask->get());
                m_currentTask = std::end(m_taskRunners);
                emit compilationEnded();
            }
        }

        void CompilationRunner::taskEnd() {
            if (running()) {
                unbindEvents(m_currentTask->get());
                ++m_currentTask;
                if (m_currentTask != std::end(m_taskRunners)) {
                    bindEvents(m_currentTask->get());
                    m_currentTask->get()->execute();
                } else {
                    emit compilationEnded();
                }
            }
        }
    }
}

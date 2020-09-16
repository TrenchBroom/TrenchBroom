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

#include "CompilationRun.h"

#include "Ensure.h"
#include "EL/EvaluationContext.h"
#include "EL/Interpolator.h"
#include "Model/CompilationProfile.h"
#include "Model/Game.h"
#include "View/CompilationContext.h"
#include "View/CompilationRunner.h"
#include "View/CompilationVariables.h"
#include "View/MapDocument.h"
#include "View/TextOutputAdapter.h"

#include <memory>
#include <string>

namespace TrenchBroom {
    namespace View {
        CompilationRun::CompilationRun() :
        m_currentRun(nullptr) {}

        CompilationRun::~CompilationRun() {
            if (running()) {
                m_currentRun->terminate();
            }
        }

        bool CompilationRun::running() const {
            return doIsRunning();
        }

        void CompilationRun::run(const Model::CompilationProfile* profile, std::shared_ptr<MapDocument> document, QTextEdit* currentOutput) {
            run(profile, std::move(document), currentOutput, false);
        }

        void CompilationRun::test(const Model::CompilationProfile* profile, std::shared_ptr<MapDocument> document, QTextEdit* currentOutput) {
            run(profile, std::move(document), currentOutput, true);
        }

        void CompilationRun::terminate() {
            if (doIsRunning()) {
                m_currentRun->terminate();
            }
        }

        bool CompilationRun::doIsRunning() const {
            return m_currentRun != nullptr && m_currentRun->running();
        }

        void CompilationRun::run(const Model::CompilationProfile* profile, std::shared_ptr<MapDocument> document, QTextEdit* currentOutput, const bool test) {
            ensure(profile != nullptr, "profile is null");
            ensure(profile->taskCount() > 0, "profile has no tasks");
            ensure(document != nullptr, "document is null");
            ensure(currentOutput != nullptr, "currentOutput is null");

            assert(!doIsRunning());
            cleanup();

            CompilationVariables variables(document, buildWorkDir(profile, document));

            auto compilationContext = std::make_unique<CompilationContext>(document, variables, TextOutputAdapter(currentOutput), test);
            m_currentRun = new CompilationRunner(std::move(compilationContext), profile, this);
            connect(m_currentRun, &CompilationRunner::compilationStarted, this, &CompilationRun::compilationStarted);
            connect(m_currentRun, &CompilationRunner::compilationEnded, this, [&]() { cleanup(); emit compilationEnded(); });
            m_currentRun->execute();
        }

        std::string CompilationRun::buildWorkDir(const Model::CompilationProfile* profile, std::shared_ptr<MapDocument> document) {
            try {
                return EL::interpolate(profile->workDirSpec(),
                                       EL::EvaluationContext(CompilationWorkDirVariables(document)));
            } catch (const Exception&) {
                return "";
            }
        }

        void CompilationRun::cleanup() {
            if (m_currentRun != nullptr) {
                // It's not safe to delete a CompilationRunner during execution of one of its signals, so use deleteLater()
                m_currentRun->deleteLater();
                m_currentRun = nullptr;
            }
        }
    }
}

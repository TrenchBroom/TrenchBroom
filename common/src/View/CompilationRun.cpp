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

#include "EL/Interpolator.h"
#include "Model/CompilationProfile.h"
#include "Model/Game.h"
#include "View/CompilationContext.h"
#include "View/CompilationRunner.h"
#include "View/CompilationVariables.h"
#include "View/MapDocument.h"
#include "View/TextCtrlOutputAdapter.h"

#include <wx/textctrl.h>
#include <wx/thread.h>

namespace TrenchBroom {
    namespace View {
        CompilationRun::CompilationRun() :
        m_currentRun(nullptr) {}

        CompilationRun::~CompilationRun() {
            if (running())
                m_currentRun->terminate();
            delete m_currentRun;
        }

        bool CompilationRun::running() const {
            wxCriticalSectionLocker lock(m_currentRunSection);
            return doIsRunning();
        }

        void CompilationRun::run(const Model::CompilationProfile* profile, MapDocumentSPtr document, wxTextCtrl* currentOutput) {
            run(profile, document, currentOutput, false);
        }

        void CompilationRun::test(const Model::CompilationProfile* profile, MapDocumentSPtr document, wxTextCtrl* currentOutput) {
            run(profile, document, currentOutput, true);
        }

        void CompilationRun::terminate() {
            wxCriticalSectionLocker lock(m_currentRunSection);
            if (doIsRunning())
                m_currentRun->terminate();
        }

        bool CompilationRun::doIsRunning() const {
            return m_currentRun != nullptr && m_currentRun->running();
        }

        void CompilationRun::run(const Model::CompilationProfile* profile, MapDocumentSPtr document, wxTextCtrl* currentOutput, const bool test) {
            ensure(profile != nullptr, "profile is null");
            ensure(document.get() != nullptr, "document is null");
            ensure(currentOutput != nullptr, "currentOutput is null");

            wxCriticalSectionLocker lock(m_currentRunSection);
            assert(!doIsRunning());
            if (m_currentRun != nullptr) {
                delete m_currentRun;
                m_currentRun = nullptr;
            }

            CompilationVariables variables(document, buildWorkDir(profile, document));

            m_currentRun = new CompilationRunner(new CompilationContext(document, variables, TextCtrlOutputAdapter(currentOutput), test), profile);
            m_currentRun->Bind(wxEVT_COMPILATION_START, &CompilationRun::OnCompilationStart, this);
            m_currentRun->Bind(wxEVT_COMPILATION_END, &CompilationRun::OnCompilationStart, this);
            m_currentRun->execute();
        }

        String CompilationRun::buildWorkDir(const Model::CompilationProfile* profile, MapDocumentSPtr document) {
            return EL::interpolate(profile->workDirSpec(), EL::EvaluationContext(CompilationWorkDirVariables(document)));
        }

        void CompilationRun::OnCompilationStart(wxEvent& event) {
            ProcessEvent(event);
        }

        void CompilationRun::OnCompilationEnd(wxEvent& event) {
            cleanup();
            ProcessEvent(event);
        }

        void CompilationRun::cleanup() {
            delete m_currentRun;
            m_currentRun = nullptr;
        }
    }
}

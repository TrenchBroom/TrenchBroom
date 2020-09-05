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

#include "IssueBrowser.h"

#include "Model/Issue.h"
#include "Model/IssueGenerator.h"
#include "Model/WorldNode.h"
#include "View/FlagsPopupEditor.h"
#include "View/IssueBrowserView.h"
#include "View/MapDocument.h"

#include <kdl/memory_utils.h>

#include <QList>
#include <QStringList>
#include <QCheckBox>
#include <QVBoxLayout>

namespace TrenchBroom {
    namespace View {
        IssueBrowser::IssueBrowser(std::weak_ptr<MapDocument> document, QWidget* parent) :
        TabBookPage(parent),
        m_document(document),
        m_view(new IssueBrowserView(m_document)),
        m_showHiddenIssuesCheckBox(nullptr),
        m_filterEditor(nullptr) {
            auto* sizer = new QVBoxLayout();
            sizer->setContentsMargins(0, 0, 0, 0);
            sizer->addWidget(m_view);
            setLayout(sizer);

            bindObservers();
        }

        IssueBrowser::~IssueBrowser() {
            unbindObservers();
        }

        QWidget* IssueBrowser::createTabBarPage(QWidget* parent) {


            auto* barPage = new QWidget(parent);
            m_showHiddenIssuesCheckBox = new QCheckBox("Show hidden issues");
            connect(m_showHiddenIssuesCheckBox, &QCheckBox::stateChanged, this, &IssueBrowser::showHiddenIssuesChanged);

            m_filterEditor = new FlagsPopupEditor(1, nullptr, "Filter", false);
            connect(m_filterEditor, &FlagsPopupEditor::flagChanged, this, &IssueBrowser::filterChanged);

            auto* barPageSizer = new QHBoxLayout();
            barPageSizer->setContentsMargins(0, 0, 0, 0);
            barPageSizer->addWidget(m_showHiddenIssuesCheckBox, 0, Qt::AlignVCenter);
            barPageSizer->addWidget(m_filterEditor, 0, Qt::AlignVCenter);
            barPage->setLayout(barPageSizer);

            return barPage;
        }

        void IssueBrowser::bindObservers() {
            auto document = kdl::mem_lock(m_document);
            document->documentWasSavedNotifier.addObserver(this, &IssueBrowser::documentWasSaved);
            document->documentWasNewedNotifier.addObserver(this, &IssueBrowser::documentWasNewedOrLoaded);
            document->documentWasLoadedNotifier.addObserver(this, &IssueBrowser::documentWasNewedOrLoaded);
            document->nodesWereAddedNotifier.addObserver(this, &IssueBrowser::nodesWereAdded);
            document->nodesWereRemovedNotifier.addObserver(this, &IssueBrowser::nodesWereRemoved);
            document->nodesDidChangeNotifier.addObserver(this, &IssueBrowser::nodesDidChange);
            document->brushFacesDidChangeNotifier.addObserver(this, &IssueBrowser::brushFacesDidChange);
        }

        void IssueBrowser::unbindObservers() {
            if (!kdl::mem_expired(m_document)) {
                auto document = kdl::mem_lock(m_document);
                document->documentWasSavedNotifier.removeObserver(this, &IssueBrowser::documentWasSaved);
                document->documentWasNewedNotifier.removeObserver(this, &IssueBrowser::documentWasNewedOrLoaded);
                document->documentWasLoadedNotifier.removeObserver(this, &IssueBrowser::documentWasNewedOrLoaded);
                document->nodesWereAddedNotifier.removeObserver(this, &IssueBrowser::nodesWereAdded);
                document->nodesWereRemovedNotifier.removeObserver(this, &IssueBrowser::nodesWereRemoved);
                document->nodesDidChangeNotifier.removeObserver(this, &IssueBrowser::nodesDidChange);
                document->brushFacesDidChangeNotifier.removeObserver(this, &IssueBrowser::brushFacesDidChange);
            }
        }

        void IssueBrowser::documentWasNewedOrLoaded(MapDocument*) {
			updateFilterFlags();
            m_view->reload();
        }

        void IssueBrowser::documentWasSaved(MapDocument*) {
            m_view->update();
        }

        void IssueBrowser::nodesWereAdded(const std::vector<Model::Node*>&) {
            m_view->reload();
        }

        void IssueBrowser::nodesWereRemoved(const std::vector<Model::Node*>&) {
            m_view->reload();
        }

        void IssueBrowser::nodesDidChange(const std::vector<Model::Node*>&) {
            m_view->reload();
        }

        void IssueBrowser::brushFacesDidChange(const std::vector<Model::BrushFaceHandle>&) {
            m_view->reload();
        }

        void IssueBrowser::issueIgnoreChanged(Model::Issue*) {
            m_view->update();
        }

        void IssueBrowser::updateFilterFlags() {
            auto document = kdl::mem_lock(m_document);
            const Model::WorldNode* world = document->world();
            const std::vector<Model::IssueGenerator*>& generators = world->registeredIssueGenerators();

            QList<int> flags;
            QStringList labels;

            for (const Model::IssueGenerator* generator : generators) {
                const Model::IssueType flag = generator->type();
                const std::string& description = generator->description();

                flags.push_back(flag);
                labels.push_back(QString::fromStdString(description));
            }

            m_filterEditor->setFlags(flags, labels);
            m_view->setHiddenGenerators(0);
            m_filterEditor->setFlagValue(~0);
        }

        void IssueBrowser::showHiddenIssuesChanged() {
            m_view->setShowHiddenIssues(m_showHiddenIssuesCheckBox->isChecked());
        }

        void IssueBrowser::filterChanged(const size_t /* index */, const int /* value */, const int setFlag, const int /* mixedFlag */) {
            m_view->setHiddenGenerators(~setFlag);
        }
    }
}

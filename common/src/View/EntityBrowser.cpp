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

#include "EntityBrowser.h"

#include "PreferenceManager.h"
#include "Preferences.h"
#include "Assets/EntityDefinition.h"
#include "Assets/EntityDefinitionManager.h"
#include "View/EntityBrowserView.h"
#include "View/ViewConstants.h"
#include "View/MapDocument.h"
#include "View/QtUtils.h"

#include <kdl/memory_utils.h>

#include <QtGlobal>
#include <QPushButton>
#include <QComboBox>
#include <QLineEdit>
#include <QScrollBar>
#include <QHBoxLayout>

// for use in QVariant
Q_DECLARE_METATYPE(TrenchBroom::Assets::EntityDefinitionSortOrder)

namespace TrenchBroom {
    namespace View {
        EntityBrowser::EntityBrowser(std::weak_ptr<MapDocument> document, GLContextManager& contextManager, QWidget* parent) :
        QWidget(parent),
        m_document(std::move(document)),
        m_sortOrderChoice(nullptr),
        m_groupButton(nullptr),
        m_usedButton(nullptr),
        m_filterBox(nullptr),
        m_scrollBar(nullptr),
        m_view(nullptr) {
            createGui(contextManager);
            connectObservers();
        }

        void EntityBrowser::reload() {
            if (m_view != nullptr) {
                m_view->invalidate();
                m_view->update();
            }
        }

        void EntityBrowser::createGui(GLContextManager& contextManager) {
            m_scrollBar = new QScrollBar(Qt::Vertical);

            auto document = kdl::mem_lock(m_document);

            m_view = new EntityBrowserView(
                m_scrollBar,
                contextManager,
                document->entityDefinitionManager(),
                document->entityModelManager(),
                *document);

            auto* browserPanelSizer = new QHBoxLayout();
            browserPanelSizer->setContentsMargins(0, 0, 0, 0);
            browserPanelSizer->setSpacing(0);
            browserPanelSizer->addWidget(m_view, 1);
            browserPanelSizer->addWidget(m_scrollBar, 0);

            auto* browserPanel = new QWidget(this);
            browserPanel->setLayout(browserPanelSizer);

            m_sortOrderChoice = new QComboBox();
            m_sortOrderChoice->addItem(tr("Name"), QVariant::fromValue(Assets::EntityDefinitionSortOrder::Name));
            m_sortOrderChoice->addItem(tr("Usage"), QVariant::fromValue(Assets::EntityDefinitionSortOrder::Usage));
            m_sortOrderChoice->setCurrentIndex(0);
            m_sortOrderChoice->setToolTip(tr("Select ordering criterion"));
            connect(m_sortOrderChoice, QOverload<int>::of(&QComboBox::activated), this, [=](int index){
                auto sortOrder = static_cast<Assets::EntityDefinitionSortOrder>(m_sortOrderChoice->itemData(index).toInt());
                m_view->setSortOrder(sortOrder);
            });

            m_groupButton = new QPushButton(tr("Group"));
            m_groupButton->setToolTip(tr("Group entity definitions by category"));
            m_groupButton->setCheckable(true);
            connect(m_groupButton, &QAbstractButton::clicked, this, [=](){
                m_view->setGroup(m_groupButton->isChecked());
            });

            m_usedButton = new QPushButton(tr("Used"));
            m_usedButton->setToolTip(tr("Only show entity definitions currently in use"));
            m_usedButton->setCheckable(true);
            connect(m_usedButton, &QAbstractButton::clicked, this, [=](){
                m_view->setHideUnused(m_usedButton->isChecked());
            });

            m_filterBox = createSearchBox();
            connect(m_filterBox, &QLineEdit::textEdited, this, [=](){
                m_view->setFilterText(m_filterBox->text().toStdString());
            });

            auto* controlSizer = new QHBoxLayout();
            controlSizer->setContentsMargins(LayoutConstants::NarrowHMargin, LayoutConstants::NarrowVMargin, LayoutConstants::NarrowHMargin, LayoutConstants::NarrowVMargin);
            controlSizer->setSpacing(LayoutConstants::NarrowHMargin);
            controlSizer->addWidget(m_sortOrderChoice, 0);
            controlSizer->addWidget(m_groupButton, 0);
            controlSizer->addWidget(m_usedButton, 0);
            controlSizer->addWidget(m_filterBox, 1);

            auto* outerSizer = new QVBoxLayout();
            outerSizer->setContentsMargins(0, 0, 0, 0);
            outerSizer->setSpacing(0);
            outerSizer->addWidget(browserPanel, 1);
            outerSizer->addLayout(controlSizer, 0);

            setLayout(outerSizer);
        }

        void EntityBrowser::connectObservers() {
            auto document = kdl::mem_lock(m_document);
            m_notifierConnection += document->documentWasNewedNotifier.connect(this, &EntityBrowser::documentWasNewed);
            m_notifierConnection += document->documentWasLoadedNotifier.connect(this, &EntityBrowser::documentWasLoaded);
            m_notifierConnection += document->modsDidChangeNotifier.connect(this, &EntityBrowser::modsDidChange);
            m_notifierConnection += document->entityDefinitionsDidChangeNotifier.connect(this, &EntityBrowser::entityDefinitionsDidChange);

            PreferenceManager& prefs = PreferenceManager::instance();
            m_notifierConnection += prefs.preferenceDidChangeNotifier.connect(this, &EntityBrowser::preferenceDidChange);
        }

        void EntityBrowser::documentWasNewed(MapDocument*) {
            reload();
        }

        void EntityBrowser::documentWasLoaded(MapDocument*) {
            reload();
        }

        void EntityBrowser::modsDidChange() {
            reload();
        }

        void EntityBrowser::entityDefinitionsDidChange() {
            reload();
        }

        void EntityBrowser::preferenceDidChange(const IO::Path& path) {
            auto document = kdl::mem_lock(m_document);
            if (document->isGamePathPreference(path)) {
                reload();
            } else {
                m_view->update();
            }
        }
    }
}

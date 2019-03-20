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

#include "EntityDefinitionFileChooser.h"

#include "CollectionUtils.h"
#include "Notifier.h"
#include "Assets/EntityDefinitionFileSpec.h"
#include "IO/Path.h"
#include "Model/Game.h"
#include "Model/GameFactory.h"
#include "View/BorderLine.h"
#include "View/ChoosePathTypeDialog.h"
#include "View/MapDocument.h"
#include "View/TitledPanel.h"
#include "View/ViewConstants.h"
#include "View/ViewUtils.h"

#include <QPushButton>
#include <QListWidget>
#include <QLabel>
#include <QVBoxLayout>
#include <QFileDialog>
#include <QDebug>

#include <cassert>

namespace TrenchBroom {
    namespace View {
        // SingleSelectionListWidget

        SingleSelectionListWidget::SingleSelectionListWidget(QWidget* parent) :
        QListWidget(parent),
        m_allowDeselectAll(true) {}

        void SingleSelectionListWidget::selectionChanged(const QItemSelection& selected, const QItemSelection& deselected) {
            QListWidget::selectionChanged(selected, deselected);

            qDebug() << "selectionChanged " << selected.size() << " sel, " << deselected.size() << " desel\n";

            if (!m_allowDeselectAll) {
                if (selectedIndexes().isEmpty() && !deselected.isEmpty()) {
                    // reselect the items that were just deselected
                    selectionModel()->select(deselected, QItemSelectionModel::Select);
                }
            }
        }

        void SingleSelectionListWidget::setAllowDeselectAll(bool allow) {
            m_allowDeselectAll = allow;
        }

        bool SingleSelectionListWidget::allowDeselectAll() const {
            return m_allowDeselectAll;
        }

        // EntityDefinitionFileChooser

        EntityDefinitionFileChooser::EntityDefinitionFileChooser(QWidget* parent, MapDocumentWPtr document) :
        QWidget(parent),
        m_document(document) {
            createGui();
            bindEvents();
            bindObservers();
        }

        EntityDefinitionFileChooser::~EntityDefinitionFileChooser() {
            unbindObservers();
        }

        void EntityDefinitionFileChooser::OnBuiltinSelectionChanged() {
            qDebug("OnBuiltinSelectionChanged");

            if (m_builtin->selectedItems().isEmpty()) {
                return;
            }

            QListWidgetItem* item = m_builtin->selectedItems().first();
            auto spec = item->data(Qt::UserRole).value<Assets::EntityDefinitionFileSpec>();

            MapDocumentSPtr document = lock(m_document);
            if (document->entityDefinitionFile() == spec) {
                qDebug("OnBuiltinSelectionChanged: already on correct file");
                return;
            }

            qDebug() << "OnBuiltinSelectionChanged: setting to " << QString::fromStdString(spec.asString());
            document->setEntityDefinitionFile(spec);
        }

        void EntityDefinitionFileChooser::OnChooseExternalClicked() {
            const QString fileName = QFileDialog::getOpenFileName(nullptr, "Load Entity Definition File", "", "Worldcraft / Hammer files (*.fgd);;QuakeC files (*.def)");
            if (fileName.isEmpty())
                return;

            loadEntityDefinitionFile(m_document, this, fileName);
        }

        void EntityDefinitionFileChooser::OnReloadExternalClicked() {
            MapDocumentSPtr document = lock(m_document);
            const Assets::EntityDefinitionFileSpec& spec = document->entityDefinitionFile();
            document->setEntityDefinitionFile(spec);
        }

        void EntityDefinitionFileChooser::createGui() {
            TitledPanel* builtinContainer = new TitledPanel(nullptr, tr("Builtin"), false);
            //builtinContainer->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_LISTBOX));
            m_builtin = new SingleSelectionListWidget(); //builtinContainer->getPanel(), wxID_ANY, wxDefaultPosition, wxDefaultSize, 0, nullptr, wxBORDER_NONE);
            m_builtin->setAllowDeselectAll(false);

            auto* builtinSizer = new QVBoxLayout();
            builtinSizer->setContentsMargins(0, 0, 0, 0);
            builtinSizer->addWidget(m_builtin, 1);

            builtinContainer->getPanel()->setLayout(builtinSizer);

            TitledPanel* externalContainer = new TitledPanel(nullptr, tr("External"), false);
            m_external = new QLabel(tr("use builtin"));
            m_chooseExternal = new QPushButton(tr("Browse..."));
            m_chooseExternal->setToolTip(tr("Click to browse for an entity definition file"));
            m_reloadExternal = new QPushButton(tr("Reload"));
            m_reloadExternal->setToolTip(tr("Reload the currently loaded entity definition file"));

            auto* externalSizer = new QHBoxLayout();
            //externalSizer->addSpacing(LayoutConstants::NarrowHMargin);
            externalSizer->addWidget(m_external, 1);//, wxEXPAND | wxTOP | wxBOTTOM, LayoutConstants::NarrowVMargin);
            //externalSizer->addSpacing(LayoutConstants::NarrowHMargin);
            externalSizer->addWidget(m_chooseExternal, 0);//, wxALIGN_CENTER_VERTICAL | wxTOP | wxBOTTOM, LayoutConstants::NarrowVMargin);
            //externalSizer->addSpacing(LayoutConstants::NarrowHMargin);
            externalSizer->addWidget(m_reloadExternal, 0);//, wxALIGN_CENTER_VERTICAL | wxTOP | wxBOTTOM, LayoutConstants::NarrowVMargin);
            //externalSizer->addSpacing(LayoutConstants::NarrowHMargin);

            externalContainer->getPanel()->setLayout(externalSizer);

            auto* sizer = new QVBoxLayout();
            sizer->setContentsMargins(0, 0, 0, 0);
            sizer->setSpacing(0);
            sizer->addWidget(builtinContainer, 1);
            sizer->addWidget(new BorderLine(nullptr, BorderLine::Direction_Horizontal), 0);
            sizer->addWidget(externalContainer, 0);
            m_builtin->setMinimumSize(100, 70);

            setLayout(sizer);
        }

        void EntityDefinitionFileChooser::bindEvents() {
            connect(m_builtin, &QListWidget::itemSelectionChanged, this, &EntityDefinitionFileChooser::OnBuiltinSelectionChanged);
            connect(m_chooseExternal, &QAbstractButton::clicked, this, &EntityDefinitionFileChooser::OnChooseExternalClicked);
            connect(m_reloadExternal, &QAbstractButton::clicked, this, &EntityDefinitionFileChooser::OnReloadExternalClicked);
        }

        void EntityDefinitionFileChooser::bindObservers() {
            MapDocumentSPtr document = lock(m_document);
            document->documentWasNewedNotifier.addObserver(this, &EntityDefinitionFileChooser::documentWasNewed);
            document->documentWasLoadedNotifier.addObserver(this, &EntityDefinitionFileChooser::documentWasLoaded);
            document->entityDefinitionsDidChangeNotifier.addObserver(this, &EntityDefinitionFileChooser::entityDefinitionsDidChange);
        }

        void EntityDefinitionFileChooser::unbindObservers() {
            if (!expired(m_document)) {
                MapDocumentSPtr document = lock(m_document);
                document->documentWasNewedNotifier.removeObserver(this, &EntityDefinitionFileChooser::documentWasNewed);
                document->documentWasLoadedNotifier.removeObserver(this, &EntityDefinitionFileChooser::documentWasLoaded);
                document->entityDefinitionsDidChangeNotifier.removeObserver(this, &EntityDefinitionFileChooser::entityDefinitionsDidChange);
            }
        }

        void EntityDefinitionFileChooser::documentWasNewed(MapDocument* document) {
            updateControls();
        }

        void EntityDefinitionFileChooser::documentWasLoaded(MapDocument* document) {
            updateControls();
        }

        void EntityDefinitionFileChooser::entityDefinitionsDidChange() {
            updateControls();
        }

        void EntityDefinitionFileChooser::updateControls() {
            m_builtin->setAllowDeselectAll(true);
            m_builtin->clear();
            m_builtin->setAllowDeselectAll(false);

            MapDocumentSPtr document = lock(m_document);
            Assets::EntityDefinitionFileSpec::List specs = document->allEntityDefinitionFiles();
            VectorUtils::sort(specs);

            for (const Assets::EntityDefinitionFileSpec& spec : specs) {
                const IO::Path& path = spec.path();

                QListWidgetItem* item = new QListWidgetItem();
                item->setData(Qt::DisplayRole, path.lastComponent().asQString());
                item->setData(Qt::UserRole, QVariant::fromValue(spec));

                m_builtin->addItem(item);
            }

            const Assets::EntityDefinitionFileSpec spec = document->entityDefinitionFile();
            if (spec.builtin()) {
                const size_t index = VectorUtils::indexOf(specs, spec);
                if (index < specs.size()) {
                    // FIXME: Why would this not be the case?
                    m_builtin->setCurrentRow(static_cast<int>(index));
                }
                m_external->setText(tr("use builtin"));

                QPalette lightText;
                lightText.setColor(QPalette::WindowText, Colors::disabledText());
                m_external->setPalette(lightText);

                QFont font = m_external->font();
                font.setStyle(QFont::StyleOblique);
                m_external->setFont(font);
            } else {
                m_builtin->clearSelection();
                m_external->setText(spec.path().asQString());

                QPalette normalPal;
                m_external->setPalette(normalPal);

                QFont font = m_external->font();
                font.setStyle(QFont::StyleNormal);
                m_external->setFont(font);
            }

            m_reloadExternal->setEnabled(document->entityDefinitionFile().external());
        }
    }
}

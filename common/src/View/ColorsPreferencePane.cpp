/*
 Copyright (C) 2020 MaxED

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

#include "ColorsPreferencePane.h"

#include "View/ColorModel.h"
#include "View/QtUtils.h"

#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QSortFilterProxyModel>
#include <QTableView>

namespace TrenchBroom::View {
    ColorsPreferencePane::ColorsPreferencePane(QWidget* parent) :
    PreferencePane(parent),
    m_table(nullptr),
    m_model(nullptr),
    m_proxy(nullptr) {
        m_model = new ColorModel(this);
        m_proxy = new QSortFilterProxyModel(this);
        m_proxy->setSourceModel(m_model);
        m_proxy->setFilterCaseSensitivity(Qt::CaseInsensitive);
        m_proxy->setFilterKeyColumn(2); // Filter based on the text in the Description column

        m_table = new QTableView();
        m_table->setCornerButtonEnabled(false);
        m_table->setModel(m_proxy);

        m_table->setHorizontalHeader(new QHeaderView(Qt::Horizontal));
        m_table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeMode::Fixed);
        m_table->horizontalHeader()->resizeSection(0, 80);
        m_table->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeMode::ResizeToContents);
        m_table->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeMode::Stretch);

        // Tighter than default vertical row height, without the overhead of autoresizing
        m_table->verticalHeader()->setDefaultSectionSize(m_table->fontMetrics().lineSpacing() + 2);

        m_table->setSelectionMode(QAbstractItemView::SelectionMode::NoSelection);

        QLineEdit* searchBox = createSearchBox();
        makeSmall(searchBox);

        auto* infoLabel = new QLabel(tr("Double-click a color to begin editing it."));
        makeInfo(infoLabel);

        auto* infoAndSearchLayout = new QHBoxLayout();
        infoAndSearchLayout->setContentsMargins(
            LayoutConstants::WideHMargin,
            LayoutConstants::MediumVMargin,
            LayoutConstants::MediumHMargin,
            LayoutConstants::MediumVMargin);
        infoAndSearchLayout->setSpacing(LayoutConstants::WideHMargin);
        infoAndSearchLayout->addWidget(infoLabel, 1);
        infoAndSearchLayout->addWidget(searchBox);

        auto* layout = new QVBoxLayout();
        layout->setContentsMargins(0, 0, 0, 0);
        layout->setSpacing(0);
        layout->addWidget(m_table, 1);
        layout->addLayout(infoAndSearchLayout);
        setLayout(layout);

        setMinimumSize(900, 550);

        connect(searchBox, &QLineEdit::textChanged, this, [&](const QString& newText) {
            m_proxy->setFilterFixedString(newText);
        });

        connect(m_table, &QTableView::doubleClicked, this, [&](const QModelIndex& index) {
            m_model->pickColor(m_proxy->mapToSource(index));
        });
    }

    bool ColorsPreferencePane::doCanResetToDefaults() {
        return true;
    }

    void ColorsPreferencePane::doResetToDefaults() {
        m_model->reset();
    }

    void ColorsPreferencePane::doUpdateControls() {
        m_table->update();
    }

    bool ColorsPreferencePane::doValidate() {
        return true;
    }
}

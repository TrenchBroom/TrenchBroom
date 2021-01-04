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

#include "MoveObjectsToolPage.h"

#include "View/MapDocument.h"
#include "View/ViewConstants.h"

#include <kdl/memory_utils.h>
#include <vecmath/vec.h>
#include <vecmath/vec_io.h>

#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>

namespace TrenchBroom {
    namespace View {
        MoveObjectsToolPage::MoveObjectsToolPage(std::weak_ptr<MapDocument> document, QWidget* parent) :
        QWidget(parent),
        m_document(document),
        m_offset(nullptr),
        m_button(nullptr) {
            createGui();
            bindObservers();
            updateGui();
        }

        MoveObjectsToolPage::~MoveObjectsToolPage() {
            unbindObservers();
        }

        void MoveObjectsToolPage::bindObservers() {
            auto document = kdl::mem_lock(m_document);
            document->selectionDidChangeNotifier.addObserver(this, &MoveObjectsToolPage::selectionDidChange);
        }

        void MoveObjectsToolPage::unbindObservers() {
            if (!kdl::mem_expired(m_document)) {
                auto document = kdl::mem_lock(m_document);
                document->selectionDidChangeNotifier.removeObserver(this, &MoveObjectsToolPage::selectionDidChange);
            }
        }

        void MoveObjectsToolPage::createGui() {
            QLabel* text = new QLabel(tr("Move objects by"));
            m_offset = new QLineEdit("0.0 0.0 0.0");
            m_button = new QPushButton(tr("Apply"));

            connect(m_button, &QAbstractButton::clicked, this, &MoveObjectsToolPage::applyMove);
            connect(m_offset, &QLineEdit::returnPressed, this, &MoveObjectsToolPage::applyMove);

            auto* layout = new QHBoxLayout();
            layout->setContentsMargins(0, 0, 0, 0);
            layout->setSpacing(LayoutConstants::MediumHMargin);

            layout->addWidget(text, 0, Qt::AlignVCenter);
            layout->addWidget(m_offset, 0, Qt::AlignVCenter);
            layout->addWidget(m_button, 0, Qt::AlignVCenter);
            layout->addStretch(1);

            setLayout(layout);
        }

        void MoveObjectsToolPage::updateGui() {
            auto document = kdl::mem_lock(m_document);
            m_button->setEnabled(document->hasSelectedNodes());
        }

        void MoveObjectsToolPage::selectionDidChange(const Selection&) {
            updateGui();
        }

        void MoveObjectsToolPage::applyMove() {
            if (const auto delta = vm::parse<FloatType, 3>(m_offset->text().toStdString())) {
                auto document = kdl::mem_lock(m_document);
                document->translateObjects(*delta);
            }
        }
    }
}

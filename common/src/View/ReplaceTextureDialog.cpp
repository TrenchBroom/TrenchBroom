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

#include "ReplaceTextureDialog.h"

#include "Assets/Texture.h"
#include "Model/BrushFace.h"
#include "Model/CollectMatchingBrushFacesVisitor.h"
#include "Model/World.h"
#include "View/BorderLine.h"
#include "View/MapDocument.h"
#include "View/TextureBrowser.h"
#include "View/TitledPanel.h"
#include "View/QtUtils.h"

#include <kdl/memory_utils.h>

#include <QDialogButtonBox>
#include <QMessageBox>
#include <QPushButton>

#include <sstream>
#include <vector>

namespace TrenchBroom {
    namespace View {
        ReplaceTextureDialog::ReplaceTextureDialog(std::weak_ptr<MapDocument> document, GLContextManager& contextManager, QWidget* parent) :
        QDialog(parent),
        m_document(document),
        m_subjectBrowser(nullptr),
        m_replacementBrowser(nullptr),
        m_replaceButton(nullptr) {
            createGui(contextManager);
        }

        void ReplaceTextureDialog::accept() {
            const Assets::Texture* subject = m_subjectBrowser->selectedTexture();
            ensure(subject != nullptr, "subject is null");

            Assets::Texture* replacement = m_replacementBrowser->selectedTexture();
            ensure(replacement != nullptr, "replacement is null");

            auto document = kdl::mem_lock(m_document);
            const std::vector<Model::BrushFace*> faces = getApplicableFaces();

            if (faces.empty()) {
                QMessageBox::warning(this, tr("Replace Failed"), tr("None of the selected faces has the selected texture"));
                return;
            }

            Transaction transaction(document, "Replace Textures");
            document->select(faces);
            document->setTexture(replacement);

            std::stringstream msg;
            msg << "Replaced texture '" << subject->name() << "' with '" << replacement->name() << "' on " << faces.size() << " faces.";

            QMessageBox::information(this, tr("Replace Succeeded"), QString::fromStdString(msg.str()));
        }

        std::vector<Model::BrushFace*> ReplaceTextureDialog::getApplicableFaces() const {
            auto document = kdl::mem_lock(m_document);
            std::vector<Model::BrushFace*> faces = document->allSelectedBrushFaces();
            if (faces.empty()) {
                Model::CollectBrushFacesVisitor collect;
                document->world()->acceptAndRecurse(collect);
                faces = collect.faces();
            }

            const Assets::Texture* subject = m_subjectBrowser->selectedTexture();
            ensure(subject != nullptr, "subject is null");

            std::vector<Model::BrushFace*> result;
            for (auto* face : faces) {
                if (face->texture() == subject) {
                    result.push_back(face);
                }
            }
            return result;
        }

        void ReplaceTextureDialog::createGui(GLContextManager& contextManager) {
            setWindowIconTB(this);
            setWindowTitle(tr("Replace Texture"));

            auto* subjectPanel = new TitledPanel(tr("Find"));
            m_subjectBrowser = new TextureBrowser(m_document, contextManager);
            m_subjectBrowser->setHideUnused(true);
            connect(m_subjectBrowser, &TextureBrowser::textureSelected, this, &ReplaceTextureDialog::subjectSelected);

            auto* subjectPanelLayout = new QVBoxLayout();
            subjectPanelLayout->setContentsMargins(QMargins());
            subjectPanelLayout->setSpacing(0);
            subjectPanelLayout->addWidget(m_subjectBrowser);
            subjectPanel->getPanel()->setLayout(subjectPanelLayout);

            auto* replacementPanel = new TitledPanel(tr("Replace with"));
            m_replacementBrowser = new TextureBrowser(m_document, contextManager);
            m_replacementBrowser->setSelectedTexture(nullptr); // Override the current texture.
            connect(m_replacementBrowser, &TextureBrowser::textureSelected, this, &ReplaceTextureDialog::replacementSelected);

            auto* replacementPanelLayout = new QVBoxLayout();
            replacementPanelLayout->setContentsMargins(QMargins());
            replacementPanelLayout->setSpacing(0);
            replacementPanelLayout->addWidget(m_replacementBrowser, 1);
            replacementPanel->getPanel()->setLayout(replacementPanelLayout);

            auto* upperLayout = new QHBoxLayout();
            upperLayout->setContentsMargins(QMargins());
            upperLayout->setSpacing(0);
            upperLayout->addWidget(subjectPanel, 1);
            upperLayout->addWidget(new BorderLine(BorderLine::Direction::Vertical), 0);
            upperLayout->addWidget(replacementPanel, 1);

            auto* buttonBox = new QDialogButtonBox(this);
            m_replaceButton = buttonBox->addButton(tr("Replace"), QDialogButtonBox::AcceptRole);
            m_replaceButton->setToolTip(tr("Perform replacement on all selected faces"));
            m_replaceButton->setEnabled(false);
            auto* closeButton = buttonBox->addButton(tr("Close"), QDialogButtonBox::RejectRole);
            closeButton->setToolTip(tr("Close this window"));

            connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
            connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

            auto* outerLayout = new QVBoxLayout();
            outerLayout->setContentsMargins(QMargins());
            outerLayout->setSpacing(0);
            outerLayout->addLayout(upperLayout, 1);
            outerLayout->addLayout(wrapDialogButtonBox(buttonBox), 0);
            setLayout(outerLayout);

            setMinimumSize(650, 450);
        }

        void ReplaceTextureDialog::subjectSelected(Assets::Texture* /* subject */) {
            updateReplaceButton();
        }

        void ReplaceTextureDialog::replacementSelected(Assets::Texture* /* replacement */) {
            updateReplaceButton();
        }

        void ReplaceTextureDialog::updateReplaceButton() {
            const Assets::Texture* subject = m_subjectBrowser->selectedTexture();
            const Assets::Texture* replacement = m_replacementBrowser->selectedTexture();
            m_replaceButton->setEnabled(subject != nullptr && replacement != nullptr);
        }
    }
}

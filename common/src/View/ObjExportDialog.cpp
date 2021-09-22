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

#include "ObjExportDialog.h"

#include "Model/ExportFormat.h"
#include "IO/ExportOptions.h"
#include "IO/Path.h"
#include "IO/PathQt.h"
#include "View/MapDocument.h"
#include "View/MapFrame.h"
#include "QtUtils.h"

#include <QDialogButtonBox>
#include <QFileDialog>
#include <QFormLayout>

namespace TrenchBroom {
    namespace View {
        ObjExportDialog::ObjExportDialog(MapFrame* mapFrame) :
        m_mapFrame(mapFrame),
        m_exportButton(nullptr),
        m_closeButton(nullptr),
        m_relativeCheckBox(nullptr),
        m_pathEdit(nullptr),
        m_browseButton(nullptr) {
            createGui();
            resize(600, 0);
        }

        void ObjExportDialog::createGui() {
            setWindowIconTB(this);
            setWindowTitle("Export as Wavefront OBJ");

            // Bottom button row.
            auto* buttonBox = new QDialogButtonBox();
            m_closeButton = buttonBox->addButton("Close", QDialogButtonBox::RejectRole);
            m_exportButton = buttonBox->addButton("Export", QDialogButtonBox::AcceptRole);

            auto* buttonLayout = new QHBoxLayout();
            buttonLayout->setContentsMargins(0, 0, 0, 0);
            buttonLayout->setSpacing(LayoutConstants::WideHMargin);
            buttonLayout->addWidget(buttonBox);

            // Compilation options.
            auto* pathLayout = new QHBoxLayout();
            m_pathEdit = new QLineEdit();
            pathLayout->addWidget(m_pathEdit);

            auto document = m_mapFrame->document();
            const IO::Path& originalPath = document->path();
            const IO::Path objPath = originalPath.replaceExtension("obj");

            m_pathEdit->setText(IO::pathAsQString(objPath));

            m_browseButton = new QPushButton();
            m_browseButton->setText("Browse...");
            pathLayout->addWidget(m_browseButton);

            m_relativeCheckBox = new QCheckBox();
            m_relativeCheckBox->setText(tr("Use texture paths relative to game directory."));

            auto* dialogLayout = new QVBoxLayout();
            dialogLayout->setContentsMargins(5, 5, 5, 5);
            dialogLayout->setSpacing(5);
            dialogLayout->addLayout(pathLayout);
            dialogLayout->addWidget(m_relativeCheckBox);
            dialogLayout->addLayout(wrapDialogButtonBox(buttonLayout));
            insertTitleBarSeparator(dialogLayout);

            setLayout(dialogLayout);

            m_exportButton->setDefault(true);

            connect(m_closeButton, &QPushButton::clicked, this, &ObjExportDialog::close);
            connect(m_browseButton, &QPushButton::clicked, this, [&]() {
                const QString newFileName = QFileDialog::getSaveFileName(this, tr("Export Wavefront OBJ file"), m_pathEdit->text(), "Wavefront OBJ files (*.obj)");
                if (!newFileName.isEmpty()) {
                    m_pathEdit->setText(newFileName);
                }
            });
            connect(m_exportButton, &QPushButton::clicked, this, [&]() {
                std::shared_ptr<IO::ObjExportOptions> options = std::make_shared<IO::ObjExportOptions>();
                options->exportPath = IO::pathFromQString(m_pathEdit->text());
                options->gameDirRelativePaths = m_relativeCheckBox->isChecked();
                m_mapFrame->exportDocument(Model::ExportFormat::WavefrontObj, options);
                close();
            });
        }
    }
}

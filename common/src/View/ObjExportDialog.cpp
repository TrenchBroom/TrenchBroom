/*
 Copyright (C) 2021 Amara M. Kilic
 Copyright (C) 2021 Kristian Duske

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

#include "Ensure.h"
#include "IO/ExportOptions.h"
#include "IO/PathQt.h"
#include "QtUtils.h"
#include "View/BorderLine.h"
#include "View/DialogHeader.h"
#include "View/FormWithSectionsLayout.h"
#include "View/MapDocument.h"
#include "View/MapFrame.h"

#include <kdl/path_utils.h>

#include <filesystem>

#include <QDialogButtonBox>
#include <QFileDialog>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QRadioButton>

namespace TrenchBroom
{
namespace View
{
ObjExportDialog::ObjExportDialog(MapFrame* mapFrame)
  : QDialog{mapFrame}
  , m_mapFrame{mapFrame}
  , m_exportPathEdit{nullptr}
  , m_browseExportPathButton{nullptr}
  , m_relativeToGamePathRadioButton{nullptr}
  , m_relativeToExportPathRadioButton{nullptr}
  , m_exportButton{nullptr}
  , m_closeButton{nullptr}
{
  ensure(m_mapFrame != nullptr, "Map frame is not null");
  createGui();
  resize(500, 0);
  // setFixedHeight(height());
}

void ObjExportDialog::createGui()
{
  setWindowIconTB(this);
  setWindowTitle("Export");

  auto* header = new DialogHeader{tr("Export Wavefront OBJ")};

  auto* formLayout = new FormWithSectionsLayout{};
  formLayout->setContentsMargins(0, 20, 0, 20);
  formLayout->setHorizontalSpacing(LayoutConstants::WideHMargin);
  formLayout->setVerticalSpacing(LayoutConstants::MediumVMargin);
  formLayout->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);

  formLayout->addSection(tr("Export Path"));

  auto* exportPathLayout = new QHBoxLayout{};
  exportPathLayout->setContentsMargins(0, 0, 0, 0);
  exportPathLayout->setSpacing(LayoutConstants::MediumHMargin);

  m_exportPathEdit = new QLineEdit{};
  m_exportPathEdit->setPlaceholderText(tr("Enter a path or click to browse"));
  exportPathLayout->addWidget(m_exportPathEdit);

  m_browseExportPathButton = new QPushButton{};
  m_browseExportPathButton->setText("Browse...");
  exportPathLayout->addWidget(m_browseExportPathButton);

  formLayout->addRow("Path", exportPathLayout);

  formLayout->addSection(
    tr("Texture Paths"),
    tr("Controls how the texture paths in the generated material file are computed."));

  m_relativeToGamePathRadioButton = new QRadioButton{};
  m_relativeToGamePathRadioButton->setText(tr("Relative to game path"));
  m_relativeToGamePathRadioButton->setChecked(true);

  m_relativeToExportPathRadioButton = new QRadioButton{};
  m_relativeToExportPathRadioButton->setText(tr("Relative to export path"));

  auto* texturePathLayout = new QVBoxLayout{};
  texturePathLayout->setContentsMargins(0, 0, 0, 0);
  texturePathLayout->setSpacing(0);
  texturePathLayout->addWidget(m_relativeToGamePathRadioButton);
  texturePathLayout->addWidget(m_relativeToExportPathRadioButton);

  formLayout->addRow(texturePathLayout);

  auto* innerLayout = new QVBoxLayout{};
  innerLayout->setContentsMargins(0, 0, 0, 0);
  innerLayout->setSpacing(0);
  innerLayout->addWidget(header);
  innerLayout->addWidget(new BorderLine{});
  innerLayout->addLayout(formLayout);

  auto* outerLayout = new QVBoxLayout{};
  outerLayout->setContentsMargins(0, 0, 0, 0);
  outerLayout->setSpacing(LayoutConstants::MediumVMargin);
  outerLayout->addLayout(innerLayout);

  // Bottom button row.
  auto* buttonBox = new QDialogButtonBox{};
  m_closeButton = buttonBox->addButton(QDialogButtonBox::Cancel);
  m_exportButton = buttonBox->addButton("Export", QDialogButtonBox::AcceptRole);
  m_exportButton->setDefault(true);

  outerLayout->addLayout(wrapDialogButtonBox(buttonBox));

  insertTitleBarSeparator(outerLayout);

  setLayout(outerLayout);

  connect(m_closeButton, &QPushButton::clicked, this, &ObjExportDialog::close);
  connect(m_browseExportPathButton, &QPushButton::clicked, this, [&]() {
    const QString newFileName = QFileDialog::getSaveFileName(
      this,
      tr("Export Wavefront OBJ file"),
      m_exportPathEdit->text(),
      "Wavefront OBJ files (*.obj)");
    if (!newFileName.isEmpty())
    {
      m_exportPathEdit->setText(newFileName);
    }
  });
  connect(m_exportButton, &QPushButton::clicked, this, [&]() {
    IO::ObjExportOptions options;
    options.exportPath = IO::pathFromQString(m_exportPathEdit->text());
    options.mtlPathMode = m_relativeToGamePathRadioButton->isChecked()
                            ? IO::ObjMtlPathMode::RelativeToGamePath
                            : IO::ObjMtlPathMode::RelativeToExportPath;
    m_mapFrame->exportDocument(options);
    close();
  });
}

void ObjExportDialog::updateExportPath()
{
  const auto document = m_mapFrame->document();
  const auto& originalPath = document->path();
  const auto objPath = kdl::path_replace_extension(originalPath, ".obj");
  m_exportPathEdit->setText(IO::pathAsQString(objPath));
}
} // namespace View
} // namespace TrenchBroom

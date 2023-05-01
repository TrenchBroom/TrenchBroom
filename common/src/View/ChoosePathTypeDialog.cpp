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

#include "ChoosePathTypeDialog.h"

#include "IO/PathQt.h"
#include "IO/SystemPaths.h"
#include "View/QtUtils.h"
#include "View/ViewConstants.h"

#include <QDialogButtonBox>
#include <QLabel>
#include <QRadioButton>

namespace TrenchBroom
{
namespace View
{

ChoosePathTypeDialog::ChoosePathTypeDialog(
  QWidget* parent, IO::Path absPath, const IO::Path& docPath, const IO::Path& gamePath)
  : QDialog{parent}
  , m_absPath{std::move(absPath)}
  , m_docRelativePath{m_absPath.lexically_relative(docPath.parent_path())}
  , m_gameRelativePath{m_absPath.lexically_relative(gamePath)}
  , m_appRelativePath{m_absPath.lexically_relative(IO::SystemPaths::appDirectory())}
{
  createGui();
}

void ChoosePathTypeDialog::createGui()
{
  setWindowTitle(tr("Path Type"));
  setWindowIconTB(this);

  auto* infoText =
    new QLabel{tr("Paths can be stored either as absolute paths or as relative "
                  "paths. Please choose how you want to store this path.")};
  infoText->setMaximumWidth(370);
  infoText->setWordWrap(true);

  auto boldFont = infoText->font();
  boldFont.setBold(true);

  m_absRadio = new QRadioButton{tr("Absolute")};
  m_absRadio->setFont(boldFont);
  m_absRadio->setChecked(true);

  auto* absolutePathText = new QLabel{IO::pathAsQString(m_absPath)};

  m_docRelativeRadio = new QRadioButton{tr("Relative to map file")};
  m_docRelativeRadio->setFont(boldFont);
  m_docRelativeRadio->setEnabled(!m_docRelativePath.empty());

  auto* mapRelativePathText = new QLabel{
    m_docRelativePath.empty() ? tr("Could not build a path.")
                              : IO::pathAsQString(m_docRelativePath)};

  m_appRelativeRadio = new QRadioButton{tr("Relative to application executable")};
  m_appRelativeRadio->setFont(boldFont);
  m_appRelativeRadio->setEnabled(!m_appRelativePath.empty());

  auto* appRelativePathText = new QLabel{
    m_appRelativePath.empty() ? tr("Could not build a path.")
                              : IO::pathAsQString(m_appRelativePath)};

  m_gameRelativeRadio = new QRadioButton{tr("Relative to game directory")};
  m_gameRelativeRadio->setFont(boldFont);
  m_gameRelativeRadio->setEnabled(!m_gameRelativePath.empty());

  auto* gameRelativePathText = new QLabel{
    m_gameRelativePath.empty() ? tr("Could not build a path.")
                               : IO::pathAsQString(m_gameRelativePath)};

  auto* innerLayout = new QVBoxLayout{};
  innerLayout->setContentsMargins(
    LayoutConstants::WideHMargin,
    LayoutConstants::WideVMargin,
    LayoutConstants::WideHMargin,
    LayoutConstants::WideVMargin);
  innerLayout->setSpacing(0);

  innerLayout->addWidget(infoText);
  innerLayout->addSpacing(LayoutConstants::WideVMargin);

  innerLayout->addWidget(m_absRadio);
  innerLayout->addWidget(absolutePathText);
  innerLayout->addSpacing(LayoutConstants::WideVMargin);

  innerLayout->addWidget(m_docRelativeRadio);
  innerLayout->addWidget(mapRelativePathText);
  innerLayout->addSpacing(LayoutConstants::WideVMargin);

  innerLayout->addWidget(m_appRelativeRadio);
  innerLayout->addWidget(appRelativePathText);
  innerLayout->addSpacing(LayoutConstants::WideVMargin);

  innerLayout->addWidget(m_gameRelativeRadio);
  innerLayout->addWidget(gameRelativePathText);

  auto* outerLayout = new QVBoxLayout{};
  outerLayout->setContentsMargins(0, 0, 0, 0);
  outerLayout->setSpacing(LayoutConstants::MediumVMargin);
  outerLayout->addLayout(innerLayout);

  auto* okCancelButtons =
    new QDialogButtonBox{QDialogButtonBox::Ok | QDialogButtonBox::Cancel};
  outerLayout->addLayout(wrapDialogButtonBox(okCancelButtons));

  insertTitleBarSeparator(outerLayout);

  setLayout(outerLayout);

  connect(okCancelButtons, &QDialogButtonBox::accepted, this, &QDialog::accept);
  connect(okCancelButtons, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

const IO::Path& ChoosePathTypeDialog::path() const
{
  if (m_docRelativeRadio->isChecked())
  {
    return m_docRelativePath;
  }
  if (m_appRelativeRadio->isChecked())
  {
    return m_appRelativePath;
  }
  if (m_gameRelativeRadio->isChecked())
  {
    return m_gameRelativePath;
  }
  return m_absPath;
}
} // namespace View
} // namespace TrenchBroom

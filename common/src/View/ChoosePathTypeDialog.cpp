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

#include <QDialogButtonBox>
#include <QLabel>
#include <QRadioButton>

#include "IO/PathQt.h"
#include "IO/SystemPaths.h"
#include "Macros.h"
#include "View/BorderLine.h"
#include "View/DialogHeader.h"
#include "View/QtUtils.h"
#include "View/ViewConstants.h"

namespace TrenchBroom::View
{

std::filesystem::path convertToPathType(
  const PathType pathType,
  const std::filesystem::path& absPath,
  const std::filesystem::path& docPath,
  const std::filesystem::path& gamePath)
{
  switch (pathType)
  {
  case PathType::Absolute:
    return absPath;
  case PathType::DocumentRelative:
    return absPath.lexically_relative(docPath.parent_path());
  case PathType::GameRelative:
    return absPath.lexically_relative(gamePath);
  case PathType::AppRelative:
    return absPath.lexically_relative(IO::SystemPaths::appDirectory());
    switchDefault();
  }
}

ChoosePathTypeDialog::ChoosePathTypeDialog(
  QWidget* parent,
  const std::filesystem::path& absPath,
  const std::filesystem::path& docPath,
  const std::filesystem::path& gamePath)
  : QDialog{parent}
{
  createGui(absPath, docPath, gamePath);
}

void ChoosePathTypeDialog::createGui(
  const std::filesystem::path& absPath,
  const std::filesystem::path& docPath,
  const std::filesystem::path& gamePath)
{
  const auto docRelativePath =
    convertToPathType(PathType::DocumentRelative, absPath, docPath, gamePath);
  const auto gameRelativePath =
    convertToPathType(PathType::GameRelative, absPath, docPath, gamePath);
  const auto appRelativePath =
    convertToPathType(PathType::AppRelative, absPath, docPath, gamePath);

  setWindowTitle(tr("Path Type"));
  setWindowIconTB(this);

  auto* infoText = new QLabel{
    tr("You can convert a path to be relative to some reference path, or you can choose "
       "to keep it absolute. A relative can make it easier to collaborate on a map.")};
  infoText->setMaximumWidth(370);
  infoText->setWordWrap(true);

  auto boldFont = infoText->font();
  boldFont.setBold(true);

  m_absRadio = new QRadioButton{tr("Absolute")};
  makeEmphasized(m_absRadio);
  m_absRadio->setChecked(true);

  auto* absolutePathText = makeInfo(new QLabel{IO::pathAsQString(absPath)});

  m_docRelativeRadio = new QRadioButton{tr("Relative to map file")};
  makeEmphasized(m_docRelativeRadio);
  m_docRelativeRadio->setEnabled(!docRelativePath.empty());

  auto* mapRelativePathText = makeInfo(new QLabel{
    docRelativePath.empty() ? tr("Could not build a path.")
                            : IO::pathAsQString(docRelativePath)});

  m_appRelativeRadio = new QRadioButton{tr("Relative to application executable")};
  makeEmphasized(m_appRelativeRadio);
  m_appRelativeRadio->setEnabled(!appRelativePath.empty());

  auto* appRelativePathText = makeInfo(new QLabel{
    appRelativePath.empty() ? tr("Could not build a path.")
                            : IO::pathAsQString(appRelativePath)});

  m_gameRelativeRadio = new QRadioButton{tr("Relative to game directory")};
  makeEmphasized(m_gameRelativeRadio);
  m_gameRelativeRadio->setEnabled(!gameRelativePath.empty());

  auto* gameRelativePathText = makeInfo(new QLabel{
    gameRelativePath.empty() ? tr("Could not build a path.")
                             : IO::pathAsQString(gameRelativePath)});

  auto* okCancelButtons =
    new QDialogButtonBox{QDialogButtonBox::Ok | QDialogButtonBox::Cancel};

  auto* innerLayout = new QVBoxLayout{};
  innerLayout->setContentsMargins(
    LayoutConstants::DialogOuterMargin,
    LayoutConstants::DialogOuterMargin,
    LayoutConstants::DialogOuterMargin,
    LayoutConstants::DialogOuterMargin);
  innerLayout->setSpacing(LayoutConstants::NarrowVMargin);

  innerLayout->addWidget(infoText);
  innerLayout->addSpacing(LayoutConstants::WideVMargin);
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
  outerLayout->setSpacing(0);
  outerLayout->addWidget(new DialogHeader{"Choose Path Type"});
  outerLayout->addWidget(new BorderLine{BorderLine::Direction::Horizontal});
  outerLayout->addLayout(innerLayout);
  outerLayout->addLayout(wrapDialogButtonBox(okCancelButtons));

  setLayout(outerLayout);

  connect(okCancelButtons, &QDialogButtonBox::accepted, this, &QDialog::accept);
  connect(okCancelButtons, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

PathType ChoosePathTypeDialog::pathType() const
{
  if (m_docRelativeRadio->isChecked())
  {
    return PathType::DocumentRelative;
  }
  if (m_appRelativeRadio->isChecked())
  {
    return PathType::AppRelative;
  }
  if (m_gameRelativeRadio->isChecked())
  {
    return PathType::GameRelative;
  }
  return PathType::Absolute;
}

} // namespace TrenchBroom::View

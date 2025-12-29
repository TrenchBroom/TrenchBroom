/*
 Copyright (C) 2010 Kristian Duske

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

#include <QBoxLayout>
#include <QDialogButtonBox>
#include <QLabel>
#include <QRadioButton>

#include "Macros.h"
#include "ui/BorderLine.h"
#include "ui/DialogButtonLayout.h"
#include "ui/DialogHeader.h"
#include "ui/QPathUtils.h"
#include "ui/QStyleUtils.h"
#include "ui/SystemPaths.h"
#include "ui/ViewConstants.h"

namespace tb::ui
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
    return absPath.lexically_relative(SystemPaths::appDirectory());
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
  setEmphasizedStyle(m_absRadio);
  m_absRadio->setChecked(true);

  auto* absolutePathText = setInfoStyle(new QLabel{pathAsQString(absPath)});

  m_docRelativeRadio = new QRadioButton{tr("Relative to map file")};
  setEmphasizedStyle(m_docRelativeRadio);
  m_docRelativeRadio->setEnabled(!docRelativePath.empty());

  auto* mapRelativePathText = setInfoStyle(new QLabel{
    docRelativePath.empty() ? tr("Could not build a path.")
                            : pathAsQString(docRelativePath)});

  m_appRelativeRadio = new QRadioButton{tr("Relative to application executable")};
  setEmphasizedStyle(m_appRelativeRadio);
  m_appRelativeRadio->setEnabled(!appRelativePath.empty());

  auto* appRelativePathText = setInfoStyle(new QLabel{
    appRelativePath.empty() ? tr("Could not build a path.")
                            : pathAsQString(appRelativePath)});

  m_gameRelativeRadio = new QRadioButton{tr("Relative to game directory")};
  setEmphasizedStyle(m_gameRelativeRadio);
  m_gameRelativeRadio->setEnabled(!gameRelativePath.empty());

  auto* gameRelativePathText = setInfoStyle(new QLabel{
    gameRelativePath.empty() ? tr("Could not build a path.")
                             : pathAsQString(gameRelativePath)});

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
  outerLayout->addWidget(new DialogHeader{tr("Choose Path Type")});
  outerLayout->addWidget(new BorderLine{});
  outerLayout->addLayout(innerLayout);
  outerLayout->addLayout(wrapDialogButtonBox(okCancelButtons));

  setLayout(outerLayout);

  connect(okCancelButtons, &QDialogButtonBox::accepted, this, &QDialog::accept);
  connect(okCancelButtons, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

PathType ChoosePathTypeDialog::pathType() const
{
  return m_docRelativeRadio->isChecked()    ? PathType::DocumentRelative
         : m_appRelativeRadio->isChecked()  ? PathType::AppRelative
         : m_gameRelativeRadio->isChecked() ? PathType::GameRelative
                                            : PathType::Absolute;
}

} // namespace tb::ui

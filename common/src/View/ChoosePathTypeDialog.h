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

#pragma once

#include <QDialog>

#include <filesystem>
#include <optional>

class QRadioButton;
class QWidget;

namespace TrenchBroom::View
{

enum class PathType
{
  Absolute,
  DocumentRelative,
  GameRelative,
  AppRelative,
};

std::filesystem::path convertToPathType(
  PathType pathType,
  const std::filesystem::path& absPath,
  const std::filesystem::path& docPath,
  const std::filesystem::path& gamePath);

class ChoosePathTypeDialog : public QDialog
{
  Q_OBJECT
private:
  QRadioButton* m_absRadio;
  QRadioButton* m_docRelativeRadio;
  QRadioButton* m_appRelativeRadio;
  QRadioButton* m_gameRelativeRadio;

private:
  void createGui(
    const std::filesystem::path& absPath,
    const std::filesystem::path& docPath,
    const std::filesystem::path& gamePath);

public:
  ChoosePathTypeDialog(
    QWidget* parent,
    const std::filesystem::path& absPath,
    const std::filesystem::path& docPath,
    const std::filesystem::path& gamePath);

  PathType pathType() const;
};

} // namespace TrenchBroom::View

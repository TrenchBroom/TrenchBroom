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

class QRadioButton;
class QWidget;

namespace TrenchBroom::View
{

class ChoosePathTypeDialog : public QDialog
{
  Q_OBJECT
private:
  std::filesystem::path m_absPath;
  std::filesystem::path m_docRelativePath;
  std::filesystem::path m_gameRelativePath;
  std::filesystem::path m_appRelativePath;

  QRadioButton* m_absRadio;
  QRadioButton* m_docRelativeRadio;
  QRadioButton* m_appRelativeRadio;
  QRadioButton* m_gameRelativeRadio;

private:
  void createGui();

public:
  ChoosePathTypeDialog(
    QWidget* parent,
    std::filesystem::path absPath,
    const std::filesystem::path& docPath,
    const std::filesystem::path& gamePath);

  const std::filesystem::path& path() const;
};

} // namespace TrenchBroom::View

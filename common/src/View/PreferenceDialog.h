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

#pragma once

#include <QDialog>

#include <memory>

class QDialogButtonBox;
class QStackedWidget;
class QToolBar;
class QWidget;

namespace TrenchBroom::View
{
class MapDocument;
class PreferencePane;

class PreferenceDialog : public QDialog
{
  Q_OBJECT
private:
  enum class PrefPane;

  std::shared_ptr<MapDocument> m_document;
  QToolBar* m_toolBar = nullptr;
  QStackedWidget* m_stackedWidget = nullptr;
  QDialogButtonBox* m_buttonBox = nullptr;

public:
  explicit PreferenceDialog(
    std::shared_ptr<MapDocument> document, QWidget* parent = nullptr);

protected: // QWidget overrides
  void closeEvent(QCloseEvent* event) override;
  bool eventFilter(QObject* o, QEvent* e) override;

private:
  void createGui();
  void switchToPane(PrefPane pane);
  PreferencePane* currentPane() const;
private slots:
  void resetToDefaults();
};
} // namespace TrenchBroom::View

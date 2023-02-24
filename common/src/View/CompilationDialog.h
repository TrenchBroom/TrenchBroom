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

#include "View/CompilationRun.h"

#include <QDialog>

class QLabel;
class QPushButton;
class QTextEdit;

namespace TrenchBroom
{
namespace View
{
class CompilationProfileManager;
class MapFrame;

class CompilationDialog : public QDialog
{
  Q_OBJECT
private:
  MapFrame* m_mapFrame{nullptr};
  CompilationProfileManager* m_profileManager{nullptr};
  QPushButton* m_launchButton{nullptr};
  QPushButton* m_compileButton{nullptr};
  QPushButton* m_testCompileButton{nullptr};
  QPushButton* m_stopCompileButton{nullptr};
  QPushButton* m_closeButton{nullptr};
  QLabel* m_currentRunLabel{nullptr};
  QTextEdit* m_output{nullptr};
  CompilationRun m_run;

public:
  explicit CompilationDialog(MapFrame* mapFrame);

private:
  void createGui();

  void keyPressEvent(QKeyEvent* event) override;

  void updateCompileButtons();
  void startCompilation(bool test);
  void stopCompilation();
  void closeEvent(QCloseEvent* event) override;
private slots:
  void compilationStarted();
  void compilationEnded();

  void selectedProfileChanged();
  void profileChanged();

private:
  void saveProfile();
};
} // namespace View
} // namespace TrenchBroom

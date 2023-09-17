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

#include <QWidget>

class QLineEdit;
class QStackedWidget;

namespace TrenchBroom::Model
{
struct GameEngineProfile;
}

namespace TrenchBroom::View
{
/**
 * Editor widget for a single game engine profile.
 */
class GameEngineProfileEditor : public QWidget
{
  Q_OBJECT
private:
  Model::GameEngineProfile* m_profile = nullptr;
  QStackedWidget* m_stackedWidget = nullptr;
  QLineEdit* m_nameEdit = nullptr;
  QLineEdit* m_pathEdit = nullptr;

public:
  explicit GameEngineProfileEditor(QWidget* parent = nullptr);

private:
  QWidget* createEditorPage();
  void updatePath(const QString& str);

public:
  void setProfile(Model::GameEngineProfile* profile);

private:
  void refresh();

  bool isValidEnginePath(const QString& str) const;
private slots:
  void nameChanged(const QString& text);
  void pathChanged();
  void changePathClicked();
signals:
  /**
   * Emitted after m_profile is changed in response to a UI action.
   */
  void profileChanged();
};

} // namespace TrenchBroom::View

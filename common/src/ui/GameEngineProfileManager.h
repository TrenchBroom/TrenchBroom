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

#include <QWidget>

#include "mdl/GameEngineConfig.h"

class QAbstractButton;

namespace tb::mdl
{
struct GameEngineProfile;
} // namespace tb::mdl

namespace tb::ui
{

class GameEngineProfileEditor;
class GameEngineProfileListBox;

/**
 * Widget for editing game engine profiles (name/path, not parameters).
 */
class GameEngineProfileManager : public QWidget
{
  Q_OBJECT
private:
  mdl::GameEngineConfig m_config;
  GameEngineProfileListBox* m_profileList = nullptr;
  GameEngineProfileEditor* m_profileEditor = nullptr;
  QAbstractButton* m_removeProfileButton = nullptr;

public:
  explicit GameEngineProfileManager(
    mdl::GameEngineConfig config, QWidget* parent = nullptr);
  const mdl::GameEngineConfig& config() const;
private slots:
  void addProfile();
  void removeProfile();

  void currentProfileChanged(mdl::GameEngineProfile* profile);
};

} // namespace tb::ui

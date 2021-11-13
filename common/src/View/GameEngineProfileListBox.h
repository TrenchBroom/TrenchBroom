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

#include "View/ControlListBox.h"

namespace TrenchBroom {
namespace Model {
class GameEngineConfig;
class GameEngineProfile;
} // namespace Model

namespace View {
class ElidedLabel;

class GameEngineProfileItemRenderer : public ControlListBoxItemRenderer {
  Q_OBJECT
private:
  Model::GameEngineProfile* m_profile;
  ElidedLabel* m_nameLabel;
  ElidedLabel* m_pathLabel;

public:
  explicit GameEngineProfileItemRenderer(
    Model::GameEngineProfile* profile, QWidget* parent = nullptr);

  void updateItem() override;

private:
  void createGui();
  void refresh();
  void profileWillBeRemoved();
  void profileDidChange();
};

class GameEngineProfileListBox : public ControlListBox {
  Q_OBJECT
private:
  const Model::GameEngineConfig* m_config;

public:
  explicit GameEngineProfileListBox(
    const Model::GameEngineConfig* config, QWidget* parent = nullptr);

  Model::GameEngineProfile* selectedProfile() const;

public:
  void setConfig(const Model::GameEngineConfig* config);
  void reloadProfiles();
  void updateProfiles();

private:
  size_t itemCount() const override;
  ControlListBoxItemRenderer* createItemRenderer(QWidget* parent, size_t index) override;
  void selectedRowChanged(int index) override;
  void doubleClicked(size_t index) override;
signals:
  /**
   * Emitted when the selection changes.
   */
  void currentProfileChanged(Model::GameEngineProfile* profile);
  /**
   * Emitted when a profile is double-clicked.
   */
  void profileSelected(Model::GameEngineProfile* profile);
};
} // namespace View
} // namespace TrenchBroom

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

#include "View/ControlListBox.h"

namespace tb::mdl
{
struct GameEngineConfig;
struct GameEngineProfile;
} // namespace tb::mdl

namespace tb::View
{
class ElidedLabel;

class GameEngineProfileItemRenderer : public ControlListBoxItemRenderer
{
  Q_OBJECT
private:
  mdl::GameEngineProfile* m_profile;
  ElidedLabel* m_nameLabel{nullptr};
  ElidedLabel* m_pathLabel{nullptr};

public:
  explicit GameEngineProfileItemRenderer(
    mdl::GameEngineProfile& profile, QWidget* parent = nullptr);

  void updateItem() override;

private:
  void createGui();
  void refresh();
  void profileWillBeRemoved();
  void profileDidChange();
};

class GameEngineProfileListBox : public ControlListBox
{
  Q_OBJECT
private:
  mdl::GameEngineConfig* m_config;

public:
  explicit GameEngineProfileListBox(
    mdl::GameEngineConfig& config, QWidget* parent = nullptr);

  mdl::GameEngineProfile* selectedProfile();

public:
  void setConfig(mdl::GameEngineConfig& config);
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
  void currentProfileChanged(mdl::GameEngineProfile* profile);
  /**
   * Emitted when a profile is double-clicked.
   */
  void profileSelected(mdl::GameEngineProfile& profile);
};

} // namespace tb::View

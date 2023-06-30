/*
 Copyright (C) 2023 Kristian Duske

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

#include "View/SmartPropertyEditor.h"

#include <memory>

class QAbstractButton;
class QListWidget;
class QWidget;

namespace TrenchBroom::View
{

class MapDocument;

class SmartWadEditor : public SmartPropertyEditor
{
  Q_OBJECT
private:
  QListWidget* m_wadPaths{nullptr};

  QAbstractButton* m_addWadsButton{nullptr};
  QAbstractButton* m_removeWadsButton{nullptr};
  QAbstractButton* m_moveWadUpButton{nullptr};
  QAbstractButton* m_moveWadDownButton{nullptr};
  QAbstractButton* m_reloadWadsButton{nullptr};

public:
  explicit SmartWadEditor(std::weak_ptr<MapDocument> document, QWidget* parent = nullptr);

  void addWads();
  void removeSelectedWads();
  void moveSelectedWadsUp();
  void moveSelectedWadsDown();
  void reloadWads();

  bool canRemoveWads() const;
  bool canMoveWadsUp() const;
  bool canMoveWadsDown() const;
  bool canReloadWads() const;

private:
  void createGui();
  void doUpdateVisual(const std::vector<Model::EntityNodeBase*>& nodes) override;

private slots:
  void updateButtons();
};

} // namespace TrenchBroom::View

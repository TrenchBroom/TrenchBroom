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

#include <QWidget>

#include "Notifier.h"
#include "Result.h"
#include "mdl/Brush.h"

#include <filesystem>
#include <memory>
#include <string>
#include <vector>

namespace tb::ui
{
class MapDocument;

class DrawShapeToolExtensionPage : public QWidget
{
  Q_OBJECT
protected:
  NotifierConnection m_notifierConnection;

public:
  Notifier<> settingsDidChangeNotifier;

  explicit DrawShapeToolExtensionPage(QWidget* parent = nullptr);

protected:
  void addWidget(QWidget* widget);
  void addApplyButton(std::weak_ptr<MapDocument> document);
};

class DrawShapeToolExtension
{
protected:
  std::weak_ptr<MapDocument> m_document;

  explicit DrawShapeToolExtension(std::weak_ptr<MapDocument> document);

public:
  virtual ~DrawShapeToolExtension();
  virtual const std::string& name() const = 0;
  virtual const std::filesystem::path& iconPath() const = 0;
  virtual DrawShapeToolExtensionPage* createToolPage(QWidget* parent = nullptr) = 0;
  virtual Result<std::vector<mdl::Brush>> createBrushes(
    const vm::bbox3d& bounds) const = 0;
};

class DrawShapeToolExtensionManager
{
public:
  Notifier<size_t> currentExtensionDidChangeNotifier;

  explicit DrawShapeToolExtensionManager(
    std::vector<std::unique_ptr<DrawShapeToolExtension>> extensions);

  const std::vector<DrawShapeToolExtension*> extensions() const;

  DrawShapeToolExtension& currentExtension();
  bool setCurrentExtensionIndex(size_t currentExtensionIndex);

private:
  std::vector<std::unique_ptr<DrawShapeToolExtension>> m_extensions;
  size_t m_currentExtensionIndex = 0;
};

} // namespace tb::ui

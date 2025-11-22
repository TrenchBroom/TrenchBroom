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

#include "ui/SmartPropertyEditor.h"

#include <vector>

class QString;
class QWidget;
class QScrollArea;

namespace tb
{
namespace mdl
{
class Map;
}

namespace ui
{
class FlagsEditor;

class SmartFlagsEditor : public SmartPropertyEditor
{
  Q_OBJECT
private:
  static const size_t NumFlags = 24;
  static const size_t NumCols = 3;

  QScrollArea* m_scrolledWindow = nullptr;
  FlagsEditor* m_flagsEditor = nullptr;
  bool m_ignoreUpdates = false;

public:
  explicit SmartFlagsEditor(mdl::Map& map, QWidget* parent = nullptr);

private:
  void createGui();
  void doUpdateVisual(const std::vector<mdl::EntityNodeBase*>& nodes) override;
  void resetScrollPos();

  void getFlags(
    const std::vector<mdl::EntityNodeBase*>& nodes,
    QStringList& labels,
    QStringList& tooltips) const;
  void getFlagValues(
    const std::vector<mdl::EntityNodeBase*>& nodes, int& setFlags, int& mixedFlags) const;
  int getFlagValue(const mdl::EntityNodeBase* node) const;

  void flagChanged(size_t index, int value, int setFlag, int mixedFlag);
};

} // namespace ui
} // namespace tb

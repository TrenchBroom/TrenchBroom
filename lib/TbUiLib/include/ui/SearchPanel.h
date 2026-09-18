/*
 Copyright (C) 2026 Kristian Duske

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

#include <string>

class QLineEdit;

namespace tb::ui
{
class MapDocument;
class SignalDelayer;

/**
 * A search box that filters/selects map objects, accepting either a bare substring
 * (matched against object names) or a full search/filter query language expression
 * (e.g. `classname == "info_player_start"`). Edited text runs the query after a short
 * delay (debounced -- typing further resets the delay rather than running once per
 * keystroke), replacing the current selection with the query's result; a malformed
 * query is reported via the search box's tooltip rather than changing the selection.
 * Clearing the box entirely deselects everything.
 */
class SearchPanel : public QWidget
{
  Q_OBJECT
private:
  MapDocument& m_document;
  QLineEdit* m_searchBox = nullptr;
  SignalDelayer* m_searchDelayer = nullptr;

public:
  explicit SearchPanel(MapDocument& document, QWidget* parent = nullptr);

private:
  void createGui();
  void runQuery(const std::string& queryText);
};

} // namespace tb::ui

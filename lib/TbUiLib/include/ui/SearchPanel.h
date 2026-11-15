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
#include <vector>

class QLineEdit;

namespace tb::ui
{
class AppController;
class MapDocument;
class SearchHelpPopup;
class SignalDelayer;

/**
 * A search box that filters/selects map objects.
 *
 * Edited text runs the query after a short delay, replacing the current selection with
 * the query's result. Pressing return runs the query immediately, even if it already ran,
 * so that it replaces a selection that was changed in the meantime. While the search
 * box has focus, a popup below it shows the result of the last search and a short syntax
 * reference. Clearing the box entirely deselects everything.
 *
 * While a name or a key after a dot is typed, the popup also lists the words that
 * complete it. Up and down select one of them, tab inserts the selected one or the first
 * one, and return inserts the selected one. If none is selected, return runs the search
 * as usual. Escape hides the completions before it hides the popup.
 *
 * The panel is expanded while the search box has focus or contains text. It is up to the
 * containing widget to give an expanded panel more space.
 */
class SearchPanel : public QWidget
{
  Q_OBJECT
private:
  MapDocument& m_document;
  QLineEdit* m_searchBox = nullptr;
  SignalDelayer* m_searchDelayer = nullptr;
  SearchHelpPopup* m_popup = nullptr;
  bool m_focused = false;
  bool m_expanded = false;

  std::vector<std::string> m_propertyKeys;
  int m_completionStart = 0;
  int m_completionEnd = 0;

public:
  SearchPanel(
    AppController& appController, MapDocument& document, QWidget* parent = nullptr);
  ~SearchPanel() override;

  bool expanded() const;

signals:
  void expandedChanged(bool expanded);

protected:
  bool eventFilter(QObject* watched, QEvent* event) override;

private:
  void createGui(AppController& appController);
  void focusChanged(QWidget* old, QWidget* now);
  void showPopup();
  void updateExpanded();
  void updateCompletions();
  bool handleCompletionKey(int key);
  void insertCompletion(const QString& completion);
  void runQuery(const std::string& queryText);
};

} // namespace tb::ui

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

#include <QFrame>
#include <QString>
#include <QStringList>

class QLabel;
class QListWidget;
class QWidget;

namespace tb::ui
{
class AppController;

/**
 * A floating panel shown below the search box while it has focus. It shows the words that
 * complete the query being typed, the result of the last search and a short reference of
 * the query syntax with a link to the manual.
 *
 * Unlike PopupWindow, this doesn't grab the keyboard, so the user can keep typing into
 * the search box while it is visible.
 */
class SearchHelpPopup : public QFrame
{
  Q_OBJECT
private:
  AppController& m_appController;
  QListWidget* m_completionList = nullptr;
  QLabel* m_statusLabel = nullptr;

public:
  explicit SearchHelpPopup(AppController& appController, QWidget* parent = nullptr);

  QStringList completions() const;

  /**
   * Shows the given completions above the status, or hides the list of completions if
   * there are none. None of them is selected.
   */
  void setCompletions(const QStringList& completions);

  /**
   * Returns the selected completion, or an empty string if none is selected.
   */
  QString selectedCompletion() const;

  /**
   * Selects the next completion, or the first one if none or the last one is selected.
   */
  void selectNextCompletion();

  /**
   * Selects the previous completion, or the last one if none or the first one is
   * selected.
   */
  void selectPreviousCompletion();

  QString status() const;
  void setStatus(const QString& status);

  /**
   * Shows this popup directly below `anchor`, at least as wide as `anchor`.
   */
  void showBelow(const QWidget& anchor);

signals:
  void completionClicked(const QString& completion);

private:
  void createGui();
};

} // namespace tb::ui

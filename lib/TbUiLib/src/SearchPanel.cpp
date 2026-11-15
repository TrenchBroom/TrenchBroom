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

#include "ui/SearchPanel.h"

#include <QApplication>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QLineEdit>
#include <QStringList>

#include "mdl/EditorContext.h"
#include "mdl/Map.h"
#include "mdl/Map_Entities.h"
#include "mdl/Map_Selection.h"
#include "mdl/Selection.h"
#include "ql/Query.h"
#include "ql/QueryCompletion.h"
#include "ui/MapDocument.h"
#include "ui/SearchBox.h"
#include "ui/SearchHelpPopup.h"
#include "ui/SignalDelayer.h"

#include "kd/ranges/to.h"

#include <chrono> // IWYU pragma: keep
#include <ranges>

namespace tb::ui
{

using namespace std::chrono_literals;

namespace
{

QString statusText(const std::string& queryText, const mdl::Selection& selection)
{
  if (selection.hasNodes())
  {
    const auto count = selection.nodes.size();
    return count == 1u ? SearchPanel::tr("1 object selected")
                       : SearchPanel::tr("%1 objects selected").arg(count);
  }
  if (selection.hasBrushFaces())
  {
    const auto count = selection.brushFaces.size();
    return count == 1u ? SearchPanel::tr("1 face selected")
                       : SearchPanel::tr("%1 faces selected").arg(count);
  }
  return SearchPanel::tr("'%1' did not match anything")
    .arg(QString::fromStdString(queryText));
}

} // namespace

SearchPanel::SearchPanel(
  AppController& appController, MapDocument& document, QWidget* parent)
  : QWidget{parent}
  , m_document{document}
  , m_searchDelayer{new SignalDelayer{500ms, this}}
{
  createGui(appController);
  connect(qApp, &QApplication::focusChanged, this, &SearchPanel::focusChanged);
}

SearchPanel::~SearchPanel()
{
  // destroying the popup can change the focus widget after this destructor has run
  disconnect(qApp, &QApplication::focusChanged, this, &SearchPanel::focusChanged);
}

bool SearchPanel::expanded() const
{
  return m_expanded;
}

bool SearchPanel::eventFilter(QObject* watched, QEvent* event)
{
  if (!m_popup->isVisible())
  {
    return QWidget::eventFilter(watched, event);
  }

  if (watched == m_searchBox && event->type() == QEvent::KeyPress)
  {
    const auto key = static_cast<QKeyEvent*>(event)->key();
    if (handleCompletionKey(key))
    {
      return true;
    }
    if (key == Qt::Key_Escape)
    {
      m_popup->hide();
      return true;
    }
  }
  else if (event->type() == QEvent::Move || event->type() == QEvent::Resize)
  {
    // a widget only receives move events when its position relative to its parent
    // changes, so the search box's ancestors must be watched too
    if (auto* widget = qobject_cast<QWidget*>(watched);
        widget == m_searchBox || (widget && widget->isAncestorOf(m_searchBox)))
    {
      m_popup->showBelow(*m_searchBox);
    }
  }

  return QWidget::eventFilter(watched, event);
}

void SearchPanel::createGui(AppController& appController)
{
  m_popup = new SearchHelpPopup{appController, this};

  m_searchBox = createSearchBox();
  m_searchBox->setObjectName("SearchPanel_SearchBox");
  m_searchBox->setPlaceholderText(tr("Type to search..."));
  m_searchBox->setMinimumWidth(200);
  m_searchBox->installEventFilter(this);

  connect(
    m_searchBox, &QLineEdit::textEdited, m_searchDelayer, &SignalDelayer::queueSignal);
  connect(m_searchBox, &QLineEdit::textEdited, this, &SearchPanel::updateCompletions);
  connect(m_searchBox, &QLineEdit::cursorPositionChanged, this, [&] {
    // moving the cursor only updates the completions that are already shown
    if (!m_popup->completions().isEmpty())
    {
      updateCompletions();
    }
  });
  connect(
    m_popup, &SearchHelpPopup::completionClicked, this, [&](const auto& completion) {
      insertCompletion(completion);
      m_searchBox->setFocus();
    });
  connect(m_searchBox, &QLineEdit::textChanged, this, &SearchPanel::updateExpanded);
  connect(m_searchBox, &QLineEdit::returnPressed, this, [&] {
    // run the search even if it isn't pending, since the selection may have changed
    m_searchDelayer->queueSignal();
    m_searchDelayer->flushSignal();
  });
  connect(m_searchDelayer, &SignalDelayer::processSignal, this, [&] {
    runQuery(m_searchBox->text().trimmed().toStdString());
  });

  auto* layout = new QHBoxLayout{};
  layout->setContentsMargins(0, 0, 0, 0);
  layout->addWidget(m_searchBox);
  setLayout(layout);
}

void SearchPanel::focusChanged(QWidget*, QWidget* now)
{
  const auto searchBoxHasFocus = now == m_searchBox;
  const auto popupHasFocus = now && (now == m_popup || m_popup->isAncestorOf(now));
  if (searchBoxHasFocus)
  {
    // the map can only change while the focus is elsewhere
    m_propertyKeys = mdl::allEntityPropertyKeys(m_document.map());
    m_popup->setCompletions({});
    showPopup();
  }
  else if (!popupHasFocus)
  {
    m_popup->hide();
    m_searchDelayer->flushSignal();
  }

  m_focused = searchBoxHasFocus || popupHasFocus;
  updateExpanded();
}

void SearchPanel::showPopup()
{
  // the search box may have been reparented since the popup was last shown, and
  // installing an event filter twice has no effect
  for (auto* widget = m_searchBox->parentWidget(); widget;
       widget = widget->parentWidget())
  {
    widget->installEventFilter(this);
  }
  m_popup->showBelow(*m_searchBox);
}

void SearchPanel::updateExpanded()
{
  if (const auto expanded = m_focused || !m_searchBox->text().isEmpty();
      expanded != m_expanded)
  {
    m_expanded = expanded;
    emit expandedChanged(m_expanded);
  }
}

void SearchPanel::updateCompletions()
{
  // the completions are shown in the popup, which stays hidden after escape
  if (!m_popup->isVisible())
  {
    return;
  }

  // the query is completed as UTF-8, but the search box counts UTF-16 code units
  const auto text = m_searchBox->text();
  const auto cursorPosition = m_searchBox->cursorPosition();
  const auto utf8Text = text.toStdString();
  const auto completion = ql::completeQuery(
    utf8Text, size_t(text.left(cursorPosition).toUtf8().size()), m_propertyKeys);

  const auto toPosition = [&](const size_t offset) {
    return int(QString::fromStdString(utf8Text.substr(0, offset)).size());
  };
  m_completionStart = toPosition(completion.start);
  m_completionEnd = toPosition(completion.end);

  const auto typedName = text.mid(m_completionStart, cursorPosition - m_completionStart);
  const auto words = completion.words | std::views::transform(QString::fromStdString)
                     | kdl::ranges::to<QStringList>();

  // keys are offered as soon as a dot is typed, other words once a name is begun
  const auto afterDot = m_completionStart > 0 && text[m_completionStart - 1] == u'.';
  const auto onlyTypedName = words == QStringList{typedName};
  m_popup->setCompletions(
    (!typedName.isEmpty() || afterDot) && !onlyTypedName ? words : QStringList{});
}

bool SearchPanel::handleCompletionKey(const int key)
{
  const auto completions = m_popup->completions();
  if (completions.isEmpty())
  {
    return false;
  }

  switch (key)
  {
  case Qt::Key_Down:
    m_popup->selectNextCompletion();
    return true;
  case Qt::Key_Up:
    m_popup->selectPreviousCompletion();
    return true;
  case Qt::Key_Tab: {
    const auto selected = m_popup->selectedCompletion();
    insertCompletion(!selected.isEmpty() ? selected : completions.front());
    return true;
  }
  case Qt::Key_Return:
  case Qt::Key_Enter:
    if (const auto selected = m_popup->selectedCompletion(); !selected.isEmpty())
    {
      insertCompletion(selected);
      return true;
    }
    // without a selected completion, return runs the search
    m_popup->setCompletions({});
    return false;
  case Qt::Key_Escape:
    m_popup->setCompletions({});
    return true;
  default:
    return false;
  }
}

void SearchPanel::insertCompletion(const QString& completion)
{
  m_searchBox->setSelection(m_completionStart, m_completionEnd - m_completionStart);
  m_searchBox->insert(completion);
  m_popup->setCompletions({});
}

void SearchPanel::runQuery(const std::string& queryText)
{
  auto& map = m_document.map();

  if (!map.editorContext().canChangeSelection())
  {
    m_popup->setStatus(tr("The current tool prevents changing the selection"));
  }
  else if (queryText.empty())
  {
    mdl::deselectAll(map);
    m_popup->setStatus("");
  }
  else
  {
    ql::executeQuery(map, ql::parseQuery(queryText));
    m_popup->setStatus(statusText(queryText, map.selection()));
  }
}

} // namespace tb::ui

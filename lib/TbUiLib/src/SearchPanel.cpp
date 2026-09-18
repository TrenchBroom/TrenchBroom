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

#include "mdl/Map.h"
#include "mdl/Map_Selection.h"
#include "mdl/Selection.h"
#include "ql/Query.h"
#include "ui/MapDocument.h"
#include "ui/SearchBox.h"
#include "ui/SearchHelpPopup.h"
#include "ui/SignalDelayer.h"

#include <chrono> // IWYU pragma: keep

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
    if (static_cast<QKeyEvent*>(event)->key() == Qt::Key_Escape)
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
  m_searchBox->setPlaceholderText(tr("Type to search..."));
  m_searchBox->setMinimumWidth(200);
  m_searchBox->installEventFilter(this);

  connect(
    m_searchBox, &QLineEdit::textEdited, m_searchDelayer, &SignalDelayer::queueSignal);
  connect(m_searchBox, &QLineEdit::textChanged, this, &SearchPanel::updateExpanded);
  connect(
    m_searchBox, &QLineEdit::returnPressed, m_searchDelayer, &SignalDelayer::flushSignal);
  connect(m_searchDelayer, &SignalDelayer::processSignal, this, [&] {
    runQuery(m_searchBox->text().toStdString());
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
    showPopup();
  }
  else if (!popupHasFocus)
  {
    m_popup->hide();
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

void SearchPanel::runQuery(const std::string& queryText)
{
  auto& map = m_document.map();

  if (queryText.empty())
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

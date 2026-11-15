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

#include "ui/SearchHelpPopup.h"

#include <QLabel>
#include <QListWidget>
#include <QVBoxLayout>

#include "ui/AppController.h"
#include "ui/QStyleUtils.h"
#include "ui/ViewConstants.h"

#include <algorithm>

namespace tb::ui
{
namespace
{

constexpr auto MaxVisibleCompletions = qsizetype{8};

} // namespace

SearchHelpPopup::SearchHelpPopup(AppController& appController, QWidget* parent)
  // Qt::Tool windows are plain top-level windows, and on Wayland, clients cannot
  // position those at all -- the compositor decides where they end up. Qt::ToolTip
  // windows are instead positioned as popups anchored to their parent, which Wayland
  // does support, and which also still doesn't grab the keyboard like Qt::Popup would.
  : QFrame{parent, Qt::ToolTip | Qt::FramelessWindowHint}
  , m_appController{appController}
{
  setAttribute(Qt::WA_ShowWithoutActivating);
  setFocusPolicy(Qt::ClickFocus);
  setFrameShape(QFrame::StyledPanel);
  createGui();

  // Make this the focus widget of its window. When a window without a focus widget is
  // activated by a click, Qt clears the focus instead of moving it into that window, and
  // the popup is hidden before the click is processed.
  setFocus();
}

QStringList SearchHelpPopup::completions() const
{
  auto result = QStringList{};
  for (int row = 0; row < m_completionList->count(); ++row)
  {
    result.append(m_completionList->item(row)->text());
  }
  return result;
}

void SearchHelpPopup::setCompletions(const QStringList& completions)
{
  m_completionList->clear();
  m_completionList->addItems(completions);
  m_completionList->setVisible(!completions.isEmpty());

  if (!completions.isEmpty())
  {
    const auto visibleRows = int(std::min(completions.size(), MaxVisibleCompletions));
    m_completionList->setFixedHeight(
      visibleRows * m_completionList->sizeHintForRow(0)
      + 2 * m_completionList->frameWidth());
  }

  resize(width(), sizeHint().height());
}

QString SearchHelpPopup::selectedCompletion() const
{
  const auto* item = m_completionList->currentItem();
  return item ? item->text() : QString{};
}

void SearchHelpPopup::selectNextCompletion()
{
  const auto row = m_completionList->currentRow();
  m_completionList->setCurrentRow(row + 1 < m_completionList->count() ? row + 1 : 0);
}

void SearchHelpPopup::selectPreviousCompletion()
{
  const auto row = m_completionList->currentRow();
  m_completionList->setCurrentRow(row > 0 ? row - 1 : m_completionList->count() - 1);
}

QString SearchHelpPopup::status() const
{
  return m_statusLabel->text();
}

void SearchHelpPopup::setStatus(const QString& status)
{
  m_statusLabel->setText(status);
  m_statusLabel->setVisible(!status.isEmpty());
  resize(width(), sizeHint().height());
}

void SearchHelpPopup::showBelow(const QWidget& anchor)
{
  resize(std::max(anchor.width(), sizeHint().width()), sizeHint().height());
  move(anchor.mapToGlobal(QPoint{0, anchor.height()}));
  show();
}

void SearchHelpPopup::createGui()
{
  m_completionList = new QListWidget{};
  m_completionList->setObjectName("SearchHelpPopup_CompletionList");
  // the search box keeps the focus while the completions are navigated with the keyboard
  m_completionList->setFocusPolicy(Qt::NoFocus);
  m_completionList->setUniformItemSizes(true);
  m_completionList->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  m_completionList->setVisible(false);

  connect(m_completionList, &QListWidget::itemClicked, this, [&](const auto* item) {
    emit completionClicked(item->text());
  });

  m_statusLabel = new QLabel{};
  m_statusLabel->setObjectName("SearchHelpPopup_StatusLabel");
  m_statusLabel->setTextFormat(Qt::PlainText);
  m_statusLabel->setWordWrap(true);
  m_statusLabel->setVisible(false);
  setEmphasizedStyle(m_statusLabel);

  auto* helpLabel = new QLabel{tr(R"(
<p>Type any text to find objects whose name, classname, properties, materials or tags
contain it, or whose layer or group name contains it. Use <code>*</code> and
<code>?</code> as wildcards.</p>

<p>Or write a query:</p>
<table cellspacing="2">
<tr><td><code>classname is "light"</code></td></tr>
<tr><td><code>type is brush</code></td></tr>
<tr><td><code>name like "door*"</code></td></tr>
<tr><td><code>materials like "*trigger*"</code></td></tr>
<tr><td><code>tags contains "Detail"</code></td></tr>
<tr><td><code>visible &amp;&amp; !locked</code></td></tr>
<tr><td><code>material like "sky*"</code> (selects faces)</td></tr>
</table>

<p>Operators: <code>== != &lt; &gt; &lt;= &gt;= &amp;&amp; || ! like contains in is</code></p>

<p>Types: <code>world layer group entity brush patch face</code></p>

<p>See the <a href="query_language">manual</a> for the full query language.</p>
)")};
  helpLabel->setObjectName("SearchHelpPopup_HelpLabel");
  helpLabel->setTextFormat(Qt::RichText);
  helpLabel->setWordWrap(true);
  setInfoStyle(helpLabel);

  // the target of a link is the section of the manual to show
  connect(helpLabel, &QLabel::linkActivated, this, [&](const auto& section) {
    m_appController.showManual(section);
  });

  auto* layout = new QVBoxLayout{};
  layout->setContentsMargins(
    LayoutConstants::WideHMargin,
    LayoutConstants::WideVMargin,
    LayoutConstants::WideHMargin,
    LayoutConstants::WideVMargin);
  layout->setSpacing(LayoutConstants::MediumVMargin);
  layout->addWidget(m_completionList);
  layout->addWidget(m_statusLabel);
  layout->addWidget(helpLabel);
  setLayout(layout);
}

} // namespace tb::ui

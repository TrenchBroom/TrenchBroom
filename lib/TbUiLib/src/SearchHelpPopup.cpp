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
#include <QVBoxLayout>

#include "ui/AppController.h"
#include "ui/QStyleUtils.h"
#include "ui/ViewConstants.h"

#include <algorithm>

namespace tb::ui
{

SearchHelpPopup::SearchHelpPopup(AppController& appController, QWidget* parent)
  : QFrame{parent, Qt::Tool | Qt::FramelessWindowHint}
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
  m_statusLabel = new QLabel{};
  m_statusLabel->setObjectName("statusLabel");
  m_statusLabel->setWordWrap(true);
  m_statusLabel->setVisible(false);
  setEmphasizedStyle(m_statusLabel);

  auto* helpLabel = new QLabel{tr(R"(
<p>Type any text to find objects whose name, classname, properties or materials contain
it. Use <code>*</code> and <code>?</code> as wildcards.</p>

<p>Or write a query:</p>
<table cellspacing="2">
<tr><td><code>classname is "light"</code></td></tr>
<tr><td><code>type is "brush"</code></td></tr>
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
  layout->addWidget(m_statusLabel);
  layout->addWidget(helpLabel);
  setLayout(layout);
}

} // namespace tb::ui

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

#include <QHBoxLayout>
#include <QLineEdit>

#include "mdl/Map.h"
#include "mdl/Map_Selection.h"
#include "mdl/Transaction.h"
#include "ql/Query.h"
#include "ui/MapDocument.h"
#include "ui/SearchBox.h"
#include "ui/SignalDelayer.h"

#include "kd/overload.h"

#include <chrono> // IWYU pragma: keep

namespace tb::ui
{

using namespace std::chrono_literals;

SearchPanel::SearchPanel(MapDocument& document, QWidget* parent)
  : QWidget{parent}
  , m_document{document}
  , m_searchDelayer{new SignalDelayer{500ms, this}}
{
  createGui();
}

void SearchPanel::createGui()
{
  m_searchBox = createSearchBox();
  m_searchBox->setPlaceholderText(tr("Search or filter..."));
  m_searchBox->setMinimumWidth(200);
  connect(
    m_searchBox, &QLineEdit::textEdited, this, [&]() { m_searchDelayer->queueSignal(); });
  connect(m_searchDelayer, &SignalDelayer::processSignal, this, [&]() {
    runQuery(m_searchBox->text().toStdString());
  });

  auto* layout = new QHBoxLayout{};
  layout->setContentsMargins(0, 0, 0, 0);
  layout->addWidget(m_searchBox);
  setLayout(layout);
}

void SearchPanel::runQuery(const std::string& queryText)
{
  m_searchBox->setToolTip("");

  const auto textToParse = ql::queryTextFrom(queryText);
  if (!textToParse)
  {
    mdl::deselectAll(m_document.map());
    return;
  }

  const auto parsed = ql::parseQuery(*textToParse);
  if (parsed.is_error())
  {
    const auto message = std::visit([](const auto& e) { return e.msg; }, parsed.error());
    m_searchBox->setToolTip(QString::fromStdString(message));
    return;
  }

  auto& map = m_document.map();
  const auto result = ql::executeQuery(map, parsed.value());

  auto transaction = mdl::Transaction{map, "Search"};
  mdl::deselectAll(map);
  std::visit(
    kdl::overload(
      [&](const std::vector<mdl::Node*>& nodes) { mdl::selectNodes(map, nodes); },
      [&](const std::vector<mdl::BrushFaceHandle>& handles) {
        mdl::selectBrushFaces(map, handles);
      }),
    result);
  transaction.commit();
}

} // namespace tb::ui

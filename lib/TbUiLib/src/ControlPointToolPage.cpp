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

#include "ui/ControlPointToolPage.h"

#include <QBoxLayout>
#include <QLabel>
#include <QSpinBox>
#include <QValidator>

#include "mdl/CommandProcessor.h"
#include "mdl/Map.h"
#include "mdl/Map_Patches.h"
#include "mdl/PatchNode.h" // IWYU pragma: keep
#include "mdl/Transaction.h"
#include "ui/MapDocument.h"

#include "kd/range_fold.h"

namespace tb::ui
{

namespace
{

class OddSpinBox : public QSpinBox
{
public:
  using QSpinBox::QSpinBox;

protected:
  QValidator::State validate(QString& input, int& pos) const override
  {
    const auto state = QSpinBox::validate(input, pos);
    return state == QValidator::Acceptable && valueFromText(input) % 2 == 0
             ? QValidator::Intermediate
             : state;
  }

  void fixup(QString& input) const override
  {
    auto value = valueFromText(input);
    value += (value % 2 == 0) ? 1 : 0;
    input = textFromValue(value);
  }
};

} // namespace

ControlPointToolPage::ControlPointToolPage(MapDocument& document, QWidget* parent)
  : QWidget{parent}
  , m_document{document}
  , m_rows{new OddSpinBox{this}}
  , m_cols{new OddSpinBox{this}}
{
  m_rows->setMinimum(3);
  m_rows->setSingleStep(2);
  m_cols->setMinimum(3);
  m_cols->setSingleStep(2);

  auto* layout = new QHBoxLayout{};
  layout->setContentsMargins(0, 0, 0, 0);

  layout->addWidget(new QLabel{tr("Rows:")});
  layout->addWidget(m_rows);
  layout->addWidget(new QLabel{tr("Columns:")});
  layout->addWidget(m_cols);
  layout->addStretch();

  setLayout(layout);

  connect(m_rows, &QSpinBox::valueChanged, this, [this](auto) { updateControlPoints(); });
  connect(m_cols, &QSpinBox::valueChanged, this, [this](auto) { updateControlPoints(); });
}

void ControlPointToolPage::connectObservers()
{
  m_notifierConnection +=
    m_document.documentDidChangeNotifier.connect([this]() { updateControls(); });
  m_notifierConnection += m_document.selectionDidChangeNotifier.connect(
    [this](const auto&) { updateControls(); });
}

void ControlPointToolPage::updateControls()
{
  const auto& map = m_document.map();
  if (map.selection().hasPatches())
  {
    const auto returnIfEqual = [](const auto& lhs, const auto& rhs) {
      return lhs && *lhs == rhs ? lhs : std::nullopt;
    };

    const auto& rowCounts =
      map.selection().patches | std::views::transform([](const auto* patchNode) {
        return patchNode->patch().pointRowCount();
      });

    const auto& colCounts =
      map.selection().patches | std::views::transform([](const auto* patchNode) {
        return patchNode->patch().pointColumnCount();
      });

    const auto rows = kdl::fold_left(
      rowCounts | std::views::drop(1),
      std::optional<size_t>{rowCounts.front()},
      returnIfEqual);

    const auto cols = kdl::fold_left(
      colCounts | std::views::drop(1),
      std::optional<size_t>{colCounts.front()},
      returnIfEqual);

    m_rows->setEnabled(rows != std::nullopt);
    m_cols->setEnabled(cols != std::nullopt);

    m_rows->setValue(int(rows.value_or(3)));
    m_cols->setValue(int(cols.value_or(3)));
  }
  else
  {
    m_rows->setEnabled(false);
    m_cols->setEnabled(false);
  }
}

void ControlPointToolPage::updateControlPoints()
{
  mdl::resamplePatches(
    m_document.map(), size_t(m_rows->value()), size_t(m_cols->value()));
}

} // namespace tb::ui

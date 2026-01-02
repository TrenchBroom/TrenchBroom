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

#include "ui/ReplaceMaterialDialog.h"

#include <QBoxLayout>
#include <QDialogButtonBox>
#include <QMessageBox>
#include <QPushButton>

#include "gl/Material.h"
#include "mdl/BrushFace.h"
#include "mdl/BrushFaceHandle.h"
#include "mdl/Map.h"
#include "mdl/Map_Brushes.h"
#include "mdl/Map_Selection.h"
#include "mdl/NodeQueries.h"
#include "mdl/PushSelection.h"
#include "mdl/Transaction.h"
#include "mdl/UpdateBrushFaceAttributes.h" // IWYU pragma: keep
#include "mdl/WorldNode.h"                 // IWYU pragma: keep
#include "ui/BorderLine.h"
#include "ui/DialogButtonLayout.h"
#include "ui/MapDocument.h"
#include "ui/MaterialBrowser.h"
#include "ui/QStyleUtils.h"
#include "ui/TitledPanel.h"

#include "kd/contracts.h"
#include "kd/ranges/to.h"

#include <fmt/format.h>

#include <ranges>
#include <vector>

namespace tb::ui
{
namespace
{

void replaceMaterials(
  mdl::Map& map,
  const std::vector<mdl::BrushFaceHandle>& faces,
  const std::string& materialName)
{
  const auto pushSelection = mdl::PushSelection{map};

  auto transaction = mdl::Transaction{map, "Replace Materials"};
  selectBrushFaces(map, faces);
  if (!setBrushFaceAttributes(map, {.materialName = materialName}))
  {
    transaction.cancel();
    return;
  }
  transaction.commit();
}

} // namespace

ReplaceMaterialDialog::ReplaceMaterialDialog(
  MapDocument& document, gl::ContextManager& contextManager, QWidget* parent)
  : QDialog{parent}
  , m_document{document}
{
  createGui(contextManager);
}

void ReplaceMaterialDialog::accept()
{
  const auto* subject = m_subjectBrowser->selectedMaterial();
  contract_assert(subject != nullptr);

  const auto* replacement = m_replacementBrowser->selectedMaterial();
  contract_assert(replacement != nullptr);

  if (const auto faces = getApplicableFaces(); !faces.empty())
  {
    replaceMaterials(m_document.map(), faces, replacement->name());

    const auto msg = fmt::format(
      "Replaced material '{}' with '{}' on {} faces.",
      subject->name(),
      replacement->name(),
      faces.size());

    QMessageBox::information(this, tr("Replace Succeeded"), QString::fromStdString(msg));
  }
  else
  {
    QMessageBox::warning(
      this,
      tr("Replace Failed"),
      tr("None of the selected faces has the selected material"));
  }
}

std::vector<mdl::BrushFaceHandle> ReplaceMaterialDialog::getApplicableFaces() const
{
  const auto* subject = m_subjectBrowser->selectedMaterial();
  contract_assert(subject != nullptr);

  auto& map = m_document.map();
  auto faces = map.selection().allBrushFaces();
  if (faces.empty())
  {
    faces = mdl::collectBrushFaces({&map.worldNode()});
  }

  return faces | std::views::filter([&](const auto& handle) {
           return handle.face().material() == subject;
         })
         | kdl::ranges::to<std::vector>();
}

void ReplaceMaterialDialog::createGui(gl::ContextManager& contextManager)
{
  setWindowIconTB(this);
  setWindowTitle(tr("Replace Material"));

  auto* subjectPanel = new TitledPanel{tr("Find")};
  m_subjectBrowser = new MaterialBrowser{m_document, contextManager};
  m_subjectBrowser->setHideUnused(true);
  connect(
    m_subjectBrowser,
    &MaterialBrowser::materialSelected,
    this,
    &ReplaceMaterialDialog::subjectSelected);

  auto* subjectPanelLayout = new QVBoxLayout{};
  subjectPanelLayout->setContentsMargins(QMargins{});
  subjectPanelLayout->setSpacing(0);
  subjectPanelLayout->addWidget(m_subjectBrowser);
  subjectPanel->getPanel()->setLayout(subjectPanelLayout);

  auto* replacementPanel = new TitledPanel{tr("Replace with")};
  m_replacementBrowser = new MaterialBrowser{m_document, contextManager};
  m_replacementBrowser->setSelectedMaterial(nullptr); // Override the current material.
  connect(
    m_replacementBrowser,
    &MaterialBrowser::materialSelected,
    this,
    &ReplaceMaterialDialog::replacementSelected);

  auto* replacementPanelLayout = new QVBoxLayout{};
  replacementPanelLayout->setContentsMargins(QMargins{});
  replacementPanelLayout->setSpacing(0);
  replacementPanelLayout->addWidget(m_replacementBrowser, 1);
  replacementPanel->getPanel()->setLayout(replacementPanelLayout);

  auto* upperLayout = new QHBoxLayout{};
  upperLayout->setContentsMargins(QMargins{});
  upperLayout->setSpacing(0);
  upperLayout->addWidget(subjectPanel, 1);
  upperLayout->addWidget(new BorderLine{BorderLine::Direction::Vertical}, 0);
  upperLayout->addWidget(replacementPanel, 1);

  auto* buttonBox = new QDialogButtonBox{};
  m_replaceButton = buttonBox->addButton(tr("Replace"), QDialogButtonBox::AcceptRole);
  m_replaceButton->setToolTip(tr("Perform replacement on all selected faces"));
  m_replaceButton->setEnabled(false);
  auto* closeButton = buttonBox->addButton(tr("Close"), QDialogButtonBox::RejectRole);
  closeButton->setToolTip(tr("Close this window"));

  connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
  connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

  auto* outerLayout = new QVBoxLayout{};
  outerLayout->setContentsMargins(QMargins{});
  outerLayout->setSpacing(0);
  outerLayout->addLayout(upperLayout, 1);
  outerLayout->addLayout(wrapDialogButtonBox(buttonBox), 0);
  insertTitleBarSeparator(outerLayout);

  setLayout(outerLayout);

  setMinimumSize(650, 450);
}

void ReplaceMaterialDialog::subjectSelected(const gl::Material* /* subject */)
{
  updateReplaceButton();
}

void ReplaceMaterialDialog::replacementSelected(const gl::Material* /* replacement */)
{
  updateReplaceButton();
}

void ReplaceMaterialDialog::updateReplaceButton()
{
  const auto* subject = m_subjectBrowser->selectedMaterial();
  const auto* replacement = m_replacementBrowser->selectedMaterial();
  m_replaceButton->setEnabled(subject && replacement);
}

} // namespace tb::ui

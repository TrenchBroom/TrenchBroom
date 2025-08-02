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

#include "ReplaceMaterialDialog.h"

#include <QDialogButtonBox>
#include <QMessageBox>
#include <QPushButton>

#include "mdl/BrushFace.h"
#include "mdl/BrushFaceHandle.h"
#include "mdl/ChangeBrushFaceAttributesRequest.h"
#include "mdl/Map.h"
#include "mdl/Material.h"
#include "mdl/NodeQueries.h"
#include "mdl/PushSelection.h"
#include "mdl/Transaction.h"
#include "mdl/WorldNode.h" // IWYU pragma: keep
#include "ui/BorderLine.h"
#include "ui/MapDocument.h"
#include "ui/MaterialBrowser.h"
#include "ui/QtUtils.h"
#include "ui/TitledPanel.h"

#include "kdl/memory_utils.h"
#include "kdl/range_to_vector.h"

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
  auto request = mdl::ChangeBrushFaceAttributesRequest{};
  request.setMaterialName(materialName);

  const auto pushSelection = mdl::PushSelection{map};

  auto transaction = mdl::Transaction{map, "Replace Materials"};
  map.selectBrushFaces(faces);
  if (!map.setFaceAttributes(request))
  {
    transaction.cancel();
    return;
  }
  transaction.commit();
}

} // namespace

ReplaceMaterialDialog::ReplaceMaterialDialog(
  std::weak_ptr<MapDocument> document, GLContextManager& contextManager, QWidget* parent)
  : QDialog{parent}
  , m_document{std::move(document)}
{
  createGui(contextManager);
}

void ReplaceMaterialDialog::accept()
{
  const auto* subject = m_subjectBrowser->selectedMaterial();
  ensure(subject != nullptr, "subject is null");

  const auto* replacement = m_replacementBrowser->selectedMaterial();
  ensure(replacement != nullptr, "replacement is null");

  if (const auto faces = getApplicableFaces(); !faces.empty())
  {
    auto& map = kdl::mem_lock(m_document)->map();
    replaceMaterials(map, faces, replacement->name());

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
  ensure(subject != nullptr, "subject is null");

  const auto& map = kdl::mem_lock(m_document)->map();
  auto faces = map.selection().allBrushFaces();
  if (faces.empty())
  {
    faces = mdl::collectBrushFaces({map.world()});
  }

  return faces | std::views::filter([&](const auto& handle) {
           return handle.face().material() == subject;
         })
         | kdl::to_vector;
}

void ReplaceMaterialDialog::createGui(GLContextManager& contextManager)
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

void ReplaceMaterialDialog::subjectSelected(const mdl::Material* /* subject */)
{
  updateReplaceButton();
}

void ReplaceMaterialDialog::replacementSelected(const mdl::Material* /* replacement */)
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

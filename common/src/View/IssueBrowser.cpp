/*
 Copyright (C) 2010-2017 Kristian Duske

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

#include "IssueBrowser.h"

#include "Model/Issue.h"
#include "Model/Validator.h"
#include "Model/WorldNode.h"
#include "View/FlagsPopupEditor.h"
#include "View/IssueBrowserView.h"
#include "View/MapDocument.h"

#include <kdl/memory_utils.h>

#include <QCheckBox>
#include <QList>
#include <QStringList>
#include <QVBoxLayout>

namespace TrenchBroom
{
namespace View
{
IssueBrowser::IssueBrowser(std::weak_ptr<MapDocument> document, QWidget* parent)
  : TabBookPage(parent)
  , m_document(document)
  , m_view(new IssueBrowserView(m_document))
  , m_showHiddenIssuesCheckBox(nullptr)
  , m_filterEditor(nullptr)
{
  auto* sizer = new QVBoxLayout();
  sizer->setContentsMargins(0, 0, 0, 0);
  sizer->addWidget(m_view);
  setLayout(sizer);

  connectObservers();
}

QWidget* IssueBrowser::createTabBarPage(QWidget* parent)
{

  auto* barPage = new QWidget(parent);
  m_showHiddenIssuesCheckBox = new QCheckBox("Show hidden issues");
  connect(
    m_showHiddenIssuesCheckBox,
    &QCheckBox::stateChanged,
    this,
    &IssueBrowser::showHiddenIssuesChanged);

  m_filterEditor = new FlagsPopupEditor(1, nullptr, "Filter", false);
  connect(
    m_filterEditor, &FlagsPopupEditor::flagChanged, this, &IssueBrowser::filterChanged);

  auto* barPageSizer = new QHBoxLayout();
  barPageSizer->setContentsMargins(0, 0, 0, 0);
  barPageSizer->addWidget(m_showHiddenIssuesCheckBox, 0, Qt::AlignVCenter);
  barPageSizer->addWidget(m_filterEditor, 0, Qt::AlignVCenter);
  barPage->setLayout(barPageSizer);

  return barPage;
}

void IssueBrowser::connectObservers()
{
  auto document = kdl::mem_lock(m_document);
  m_notifierConnection +=
    document->documentWasSavedNotifier.connect(this, &IssueBrowser::documentWasSaved);
  m_notifierConnection += document->documentWasNewedNotifier.connect(
    this, &IssueBrowser::documentWasNewedOrLoaded);
  m_notifierConnection += document->documentWasLoadedNotifier.connect(
    this, &IssueBrowser::documentWasNewedOrLoaded);
  m_notifierConnection +=
    document->nodesWereAddedNotifier.connect(this, &IssueBrowser::nodesWereAdded);
  m_notifierConnection +=
    document->nodesWereRemovedNotifier.connect(this, &IssueBrowser::nodesWereRemoved);
  m_notifierConnection +=
    document->nodesDidChangeNotifier.connect(this, &IssueBrowser::nodesDidChange);
  m_notifierConnection += document->brushFacesDidChangeNotifier.connect(
    this, &IssueBrowser::brushFacesDidChange);
}

void IssueBrowser::documentWasNewedOrLoaded(MapDocument*)
{
  updateFilterFlags();
  m_view->reload();
}

void IssueBrowser::documentWasSaved(MapDocument*)
{
  m_view->update();
}

void IssueBrowser::nodesWereAdded(const std::vector<Model::Node*>&)
{
  m_view->reload();
}

void IssueBrowser::nodesWereRemoved(const std::vector<Model::Node*>&)
{
  m_view->reload();
}

void IssueBrowser::nodesDidChange(const std::vector<Model::Node*>&)
{
  m_view->reload();
}

void IssueBrowser::brushFacesDidChange(const std::vector<Model::BrushFaceHandle>&)
{
  m_view->reload();
}

void IssueBrowser::issueIgnoreChanged(Model::Issue*)
{
  m_view->update();
}

void IssueBrowser::updateFilterFlags()
{
  auto document = kdl::mem_lock(m_document);
  const Model::WorldNode* world = document->world();
  const auto validators = world->registeredValidators();

  QList<int> flags;
  QStringList labels;

  for (const auto* validator : validators)
  {
    const auto flag = validator->type();
    const auto& description = validator->description();

    flags.push_back(flag);
    labels.push_back(QString::fromStdString(description));
  }

  m_filterEditor->setFlags(flags, labels);
  m_view->setHiddenIssueTypes(0);
  m_filterEditor->setFlagValue(~0);
}

void IssueBrowser::showHiddenIssuesChanged()
{
  m_view->setShowHiddenIssues(m_showHiddenIssuesCheckBox->isChecked());
}

void IssueBrowser::filterChanged(
  const size_t /* index */,
  const int /* value */,
  const int setFlag,
  const int /* mixedFlag */)
{
  m_view->setHiddenIssueTypes(~setFlag);
}
} // namespace View
} // namespace TrenchBroom

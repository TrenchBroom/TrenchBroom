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

#include "IssueBrowser.h"

#include <QCheckBox>
#include <QList>
#include <QStringList>
#include <QVBoxLayout>

#include "mdl/Issue.h"
#include "mdl/Map.h"
#include "mdl/Validator.h"
#include "mdl/WorldNode.h"
#include "ui/FlagsPopupEditor.h"
#include "ui/IssueBrowserView.h"
#include "ui/MapDocument.h"

#include "kdl/memory_utils.h"

#include <utility>

namespace tb::ui
{

IssueBrowser::IssueBrowser(std::weak_ptr<MapDocument> document, QWidget* parent)
  : TabBookPage{parent}
  , m_document{std::move(document)}
  , m_view{new IssueBrowserView{m_document}}
{
  auto* sizer = new QVBoxLayout{};
  sizer->setContentsMargins(0, 0, 0, 0);
  sizer->addWidget(m_view);
  setLayout(sizer);

  connectObservers();
}

QWidget* IssueBrowser::createTabBarPage(QWidget* parent)
{
  auto* barPage = new QWidget{parent};
  m_showHiddenIssuesCheckBox = new QCheckBox{"Show hidden issues"};
  connect(
    m_showHiddenIssuesCheckBox,
    &QCheckBox::checkStateChanged,
    this,
    &IssueBrowser::showHiddenIssuesChanged);

  m_filterEditor = new FlagsPopupEditor{1, "Filter", false};
  connect(
    m_filterEditor, &FlagsPopupEditor::flagChanged, this, &IssueBrowser::filterChanged);

  auto* barPageSizer = new QHBoxLayout{};
  barPageSizer->setContentsMargins(0, 0, 0, 0);
  barPageSizer->addWidget(m_showHiddenIssuesCheckBox, 0, Qt::AlignVCenter);
  barPageSizer->addWidget(m_filterEditor, 0, Qt::AlignVCenter);
  barPage->setLayout(barPageSizer);

  return barPage;
}

void IssueBrowser::connectObservers()
{
  auto& map = kdl::mem_lock(m_document)->map();
  m_notifierConnection +=
    map.mapWasCreatedNotifier.connect(this, &IssueBrowser::mapWasCreated);
  m_notifierConnection +=
    map.mapWasLoadedNotifier.connect(this, &IssueBrowser::mapWasLoaded);
  m_notifierConnection +=
    map.mapWasSavedNotifier.connect(this, &IssueBrowser::mapWasSaved);
  m_notifierConnection +=
    map.nodesWereAddedNotifier.connect(this, &IssueBrowser::nodesWereAdded);
  m_notifierConnection +=
    map.nodesWereRemovedNotifier.connect(this, &IssueBrowser::nodesWereRemoved);
  m_notifierConnection +=
    map.nodesDidChangeNotifier.connect(this, &IssueBrowser::nodesDidChange);
  m_notifierConnection +=
    map.brushFacesDidChangeNotifier.connect(this, &IssueBrowser::brushFacesDidChange);
}

void IssueBrowser::mapWasCreated(mdl::Map&)
{
  reload();
}

void IssueBrowser::mapWasLoaded(mdl::Map&)
{
  reload();
}

void IssueBrowser::mapWasSaved(mdl::Map&)
{
  m_view->update();
}

void IssueBrowser::nodesWereAdded(const std::vector<mdl::Node*>&)
{
  m_view->reload();
}

void IssueBrowser::nodesWereRemoved(const std::vector<mdl::Node*>&)
{
  m_view->reload();
}

void IssueBrowser::nodesDidChange(const std::vector<mdl::Node*>&)
{
  m_view->reload();
}

void IssueBrowser::brushFacesDidChange(const std::vector<mdl::BrushFaceHandle>&)
{
  m_view->reload();
}

void IssueBrowser::issueIgnoreChanged(mdl::Issue*)
{
  m_view->update();
}

void IssueBrowser::reload()
{
  updateFilterFlags();
  m_view->reload();
}

void IssueBrowser::updateFilterFlags()
{
  const auto& map = kdl::mem_lock(m_document)->map();
  const auto* world = map.world();
  const auto validators = world->registeredValidators();

  auto flags = QList<int>{};
  auto labels = QStringList{};

  for (const auto* validator : validators)
  {
    flags.push_back(validator->type());
    labels.push_back(QString::fromStdString(validator->description()));
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

} // namespace tb::ui

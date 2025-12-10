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

#include "CompilationDialog.h"

#include <QApplication>
#include <QCloseEvent>
#include <QDialogButtonBox>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QTextEdit>

#include "Logger.h"
#include "TrenchBroomApp.h"
#include "mdl/CompilationProfile.h"
#include "mdl/GameInfo.h"
#include "mdl/GameManager.h"
#include "mdl/Map.h"
#include "ui/CompilationProfileManager.h"
#include "ui/CompilationRunner.h"
#include "ui/LaunchGameEngineDialog.h"
#include "ui/MapDocument.h" // IWYU pragma: keep
#include "ui/MapFrame.h"
#include "ui/QtUtils.h"
#include "ui/Splitter.h"
#include "ui/TitledPanel.h"
#include "ui/ViewConstants.h"

#include "kd/contracts.h"

namespace tb::ui
{

CompilationDialog::CompilationDialog(MapFrame* mapFrame)
  : QDialog{mapFrame}
  , m_mapFrame{mapFrame}
{
  contract_pre(mapFrame != nullptr);

  createGui();
  setMinimumSize(600, 300);
  resize(800, 600);
  updateCompileButtons();
}

void CompilationDialog::createGui()
{
  setWindowIconTB(this);
  setWindowTitle("Compile");

  auto& document = m_mapFrame->document();
  const auto& compilationConfig = document.map().gameInfo().gameConfig.compilationConfig;

  m_profileManager = new CompilationProfileManager{document, compilationConfig};

  auto* outputPanel = new TitledPanel{tr("Output")};
  m_output = new QTextEdit{};
  m_output->setReadOnly(true);
  m_output->setFont(Fonts::fixedWidthFont());

  auto* outputLayout = new QVBoxLayout{};
  outputLayout->setContentsMargins(0, 0, 0, 0);
  outputLayout->setSpacing(0);
  outputLayout->addWidget(m_output);
  outputPanel->getPanel()->setLayout(outputLayout);

  auto* splitter = new Splitter{Qt::Vertical};
  splitter->addWidget(m_profileManager);
  splitter->addWidget(m_output);
  splitter->setSizes({2, 1});

  auto* buttonBox = new QDialogButtonBox{};
  m_launchButton = buttonBox->addButton(tr("Launch..."), QDialogButtonBox::NoRole);
  m_stopCompileButton = buttonBox->addButton(tr("Stop"), QDialogButtonBox::NoRole);
  m_testCompileButton = buttonBox->addButton(tr("Test"), QDialogButtonBox::NoRole);
  m_compileButton = buttonBox->addButton(tr("Compile"), QDialogButtonBox::NoRole);
  m_closeButton = buttonBox->addButton(tr("Close"), QDialogButtonBox::RejectRole);

  m_currentRunLabel = new QLabel{""};
  m_currentRunLabel->setAlignment(Qt::AlignRight);

  auto* buttonLayout = new QHBoxLayout{};
  buttonLayout->setContentsMargins(0, 0, 0, 0);
  buttonLayout->setSpacing(LayoutConstants::WideHMargin);
  buttonLayout->addWidget(m_launchButton, 0, Qt::AlignVCenter);
  buttonLayout->addWidget(m_currentRunLabel, 1, Qt::AlignVCenter);
  buttonLayout->addWidget(buttonBox);

  auto* dialogLayout = new QVBoxLayout{};
  dialogLayout->setContentsMargins(0, 0, 0, 0);
  dialogLayout->setSpacing(0);
  dialogLayout->addWidget(splitter, 1);
  dialogLayout->addLayout(wrapDialogButtonBox(buttonLayout));
  insertTitleBarSeparator(dialogLayout);

  setLayout(dialogLayout);

  m_compileButton->setDefault(true);

  connect(
    &m_run,
    &CompilationRun::compilationStarted,
    this,
    &CompilationDialog::compilationStarted);
  connect(
    &m_run,
    &CompilationRun::compilationEnded,
    this,
    &CompilationDialog::compilationEnded);
  connect(
    m_profileManager,
    &CompilationProfileManager::selectedProfileChanged,
    this,
    &CompilationDialog::selectedProfileChanged);
  connect(
    m_profileManager,
    &CompilationProfileManager::profileChanged,
    this,
    &CompilationDialog::profileChanged);

  connect(
    m_compileButton, &QPushButton::clicked, this, [&]() { startCompilation(false); });
  connect(
    m_testCompileButton, &QPushButton::clicked, this, [&]() { startCompilation(true); });
  connect(m_stopCompileButton, &QPushButton::clicked, this, [&]() { stopCompilation(); });
  connect(m_launchButton, &QPushButton::clicked, this, [&]() {
    LaunchGameEngineDialog dialog(m_mapFrame->document(), this);
    dialog.exec();
  });
  connect(m_closeButton, &QPushButton::clicked, this, &CompilationDialog::close);
}

void CompilationDialog::keyPressEvent(QKeyEvent* event)
{
  // Dismissing the dialog with Escape doesn't invoke CompilationDialog::closeEvent, so
  // handle it here, so we can potentially block it.
  if (event->key() == Qt::Key_Escape)
  {
    close();
  }
  else
  {
    QDialog::keyPressEvent(event);
  }
}

void CompilationDialog::updateCompileButtons()
{
  const auto* profile = m_profileManager->selectedProfile();

  m_compileButton->setEnabled(!m_run.running() && profile && !profile->tasks.empty());
  m_testCompileButton->setEnabled(!m_run.running() && profile && !profile->tasks.empty());
  m_stopCompileButton->setEnabled(m_run.running());
}

void CompilationDialog::startCompilation(const bool test)
{
  saveProfile();

  if (m_run.running())
  {
    m_run.terminate();
  }
  else
  {
    const auto* profile = m_profileManager->selectedProfile();
    contract_assert(profile != nullptr);
    contract_assert(!profile->tasks.empty());

    runProfile(*profile, test) | kdl::transform_error([&](const auto& e) {
      m_output->setText(tr("Compilation failed: %1").arg(QString::fromStdString(e.msg)));
    });
  }
}

Result<void> CompilationDialog::runProfile(
  const mdl::CompilationProfile& profile, const bool test)
{
  const auto& map = m_mapFrame->document().map();
  return test ? m_run.test(profile, map, m_output) : m_run.run(profile, map, m_output);
}

void CompilationDialog::stopCompilation()
{
  if (m_run.running())
  {
    m_run.terminate();
  }
}

void CompilationDialog::closeEvent(QCloseEvent* event)
{
  if (m_run.running())
  {
    const auto result = QMessageBox::warning(
      this,
      tr("Warning"),
      tr("Closing this dialog will stop the running compilation. Are you sure?"),
      QMessageBox::Yes | QMessageBox::No,
      QMessageBox::Yes);

    if (result != QMessageBox::Yes)
    {
      event->ignore();
      return;
    }

    stopCompilation();
  }
  saveProfile();
  event->accept();
}

void CompilationDialog::compilationStarted()
{
  const auto* profile = m_profileManager->selectedProfile();
  contract_assert(profile != nullptr);

  m_currentRunLabel->setText(QString::fromStdString("Running " + profile->name));
  m_output->setText("");

  updateCompileButtons();
}

void CompilationDialog::compilationEnded()
{
  m_currentRunLabel->setText("");

  updateCompileButtons();
}

void CompilationDialog::selectedProfileChanged()
{
  updateCompileButtons();
}

void CompilationDialog::profileChanged()
{
  updateCompileButtons();
}

void CompilationDialog::saveProfile()
{
  const auto& map = m_mapFrame->document().map();
  const auto& gameName = map.gameInfo().gameConfig.name;

  auto& app = TrenchBroomApp::instance();
  auto& gameManager = app.gameManager();
  gameManager.updateCompilationConfig(
    gameName, m_profileManager->config(), m_mapFrame->logger())
    | kdl::transform_error([&](const auto& e) { m_mapFrame->logger().error() << e.msg; });
}

} // namespace tb::ui

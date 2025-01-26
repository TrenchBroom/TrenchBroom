#include "AdvancedEntitySelectDialog.h"

#include <QCheckBox>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QRegularExpression>
#include <QVBoxLayout>

#include "mdl/EntityNodeBase.h"
#include "ui/BorderLine.h"
#include "ui/DialogHeader.h"
#include "ui/EntityPropertyModel.h"
#include "ui/MapDocument.h"
#include "ui/QtUtils.h"

#include "kdl/memory_utils.h"

namespace tb::ui
{
AdvancedEntitySelectDialog::AdvancedEntitySelectDialog(
  QWidget* parent, std::weak_ptr<MapDocument> document)
  : QDialog{parent}
  , m_document{std::move(document)}
{
  initEntityTypesSection();
  initKeyValueSection();
  initByClassSection();
  initActionButtons();
  initDialog();
}

void AdvancedEntitySelectDialog::repopulateEntityClassList()
{
  if (not m_enableByClass->isChecked())
  {
    m_selectedClass->clear();
    return;
  }

  auto document = kdl::mem_lock(m_document);

  // TODO: there has to be a better way to get all entities...
  document->selectAllNodes();
  const auto entityNodes = document->allSelectedEntityNodes();
  document->deselectAll();

  QStringList classesList;

  for (const auto& node : entityNodes)
  {
    if (node->entity().pointEntity())
    {
      if (not m_pointEntities->isChecked())
      {
        // don't include point entities if point entities checkbox is not checked
        continue;
      }
    }

    if (not node->entity().pointEntity())
    {
      if (not m_brushEntities->isChecked())
      {
        // don't include brush entities if brush entities checkbox is not checked
        continue;
      }
    }

    const auto className = QString::fromStdString(node->entity().classname());

    // no need to list same entity class multiple times
    if (classesList.contains(className))
    {
      continue;
    }

    classesList.push_back(className);
  }

  classesList.sort();

  m_selectedClass->clear();
  m_selectedClass->addItems(classesList);
}

void AdvancedEntitySelectDialog::initEntityTypesSection()
{
  m_brushEntities = new QCheckBox{tr("Brush")};
  m_pointEntities = new QCheckBox{tr("Point")};

  for (auto entityType : {m_brushEntities, m_pointEntities})
  {
    entityType->setChecked(true);

    makeEmphasized(entityType);

    connect(
      entityType, &QCheckBox::toggled, this, [this, entityType](const auto checked) {
        toggleEmphasized(entityType, checked);

        repopulateEntityClassList();
        reloadSelectButtonState();
      });
  }

  m_entityTypesGroup = new QGroupBox(tr("Entity types"));
  {
    auto* entityTypesLayout = new QVBoxLayout{};

    entityTypesLayout->addWidget(m_brushEntities);
    entityTypesLayout->addSpacing(LayoutConstants::WideVMargin);

    entityTypesLayout->addWidget(m_pointEntities);
    entityTypesLayout->addSpacing(LayoutConstants::WideVMargin);

    m_entityTypesGroup->setLayout(entityTypesLayout);
  }
}

void AdvancedEntitySelectDialog::initKeyValueSection()
{
  m_key = new QLineEdit{};
  auto* equalsLabel = new QLabel{tr("=")};
  m_value = new QLineEdit{};

  for (const auto& lineEdit : {m_key, m_value})
  {
    connect(lineEdit, &QLineEdit::textChanged, this, [this](const auto&) {
      reloadSelectButtonState();
    });
  }

  auto* kvGroup = new QGroupBox(tr("Key/Value"));

  auto* entityKeyValuesLayout = new QVBoxLayout{};
  {
    m_enableByKeyValue = new QCheckBox{tr("Enabled")};

    entityKeyValuesLayout->addWidget(m_enableByKeyValue);
    entityKeyValuesLayout->addSpacing(LayoutConstants::WideVMargin);

    auto* kvLayout = new QHBoxLayout{};
    {
      kvLayout->addWidget(m_key);
      kvLayout->addWidget(equalsLabel);
      kvLayout->addWidget(m_value);

      kvGroup->setLayout(kvLayout);
    }

    kvGroup->setEnabled(false);

    entityKeyValuesLayout->addWidget(kvGroup);
    entityKeyValuesLayout->addSpacing(LayoutConstants::WideVMargin);

    m_entityKeyValuesGroup = new QGroupBox(tr("Entity key/value"));
    m_entityKeyValuesGroup->setLayout(entityKeyValuesLayout);
  }

  connect(
    m_enableByKeyValue, &QCheckBox::toggled, this, [this, kvGroup](const auto checked) {
      kvGroup->setEnabled(checked);

      toggleEmphasized(m_enableByKeyValue, checked);
      reloadSelectButtonState();
    });
}

void AdvancedEntitySelectDialog::initByClassSection()
{
  m_selectedClass = new QComboBox{};
  m_selectedClass->setEnabled(false);

  {
    m_entityClassGroup = new QGroupBox(tr("Entity class"));

    auto* entityClassLayout = new QVBoxLayout{};
    m_enableByClass = new QCheckBox{tr("Enabled")};

    entityClassLayout->addWidget(m_enableByClass);
    entityClassLayout->addSpacing(LayoutConstants::WideVMargin);
    entityClassLayout->addWidget(m_selectedClass);

    m_entityClassGroup->setLayout(entityClassLayout);
  }

  connect(m_enableByClass, &QCheckBox::toggled, this, [this](const auto checked) {
    m_selectedClass->setEnabled(checked);

    toggleEmphasized(m_enableByClass, checked);
    repopulateEntityClassList();
  });
}

void AdvancedEntitySelectDialog::initActionButtons()
{
  m_selectButton = new QPushButton(tr("Select"));
  m_cancelButton = new QPushButton(tr("Cancel"));

  m_buttonBox = new QDialogButtonBox{};

  m_buttonBox->addButton(m_selectButton, QDialogButtonBox::ButtonRole::AcceptRole);
  m_buttonBox->addButton(m_cancelButton, QDialogButtonBox::ButtonRole::RejectRole);

  connect(
    m_buttonBox,
    &QDialogButtonBox::accepted,
    this,
    &AdvancedEntitySelectDialog::onAccept);
  connect(m_buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

void AdvancedEntitySelectDialog::initDialog()
{
  setWindowTitle(tr("Select Advanced"));
  setWindowIconTB(this);

  auto* dialogHeader = new DialogHeader{tr("Search filters")};

  auto* innerLayout = new QVBoxLayout{};
  innerLayout->setContentsMargins(
    LayoutConstants::DialogOuterMargin,
    LayoutConstants::DialogOuterMargin,
    LayoutConstants::DialogOuterMargin,
    LayoutConstants::DialogOuterMargin);

  innerLayout->setSpacing(LayoutConstants::NarrowVMargin);
  innerLayout->addWidget(m_entityTypesGroup);

  innerLayout->addSpacing(LayoutConstants::WideHMargin);
  innerLayout->addWidget(m_entityKeyValuesGroup);

  innerLayout->addSpacing(LayoutConstants::WideHMargin);
  innerLayout->addWidget(m_entityClassGroup);

  const auto metrics = QFontMetrics(dialogHeader->font());
  dialogHeader->setMaximumHeight(metrics.lineSpacing() * 2);

  auto* outerLayout = new QVBoxLayout{};
  outerLayout->setContentsMargins(0, 0, 0, 0);
  outerLayout->setSpacing(0);
  outerLayout->addWidget(dialogHeader);
  outerLayout->addWidget(new BorderLine{});
  outerLayout->addLayout(innerLayout);
  outerLayout->addLayout(wrapDialogButtonBox(m_buttonBox));

  setLayout(outerLayout);

  constexpr auto desiredSize = QSize{700, 600};

  setMinimumSize(desiredSize);
  setMaximumSize(desiredSize);
}

void AdvancedEntitySelectDialog::onAccept()
{
  auto document = kdl::mem_lock(m_document);

  // TODO: there has to be a better way to get all entities...
  document->selectAllNodes();
  const auto entityNodes = document->allSelectedEntityNodes();
  document->deselectAll();

  std::function<bool(tb::mdl::EntityNodeBase*)> predicate;

  if (m_brushEntities->isChecked() and m_pointEntities->isChecked())
  {
    predicate = [](auto) { return true; };
  }
  else if (m_brushEntities->isChecked())
  {
    predicate = [](auto entityNode) { return not entityNode->entity().pointEntity(); };
  }
  else
  {
    predicate = [](auto entityNode) { return entityNode->entity().pointEntity(); };
  }

  if (m_enableByKeyValue->isChecked())
  {
    predicate = [this,
                 oldPredicate = predicate,
                 key = QRegularExpression{m_key->text()},
                 value = QRegularExpression{m_value->text()}](auto entityNode) {
      const auto previousConditionsResult = oldPredicate(entityNode);

      if (not previousConditionsResult)
      {
        return false;
      }

      const auto entityProps = rowsForEntityNode(entityNode, true, true);

      return std::find_if(
               std::begin(entityProps),
               std::end(entityProps),
               [&key, &value](const auto& property) {
                 return key.match(QString::fromStdString(property.key())).hasMatch()
                        and value.match(QString::fromStdString(property.value()))
                              .hasMatch();
               })
             != std::end(entityProps);
    };
  }

  if (m_enableByClass->isChecked())
  {
    predicate = [this, oldPredicate = predicate](auto entityNode) {
      return oldPredicate(entityNode)
             and entityNode->entity().classname() == m_selectedClass->currentText();
    };
  }

  auto entitiesToSelect = std::vector<tb::mdl::Node*>{};
  entitiesToSelect.reserve(entityNodes.size());

  std::copy_if(
    std::begin(entityNodes),
    std::end(entityNodes),
    std::back_inserter(entitiesToSelect),
    predicate);

  document->selectNodes(entitiesToSelect);

  QDialog::accept();
}

void AdvancedEntitySelectDialog::reloadSelectButtonState()
{
  if (not m_brushEntities->isChecked() and not m_pointEntities->isChecked())
  {
    m_selectButton->setEnabled(false);
    return;
  }

  if (
    m_enableByKeyValue->isChecked()
    and (m_key->text().isEmpty() or m_value->text().isEmpty()))
  {
    m_selectButton->setEnabled(false);
    return;
  }

  m_selectButton->setEnabled(true);
}
} // namespace tb::ui

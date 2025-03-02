#pragma once

#include <QDialog>

class QWidget;
class QCheckBox;
class QComboBox;
class QPushButton;
class QGroupBox;
class QDialogButtonBox;
class QLineEdit;

namespace tb::ui
{
class MapDocument;

class AdvancedEntitySelectDialog : public QDialog
{
  Q_OBJECT
private:
  std::weak_ptr<MapDocument> m_document;

  QCheckBox* m_brushEntities;
  QCheckBox* m_pointEntities;
  QGroupBox* m_entityTypesGroup;

  QCheckBox* m_enableByKeyValue;
  QGroupBox* m_entityKeyValuesGroup;
  QLineEdit* m_key;
  QLineEdit* m_value;

  QCheckBox* m_enableByClass;
  QComboBox* m_selectedClass;
  QGroupBox* m_entityClassGroup;

  QPushButton* m_selectButton;
  QPushButton* m_cancelButton;
  QDialogButtonBox* m_buttonBox;

public:
  AdvancedEntitySelectDialog(QWidget* parent, std::weak_ptr<MapDocument> document);

private:
  void repopulateEntityClassList();
  void reloadSelectButtonState();

  void initEntityTypesSection();
  void initKeyValueSection();
  void initByClassSection();
  void initActionButtons();
  void initDialog();

  void onAccept();
};
} // namespace tb::ui
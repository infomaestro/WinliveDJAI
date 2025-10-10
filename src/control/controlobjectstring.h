#pragma once
#include <QMap>
#include <QObject>
#include <QString>

#include "control/control.h"

class ControlObjectString : public QObject {
    Q_OBJECT
  public:
    ControlObjectString(const ConfigKey& key, QObject* pParent = nullptr);
    void set(const QString& value);
    QString get() const;

  signals:
    void valueChanged(QString value);

  private:
    ConfigKey m_key;

    // Registry globale per condividere i valori
    static QMap<ConfigKey, QString> s_values;
    static QList<ControlObjectString*> s_instances;
};
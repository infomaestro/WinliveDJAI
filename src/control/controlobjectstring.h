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
   
    
    void setBackColors(const QString& colorSpec);
    QString getColor() const;
    QString get() const;

  signals:
    void valueChanged(QString value);
    void colorChanged(QString color); // Nuovo signal

  private:
    ConfigKey m_key;
    QString m_color; // Colore specifico per questa istanza
    // Registry globale per condividere i valori
    static QMap<ConfigKey, QString> s_values;
    static QMap<ConfigKey, QString> s_colors; // Mappa statica per i colori
    static QList<ControlObjectString*> s_instances;
};
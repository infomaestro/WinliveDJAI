#include "control/controlobjectstring.h"
#include "moc_controlobjectstring.cpp"

QMap<ConfigKey, QString> ControlObjectString::s_values;
QMap<ConfigKey, QString> ControlObjectString::s_colors;
QList<ControlObjectString*> ControlObjectString::s_instances;

ControlObjectString::ControlObjectString(const ConfigKey& key, QObject* pParent)
        : QObject(pParent), m_key(key) {
    s_instances.append(this);
}

void ControlObjectString::set(const QString& value) {
    s_values[m_key] = value;

    // Notifica TUTTE le istanze con la stessa ConfigKey
    for (auto* instance : s_instances) {
        if (instance->m_key == m_key) {
            emit instance->valueChanged(value);
        }
    }
}

QString ControlObjectString::get() const {
    return s_values.value(m_key);
}

void ControlObjectString::setBackColors(const QString& colorSpec) {
    s_colors[m_key] = colorSpec;

    for (auto* instance : s_instances) {
        if (instance->m_key == m_key) {
            emit instance->colorChanged(colorSpec);
        }
    }
}

QString ControlObjectString::getColor() const {
    return s_colors.value(m_key);
}
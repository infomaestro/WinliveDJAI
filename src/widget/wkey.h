#pragma once

#include "widget/wlabel.h"
#include "control/controlproxy.h"

class WKey : public WLabel  {
    Q_OBJECT
  public:
    explicit WKey(const QString& group, QWidget* pParent = nullptr);

    void onConnectedControlChanged(double dParameter, double dValue) override;
    void setup(const QDomNode& node, const SkinContext& context) override;

  private slots:
    void setValue(double dValue);
    void keyNotationChanged(double dValue);
    void setCents();

  private:
    mixxx::track::io::key::ChromaticKey m_originalKey; // <-- AGGIUNGI QUESTO
    double m_dOldValue;
    bool m_displayCents;
    bool m_displayKey;
    bool m_displayValue;
    ControlProxy m_keyNotation;
    ControlProxy m_engineKeyDistance;
    ControlProxy m_pPitch; // <-- AGGIUNTO (non serve unique_ptr per ControlProxy)
};

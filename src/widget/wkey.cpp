#include "widget/wkey.h"

#include "library/library_prefs.h"
#include "moc_wkey.cpp"
#include "skin/legacy/skincontext.h"
#include "track/keyutils.h"
#include "engine/controls/keycontrol.h"

WKey::WKey(const QString& group, QWidget* pParent)
        : WLabel(pParent),
          m_originalKey(mixxx::track::io::key::INVALID), // <-- init chiave originale 
          m_dOldValue(0),
          m_keyNotation(mixxx::library::prefs::kKeyNotationConfigKey, this),
          m_engineKeyDistance(group,
                  "visual_key_distance",
                  this,
                  ControlFlag::AllowMissingOrInvalid),
          m_pPitch(group, "pitch", this) {
    setValue(m_dOldValue);
    m_keyNotation.connectValueChanged(this, &WKey::keyNotationChanged);
    m_engineKeyDistance.connectValueChanged(this, &WKey::setCents);

}

void WKey::onConnectedControlChanged(double dParameter, double dValue) {
    Q_UNUSED(dParameter);
    // Enums are not currently represented using parameter space so it doesn't
    // make sense to use the parameter here yet.
    setValue(dValue);
}

void WKey::setup(const QDomNode& node, const SkinContext& context) {
    WLabel::setup(node, context);
    m_displayCents = context.selectBool(node, "DisplayCents", false);
    m_displayKey = context.selectBool(node, "DisplayKey", true);
    m_displayValue = context.selectBool(node, "DisplayValue", false);
}

void WKey::setValue(double dValue) {
    mixxx::track::io::key::ChromaticKey key =
            KeyUtils::keyFromNumericValue(dValue);
    
    // Salva la prima chiave come originale
    if (m_originalKey == mixxx::track::io::key::INVALID &&
            key != mixxx::track::io::key::INVALID) {
        m_originalKey = key; // Salva la prima chiave valida come originale
    }

    // Rileva se la traccia è cambiata (reset)
    if (m_dOldValue == 0.0 && dValue != 0.0) {
        m_originalKey = key; // Nuova traccia caricata, aggiorna l'originale
    }

    m_dOldValue = dValue; 
    if (key != mixxx::track::io::key::INVALID) {
        // Render this key with the user-provided notation.
        QString keyStr = "";
        if (m_displayKey) {
            keyStr = KeyUtils::keyToString(key);
        }
        if (m_displayCents) {
            double diff_cents = m_engineKeyDistance.get();
            int cents_to_display = static_cast<int>(diff_cents * 100);
            char sign = ' ';
            if (diff_cents < 0) {
                sign = '-';
            } else if (diff_cents > 0) {
                sign = '+';
            }
            keyStr.append(QString(" %1%2c").arg(sign).arg(qAbs(cents_to_display)));
        }
        if (m_displayValue) {  // visualizza il cambio tonalità
            double value_to_display = m_pPitch.get(); // Leggi il valore
            keyStr.prepend(QString("%1 ").arg(qRound(value_to_display)));

        }


        setText(keyStr);
    } else {
        setText("");
    }
}

void WKey::setCents() {
    setValue(m_dOldValue);
}

void WKey::keyNotationChanged(double dKeyNotationValue) {
    Q_UNUSED(dKeyNotationValue);
    // NOTE: dKeyNotationValue is the index of the key notation type, NOT the
    // key itself, so we intentionally set the old value again to update the UI.
    setValue(m_dOldValue);
}


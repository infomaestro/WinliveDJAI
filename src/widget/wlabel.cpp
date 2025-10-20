#include "widget/wlabel.h"

#include <QEvent>
#include <QFont>

#include "moc_wlabel.cpp"
#include "skin/legacy/skincontext.h"
#include "widget/wskincolor.h"

WLabel::WLabel(QWidget* pParent)
        : QLabel(pParent),
          WBaseWidget(this),
          m_skinText(),
          m_longText(),
          m_elideMode(Qt::ElideNone),
          m_scaleFactor(1.0),
          m_highlight(0),
          m_widthHint(0),
          m_pControlObjectString(nullptr),
          m_pBlinkTimer(nullptr), // AGGIUNGI QUESTO
          m_blinkState(false) {
}

void WLabel::setup(const QDomNode& node, const SkinContext& context) {
    m_scaleFactor = context.getScaleFactor();

    // Colors
    QPalette pal = palette(); // we have to copy out the palette to edit it since it's const (probably for threadsafety)

    // Connection per ConfigKey dinamiche (consente modifiche runtime se c'è la proprietà RuntimeEditable = true)
    
    bool isEditable = context.selectBool(node, "RuntimeEditable", false);

    if (isEditable) {
        QDomElement connection = context.selectElement(node, "Connection");
        if (!connection.isNull()) {
            QString configKey = context.selectString(connection, "ConfigKey");
            if (!configKey.isEmpty()) {
                ConfigKey key = ConfigKey::parseCommaSeparated(configKey);
                // if (key.item.contains("karaoke_info")) { //non ho più bisogno di identificare karaoke_info
                m_pControlObjectString = new ControlObjectString(key, this);
                
                connect(m_pControlObjectString, &ControlObjectString::valueChanged, this, &WLabel::slotStringValueChanged);
                slotStringValueChanged(m_pControlObjectString->get());
                
                connect(m_pControlObjectString, &ControlObjectString::colorChanged, this, &WLabel::slotStringcolorChanged);
                slotStringcolorChanged(m_pControlObjectString->getColor());
                //}
            }
        }
    }


    QDomElement bgColor = context.selectElement(node, "BgColor");
    if (!bgColor.isNull()) {
        m_qBgColor = QColor(context.nodeToString(bgColor));
        pal.setColor(this->backgroundRole(), WSkinColor::getCorrectColor(m_qBgColor));
        setAutoFillBackground(true);
    }

    m_qFgColor = QColor(context.selectString(node, "FgColor"));
    pal.setColor(this->foregroundRole(), WSkinColor::getCorrectColor(m_qFgColor));
    setPalette(pal);

    // Font size
    QString strFontSize;
    if (context.hasNodeSelectString(node, "FontSize", &strFontSize)) {
        bool widthOk = false;
        double dFontSize = strFontSize.toDouble(&widthOk);
        if (widthOk && dFontSize >= 0) {
            QFont fonti = font();
            // We do not scale the font here, because in most cases
            // this is overridden by the style sheet font size
            fonti.setPointSizeF(dFontSize);
            setFont(fonti);
        }
    }

    // Text
    if (context.hasNodeSelectString(node, "Text", &m_skinText)) {
        setText(m_skinText);
    }

    // Alignment
    QString alignment;
    if (context.hasNodeSelectString(node, "Alignment", &alignment)) {
        alignment = alignment.toLower();
        if (alignment == "right") {
            setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        } else if (alignment == "center") {
            setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
        } else if (alignment == "left") {
            setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        } else {
            qDebug() << "WLabel::setup(): Alignment =" << alignment <<
                    " unknown, use right, center or left";
        }
    }

    // Adds an ellipsis to truncated text
    QString elide;
    if (context.hasNodeSelectString(node, "Elide", &elide)) {
        elide = elide.toLower();
        if (elide == "right") {
            m_elideMode = Qt::ElideRight;
        } else if (elide == "middle") {
            m_elideMode = Qt::ElideMiddle;
        } else if (elide == "left") {
            m_elideMode = Qt::ElideLeft;
        } else if (elide == "none") {
            m_elideMode = Qt::ElideNone;
        } else {
            qDebug() << "WLabel::setup(): Elide =" << elide <<
                    "unknown, use right, middle, left or none.";
        }
    }

    


}

QString WLabel::text() const {
    return m_longText;
}

void WLabel::setText(const QString& text) {
    m_longText = text;
    if (m_elideMode != Qt::ElideNone) {
        QFontMetrics metrics(font());
        // Measure the text for the optimum label width
        // frameWidth() is the maximum of the sum of margin, border and padding
        // width of the left and the right side.
        m_widthHint = metrics.size(0, m_longText).width() + 2 * frameWidth();
        QString elidedText = metrics.elidedText(
                m_longText, m_elideMode, width() - 2 * frameWidth());
        QLabel::setText(elidedText);
    } else {
        QLabel::setText(m_longText);
    }
}

bool WLabel::event(QEvent* pEvent) {
    if (pEvent->type() == QEvent::ToolTip) {
        updateTooltip();
    } else if (pEvent->type() == QEvent::FontChange) {
        const QFont& fonti = font();
        // Change the new font on the fly by casting away its constancy
        // using setFont() here, would results into a recursive loop
        // resetting the font to the original css values.
        // Only scale pixel size fonts, point size fonts are scaled by the OS
        if (fonti.pixelSize() > 0) {
            const_cast<QFont&>(fonti).setPixelSize(
                    static_cast<int>(fonti.pixelSize() * m_scaleFactor));
        }
        // measure text with the new font
        setText(m_longText);
    }
    return QLabel::event(pEvent);
}

void WLabel::resizeEvent(QResizeEvent* event) {
    QLabel::resizeEvent(event);
    setText(m_longText);
}

void WLabel::fillDebugTooltip(QStringList* debug) {
    WBaseWidget::fillDebugTooltip(debug);
    *debug << QString("Text: \"%1\"").arg(text());
}

int WLabel::getHighlight() const {
    return m_highlight;
}

void WLabel::setHighlight(int highlight) {
    if (m_highlight == highlight) {
        return;
    }
    m_highlight = highlight;
    emit highlightChanged(m_highlight);
}

void WLabel::slotStringValueChanged(QString value) {
    setText(value);
}

void WLabel::slotStringcolorChanged(QString value) {
    if (m_pBlinkTimer) {
        m_pBlinkTimer->stop();
    }

    // Controlla se è un formato blink: "#FF0000|#00FF00|blink:500"
    QStringList parts = value.split('|');

    if (parts.size() >= 3 && parts[2].startsWith("blink:")) {
        // Effetto blink
        QString color1 = parts[0];
        QString color2 = parts[1];
        int interval = parts[2].mid(6).toInt(); // Rimuove "blink:"

        startBlink(color1, color2, interval);
    } else {
        // Colore singolo
        QColor color(value);
        if (!color.isValid()) {
            qWarning() << "WLabel: Colore non valido:" << value;
            return;
        }
        setStyleSheet(QString("QLabel { background-color: %1; }").arg(color.name()));
    }
   
}
void WLabel::startBlink(const QString& color1, const QString& color2, int intervalMs) {
    m_blinkColor1 = color1;
    m_blinkColor2 = color2;
    m_blinkState = false;

    if (!m_pBlinkTimer) {
        m_pBlinkTimer = new QTimer(this);
        connect(m_pBlinkTimer, &QTimer::timeout, this, &WLabel::slotBlink);
    }

    m_pBlinkTimer->start(intervalMs);
}

void WLabel::stopBlink() {
    if (m_pBlinkTimer) {
        m_pBlinkTimer->stop();
    }
}

void WLabel::slotBlink() {
    m_blinkState = !m_blinkState;
    QString color = m_blinkState ? m_blinkColor1 : m_blinkColor2;
    setStyleSheet(QString("QLabel { background-color: %1; }").arg(color));
}

QSize WLabel::sizeHint() const {
    // make sure the sizeHint fits for the entire string.
    QSize size = QLabel::sizeHint();
    if (m_elideMode != Qt::ElideNone) {
        size.setWidth(m_widthHint);
    }
    return size;
}

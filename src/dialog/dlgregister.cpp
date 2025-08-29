#include "dialog/dlgregister.h"
#include "moc_dlgregister.cpp"
#include <QApplication>
#include <QScreen>
#include <QLineEdit>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QUrlQuery>
#include <QUrl>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMessageBox>
#include "util/serial.h"

DlgRegister::DlgRegister(QWidget* parent)
        : QDialog(parent),
          Ui::DlgRegisterDlg() {

    setupUi(this);
    setFixedSize(600, 420);
    buttonClose->setVisible(false);
    textEdit->setReadOnly (true);
    // center screen
    if (QScreen* screen = QApplication::primaryScreen()) {
        QRect screenGeometry = screen->geometry();
        int x = (screenGeometry.width() - width()) / 2;
        int y = (screenGeometry.height() - height()) / 2;
        move(x, y);
    }

    // get the volume
    m_idLicense = VolumeSerialHelper::getVolumeSerial();
    if (m_idLicense.isEmpty() == true) {
        QMessageBox::warning(this, tr("Error"), tr("Unable to get License Id"));
    }


    // check if license already exists
    CheckLicenseHelper existingLicense;
    if (existingLicense.loadFromFile()) {

        qDebug() << "Existing license found!";
        qDebug() << "Status:" << existingLicense.getLicenseStatus();

        editPersonalAccount->setText(existingLicense.personalAccount());
        editSerialNumber->setText(existingLicense.serialNumber());

        if (existingLicense.hasValidLicense(m_idLicense) == true) {

            // show fields with existing data
            editPersonalAccount->setReadOnly(true);
            editSerialNumber->setReadOnly(true);
            buttonRegister->setEnabled(false);
            label_4->setVisible(false);
            textEdit->setHtml(tr("<b>Thank you for registering the software!</b><br><br>You can now have access to all extra features without limitations."));
            // show current license info
            QString info = QString(tr("License activated on %1")).arg(existingLicense.activationDate().toString());
            updateStatus(info);

            buttonRegister->setVisible(false);
            buttonCancel->setVisible(false);
            buttonClose->setVisible(true);


        } else {
            // show fields with existing data
            editPersonalAccount->setReadOnly(false);
            editSerialNumber->setReadOnly(false);

            // show current license info
            QString info = QString(tr("License not activated for this Id (%1)")).arg(m_idLicense);
            updateStatus(info);
        }
    } else {
        // no license found
        updateStatus(tr("No License activated"));
    }

    // network manager
    m_networkManager = new QNetworkAccessManager(this);
    connect(m_networkManager, &QNetworkAccessManager::finished, this, &DlgRegister::onNetworkReplyFinished);

    auto registrationHandler = [this]() {

        QString personalAccount = editPersonalAccount->text().trimmed().toUpper();
        QString serialNumber = editSerialNumber->text().trimmed().toUpper();

        editPersonalAccount->setText(personalAccount);
        editSerialNumber->setText(serialNumber);

        // check
        if (personalAccount.isEmpty()) {
            QMessageBox::warning(this, tr("Error"), tr("Please enter the Personal Account"));
            editPersonalAccount->setFocus();
            return;
        }
        if (serialNumber.isEmpty()) {
            QMessageBox::warning(this, tr("Error"), tr("Please enter the Serial Number"));
            editSerialNumber->setFocus();
            return;
        }


        // ask confirm
        if (QMessageBox::No == QMessageBox::question(this, tr("Confirmation"), tr("Confirm registration?"), QMessageBox::Yes | QMessageBox::No)) {
            return;
        }
        buttonRegister->setEnabled(false);
        buttonRegister->setText(tr("Registration in progress..."));

        // call server
        QUrl url("https://www.promusicsoftware.com/wl099.php"); 

        QUrlQuery query;
        query.addQueryItem("wl001prac", personalAccount);
        query.addQueryItem("wl001wlsn", serialNumber);
        query.addQueryItem("id", m_idLicense);
        url.setQuery(query);

        QNetworkRequest request(url);
        request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");

        // get
        m_networkManager->get(request);
    };

    connect(buttonRegister, &QPushButton::clicked, registrationHandler);
    connect(buttonCancel, &QPushButton::clicked, [this]() {
        this->reject(); 
    });
    connect(buttonClose, &QPushButton::clicked, [this]() {
        this->reject(); 
    });

}


DlgRegister::~DlgRegister() {
}

void DlgRegister::onNetworkReplyFinished(QNetworkReply* reply) {

    // re-enable button
    buttonRegister->setEnabled(true);
    buttonRegister->setText(tr("Register"));

    m_keyLicense = "";

    if (reply->error() == QNetworkReply::NoError) {
        
        // server response
        QByteArray responseData = reply->readAll();
        QString responseString = QString::fromUtf8(responseData).trimmed();

        qDebug() << "Server response:" << responseString;

        // check server response
        if (responseString.length() > 3 && responseString.left(2) == "OK") {
            // save licence
            // get the key after "OK_"
            m_keyLicense = responseString.mid(3).trimmed(); 
            if (this->saveLicense() == false) {
                QMessageBox::warning(this, tr("Error"), tr("Failed to save license file!"));
            }
            else {
                QMessageBox::information(this, tr("Success"), tr("Registration completed successfully!"));
                buttonRegister->setVisible(false);
                buttonCancel->setVisible(false);
                buttonClose->setVisible(true);
                this->close();
            }

        } else {
            QMessageBox::warning(this, tr("Error"), tr("Registration failed. Please try again."));
            // Disabilita tutta la finestra
            this->setEnabled(false);
            // Timer per riabilitare
            QTimer::singleShot(5000, this, [=]() {
                this->setEnabled(true);
            });
        }
    } else {
        // network error
        QString errorString = reply->errorString();
        qDebug() << "Network error:" << errorString;
        QMessageBox::critical(this, tr("Connection Error"), tr("Unable to contact server:") + "\n" + errorString);
    }

    reply->deleteLater();
}

void DlgRegister::keyPressEvent(QKeyEvent* event) {
    if (event->key() != Qt::Key_Escape) {
        QDialog::keyPressEvent(event);
    }
}

void DlgRegister::close() {
    reject();
}

bool DlgRegister::saveLicense() {
    CheckLicenseHelper license;
    
    // set license data
    license.setPersonalAccount(editPersonalAccount->text());
    license.setSerialNumber(editSerialNumber->text());
    license.setIdLicense(m_idLicense);
    license.setKeyLicense(m_keyLicense);
    license.setActivationDate(QDateTime::currentDateTime());

    // save to encrypted file
    if (license.saveToFile()) {
        qDebug() << "License created and saved!";
        qDebug() << "Personal Account:" << license.personalAccount();
        qDebug() << "Serial Number:" << license.serialNumber();
        qDebug() << "License ID:" << license.idLicense();
        qDebug() << "License KEY:" << license.keyLicense();
        qDebug() << "Activation Date:" << license.activationDate().toString();
        qDebug() << "File location:" << license.getDefaultFilePath();

        // show current license info
        QString info = QString(tr("License activated on %1")).arg(license.activationDate().toString());
        updateStatus(info);

        return true;
    } else {
        qWarning() << "Failed to save license file!";
        return false;
    }
}

void DlgRegister::updateStatus(const QString& status) {
    
    QString info = QString(tr("Status: <b>%1</b>")).arg(status);
    labelStatus->setText(info);
}




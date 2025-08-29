#ifndef VOLUMESERIALHELPER_H
#define VOLUMESERIALHELPER_H

#include <QString>
#include <QDateTime>
#include <QStandardPaths>

class VolumeSerialHelper {
  public:
    static QString getVolumeSerial();
    static QString getApplicationDrivePath();

  private:
    static QString getWindowsVolumeSerial(const QString& drivePath);
    static QString getMacVolumeSerial(const QString& drivePath);
};
class CheckLicenseHelper {

  public:
    CheckLicenseHelper();

    // getters
    QString personalAccount() const {
        return m_personalAccount;
    }
    QString serialNumber() const {
        return m_serialNumber;
    }
    QString idLicense() const {
        return m_idLicense;
    }
    QString keyLicense() const {
        return m_keyLicense;
    }
    QDateTime activationDate() const {
        return m_activationDate;
    }

    // setters
    void setPersonalAccount(const QString& account);
    void setSerialNumber(const QString& serial);
    void setIdLicense(const QString& id);
    void setKeyLicense(const QString& key);
    void setActivationDate(const QDateTime& date);

    // operations
    bool saveToFile(const QString& filePath = QString());
    bool loadFromFile(const QString& filePath = QString());

    // license validation
    bool isLicenseValid() const;
    bool isLicenseExpired(int validDays = 90) const;
    QString getLicenseStatus() const;

    // utility functions
    void clear();
    QString getDefaultFilePath() const;
    bool hasValidLicense(const QString& id = "") const;

    // helpers
    static CheckLicenseHelper fromFile(const QString& filePath = QString());

  private:
    // member variables
    QString m_personalAccount;
    QString m_serialNumber;
    QString m_idLicense;
    QString m_keyLicense;
    QDateTime m_activationDate;

    // encryption/Decryption (simple XOR cipher)
    QString encryptData(const QString& data) const;
    QString decryptData(const QString& encryptedData) const;

    // internal helpers
    QString dataToString() const;
    bool parseFromString(const QString& data);

    // encryption key (in real app, this should be more sophisticated)
    static const char* ENCRYPTION_KEY;
};


#endif // VOLUMESERIALHELPER_H
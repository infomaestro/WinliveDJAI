#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QProcess>
#include <QStandardPaths>
#include <QCryptographicHash>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QRandomGenerator>
#include <QTextStream>

#include "serial.h"

#ifdef Q_OS_WIN
#include <windows.h>

#include <QLibrary>
#endif

#ifdef Q_OS_MAC
#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/IOBSD.h>
#include <IOKit/IOKitLib.h>
#include <IOKit/storage/IOBlockStorageDriver.h>
#include <IOKit/storage/IOMedia.h>
#include <sys/mount.h>
#include <sys/param.h>
#include <sys/ucred.h>
#endif

QString VolumeSerialHelper::getVolumeSerial() {
    QString drivePath = getApplicationDrivePath();
    QString serial;

#ifdef Q_OS_WIN
    serial = getWindowsVolumeSerial(drivePath);
#elif defined(Q_OS_MAC)
    serial = getMacVolumeSerial(drivePath);
#else
    qWarning() << "Volume serial detection not supported on this platform";
    serial = "UNSUPPORTED_PLATFORM";
#endif

    qDebug() << "Application drive path:" << drivePath;
    qDebug() << "Volume serial:" << serial;

    return serial;
}

QString VolumeSerialHelper::getApplicationDrivePath() {
    // Get the path where the application is located
    QString appPath = QCoreApplication::applicationDirPath();

#ifdef Q_OS_WIN
    // Extract drive letter (e.g., "C:" from "C:/Program Files/MyApp")
    return appPath.left(2);
#elif defined(Q_OS_MAC)
    // For macOS, find the mount point
    QProcess process;
    process.start("df", QStringList() << appPath);
    process.waitForFinished();

    QString output = process.readAllStandardOutput();
    QStringList lines = output.split('\n');

    if (lines.size() > 1) {
        QStringList parts = lines[1].split(QRegExp("\\s+"));
        if (parts.size() > 5) {
            return parts.last(); // Mount point
        }
    }

    return "/"; // Fallback to root
#else
    return appPath;
#endif
}

QString VolumeSerialHelper::getWindowsVolumeSerial(const QString& drivePath) {
#ifdef Q_OS_WIN
    // Method 1: Using Windows API GetVolumeInformation
    wchar_t volumeName[MAX_PATH + 1];
    wchar_t fileSystemName[MAX_PATH + 1];
    DWORD serialNumber = 0;
    DWORD maxComponentLength = 0;
    DWORD fileSystemFlags = 0;

    QString rootPath = drivePath + "\\";

    BOOL result = GetVolumeInformationW(
            reinterpret_cast<const wchar_t*>(rootPath.utf16()),
            volumeName,
            MAX_PATH,
            &serialNumber,
            &maxComponentLength,
            &fileSystemFlags,
            fileSystemName,
            MAX_PATH);

    if (result) {
        return QString::number(serialNumber, 16).toUpper();
    }

    // Method 2: Fallback using WMI via QProcess
    QProcess process;
    QString wmiQuery = QString("wmic logicaldisk where caption=\"%1\" get volumeserialnumber /format:value").arg(drivePath);

    process.start("cmd.exe", QStringList() << "/c" << wmiQuery);
    process.waitForFinished(5000);

    QString output = process.readAllStandardOutput();
    QStringList lines = output.split('\n');

    for (const QString& line : lines) {
        if (line.contains("VolumeSerialNumber=")) {
            QString serial = line.split('=').last().trimmed();
            if (!serial.isEmpty()) {
                return serial.toUpper();
            }
        }
    }

    // Method 3: Another fallback using PowerShell
    process.start("powershell.exe", QStringList() << "-Command" << QString("(Get-WmiObject -Class Win32_LogicalDisk -Filter \"DeviceID='%1'\").VolumeSerialNumber").arg(drivePath));
    process.waitForFinished(5000);

    output = process.readAllStandardOutput().trimmed();
    if (!output.isEmpty()) {
        return output.toUpper();
    }

#endif
    return "UNKNOWN_WINDOWS";
}

QString VolumeSerialHelper::getMacVolumeSerial(const QString& drivePath) {
#ifdef Q_OS_MAC
    // Method 1: Using system_profiler
    QProcess process;
    process.start("system_profiler", QStringList() << "SPStorageDataType" << "-xml");
    process.waitForFinished(10000);

    QString output = process.readAllStandardOutput();

    // Parse XML to find volume UUID (closest to Windows volume serial)
    // This is a simplified approach - you might want to use QXmlStreamReader for robust parsing
    if (output.contains("volume_uuid")) {
        QRegExp rx("<string>([A-F0-9\\-]{36})</string>");
        if (rx.indexIn(output) != -1) {
            QString uuid = rx.cap(1);
            // Remove hyphens and take first 8 characters to mimic Windows format
            return uuid.remove('-').left(8).toUpper();
        }
    }

    // Method 2: Using diskutil
    process.start("diskutil", QStringList() << "info" << drivePath);
    process.waitForFinished(5000);

    output = process.readAllStandardOutput();
    QStringList lines = output.split('\n');

    for (const QString& line : lines) {
        if (line.contains("Volume UUID:") || line.contains("Disk / Partition UUID:")) {
            QString uuid = line.split(':').last().trimmed();
            if (!uuid.isEmpty()) {
                return uuid.remove('-').left(8).toUpper();
            }
        }
    }

    // Method 3: Using IOKit (more complex but more reliable)
    // This would require additional IOKit framework linking
    // For now, using a simpler approach

    // Method 4: Get device serial using ioreg
    process.start("ioreg", QStringList() << "-c" << "IOMedia" << "-r");
    process.waitForFinished(5000);

    output = process.readAllStandardOutput();

    // Look for serial number in the output
    QRegExp serialRx("\"Serial Number\" = \"([^\"]+)\"");
    if (serialRx.indexIn(output) != -1) {
        return serialRx.cap(1).toUpper();
    }

#endif
    return "UNKNOWN_MAC";
}


// simple encryption key
const char* CheckLicenseHelper::ENCRYPTION_KEY = "WDJLicense2025Key!";

CheckLicenseHelper::CheckLicenseHelper()
        : m_activationDate(QDateTime::currentDateTime()) {
}

void CheckLicenseHelper::setPersonalAccount(const QString& account) {
    m_personalAccount = account.trimmed();
}

void CheckLicenseHelper::setSerialNumber(const QString& serial) {
    m_serialNumber = serial.trimmed();
}

void CheckLicenseHelper::setIdLicense(const QString& id) {
    m_idLicense = id.trimmed();
}

void CheckLicenseHelper::setKeyLicense(const QString& key) {
    m_keyLicense = key.trimmed();
}

void CheckLicenseHelper::setActivationDate(const QDateTime& date) {
    m_activationDate = date;
}

QString CheckLicenseHelper::getDefaultFilePath() const {

    // save in application data directory
    QString appConfigPath = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    QDir dir(appConfigPath);
    if (!dir.exists()) {
        dir.mkpath(appConfigPath);
    }
    return dir.filePath("license.dat");
}

bool CheckLicenseHelper::saveToFile(const QString& filePath) {
    QString path = filePath.isEmpty() ? getDefaultFilePath() : filePath;

    // convert data to JSON string
    QString jsonData = dataToString();

    // encrypt the data
    QString encryptedData = encryptData(jsonData);

    // save to file
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qWarning() << "Cannot open license file for writing:" << path;
        return false;
    }

    QTextStream out(&file);
    out << encryptedData;
    file.close();

    qDebug() << "License saved to:" << path;
    return true;
}

bool CheckLicenseHelper::loadFromFile(const QString& filePath) {
    QString path = filePath.isEmpty() ? getDefaultFilePath() : filePath;

    QFile file(path);
    if (!file.exists()) {
        qDebug() << "License file does not exist:" << path;
        return false;
    }

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Cannot open license file for reading:" << path;
        return false;
    }

    QTextStream in(&file);
    QString encryptedData = in.readAll();
    file.close();

    // decrypt the data
    QString jsonData = decryptData(encryptedData);

    // parse the decrypted data
    bool success = parseFromString(jsonData);

    if (success) {
        qDebug() << "License loaded successfully from:" << path;
    } else {
        qWarning() << "Failed to parse license data";
    }

    return success;
}

QString CheckLicenseHelper::dataToString() const {
    QJsonObject jsonObj;
    jsonObj["personalAccount"] = m_personalAccount;
    jsonObj["serialNumber"] = m_serialNumber;
    jsonObj["idLicense"] = m_idLicense;
    jsonObj["keyLicense"] = m_keyLicense;
    jsonObj["activationDate"] = m_activationDate.toString(Qt::ISODate);

    QJsonDocument doc(jsonObj);
    return doc.toJson(QJsonDocument::Compact);
}

bool CheckLicenseHelper::parseFromString(const QString& data) {
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(data.toUtf8(), &error);

    if (error.error != QJsonParseError::NoError) {
        qWarning() << "JSON parse error:" << error.errorString();
        return false;
    }

    QJsonObject jsonObj = doc.object();

    m_personalAccount = jsonObj["personalAccount"].toString();
    m_serialNumber = jsonObj["serialNumber"].toString();
    m_idLicense = jsonObj["idLicense"].toString();
    m_keyLicense = jsonObj["keyLicense"].toString();
    m_activationDate = QDateTime::fromString(jsonObj["activationDate"].toString(), Qt::ISODate);

    return !m_personalAccount.isEmpty() && !m_serialNumber.isEmpty() && !m_idLicense.isEmpty();
}

QString CheckLicenseHelper::encryptData(const QString& data) const {
    QByteArray key = QByteArray(ENCRYPTION_KEY);
    QByteArray input = data.toUtf8();
    QByteArray encrypted;

    // simple XOR encryption (rotate key)
    for (int i = 0; i < input.size(); ++i) {
        char keyChar = key[i % key.size()];
        encrypted.append(input[i] ^ keyChar);
    }

    // encode to Base64 for safe text storage
    return encrypted.toBase64();
}

QString CheckLicenseHelper::decryptData(const QString& encryptedData) const {
    // decode from Base64
    QByteArray encrypted = QByteArray::fromBase64(encryptedData.toUtf8());
    QByteArray key = QByteArray(ENCRYPTION_KEY);
    QByteArray decrypted;

    // simple XOR decryption (same as encryption)
    for (int i = 0; i < encrypted.size(); ++i) {
        char keyChar = key[i % key.size()];
        decrypted.append(encrypted[i] ^ keyChar);
    }

    return QString::fromUtf8(decrypted);
}

bool CheckLicenseHelper::isLicenseValid() const {
    return !m_personalAccount.isEmpty() &&
            !m_serialNumber.isEmpty() &&
            !m_idLicense.isEmpty() &&
            !m_keyLicense.isEmpty() &&
            m_activationDate.isValid();
}

bool CheckLicenseHelper::isLicenseExpired(int validDays) const {
    if (!isLicenseValid()) {
        return true;
    }

    QDateTime expiryDate = m_activationDate.addDays(validDays);
    return QDateTime::currentDateTime() > expiryDate;
}

QString CheckLicenseHelper::getLicenseStatus() const {
    if (!isLicenseValid()) {
        return "Invalid License";
    }

    if (isLicenseExpired()) {
        return "License Expired";
    }

    // calculate days remaining
    QDateTime expiryDate = m_activationDate.addDays(365);
    int daysRemaining = QDateTime::currentDateTime().daysTo(expiryDate);

    if (daysRemaining > 30) {
        return "License Valid";
    } else {
        return QString("License Expires in %1 days").arg(daysRemaining);
    }
}

bool CheckLicenseHelper::hasValidLicense(const QString& id) const {
    QString idLicense = id;
    if (id.isEmpty()) {
        idLicense = VolumeSerialHelper::getVolumeSerial();
    }
    return isLicenseValid() && m_idLicense == idLicense;
}

void CheckLicenseHelper::clear() {
    m_personalAccount.clear();
    m_serialNumber.clear();
    m_idLicense.clear();
    m_keyLicense.clear();
    m_activationDate = QDateTime();
}

CheckLicenseHelper CheckLicenseHelper::fromFile(const QString& filePath) {
    CheckLicenseHelper helper;
    helper.loadFromFile(filePath);
    return helper;
}

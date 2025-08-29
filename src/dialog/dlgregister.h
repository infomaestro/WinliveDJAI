#pragma once

#include <QCheckBox>
#include <QDialog>
#include <QHBoxLayout>
#include <QLabel>
#include <QPixmap>
#include <QPushButton>
#include <QVBoxLayout>
#include "dialog/ui_dlgregisterdlg.h"
#include <QNetworkAccessManager>
#include <QNetworkReply>


class DlgRegister : public QDialog, public Ui::DlgRegisterDlg {
    Q_OBJECT

  public:
    explicit DlgRegister(QWidget* parent = nullptr);
    ~DlgRegister();

  private slots:
    void onNetworkReplyFinished(QNetworkReply* reply);

  private:
    Ui::DlgRegisterDlg* ui;
    QNetworkAccessManager* m_networkManager;
    QString m_keyLicense;
    QString m_idLicense;

    void keyPressEvent(QKeyEvent* event);
    void close();
    bool saveLicense();
    void updateStatus(const QString& status);
};
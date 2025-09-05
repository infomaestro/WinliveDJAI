#include "skin/skincontrols.h"
#include "dialog/dlgregister.h"
#include "util/serial.h"

#include <QString>
#include <QUrlQuery>
#include <QUrl>
#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QObject>
#include <QDesktopServices>

namespace {
const QString kSkinGroup = QStringLiteral("[Skin]");
} // namespace

SkinControls::SkinControls()
        : m_showEffectRack(ConfigKey(kSkinGroup, QStringLiteral("show_effectrack")),
                  true,
                  true),
          m_showLibraryCoverArt(ConfigKey(kSkinGroup, QStringLiteral("show_library_coverart")),
                  true,
                  true),
          m_showMicrophones(ConfigKey(kSkinGroup, QStringLiteral("show_microphones")),
                  true,
                  true),
          m_showPreviewDecks(ConfigKey(kSkinGroup, QStringLiteral("show_preview_decks")),
                  true,
                  true),
          m_showSamplers(ConfigKey(kSkinGroup, QStringLiteral("show_samplers")),
                  true,
                  true),
          m_show4EffectUnits(ConfigKey(kSkinGroup, QStringLiteral("show_4effectunits")),
                  true,
                  false),
          m_showCoverArt(ConfigKey(kSkinGroup, QStringLiteral("show_coverart")),
                  true,
                  true),
          m_showMaximizedLibrary(ConfigKey(kSkinGroup, QStringLiteral("show_maximized_library")),
                  true,
                  false),
          m_showMixer(ConfigKey(kSkinGroup, QStringLiteral("show_mixer")),
                  true,
                  true),
          m_showSettings(ConfigKey(kSkinGroup, QStringLiteral("show_settings")),
                  false,
                  false),
          m_showSpinnies(ConfigKey(kSkinGroup, QStringLiteral("show_spinnies")),
                  true,
                  true),
          m_showVinylControl(ConfigKey(kSkinGroup, QStringLiteral("show_vinylcontrol")),
                  true,
                  false),          
		  m_showWinliveAi(ConfigKey(kSkinGroup, QStringLiteral("show_winliveai")),
                  false,
                  false){

    m_showEffectRack.setButtonMode(ControlPushButton::TOGGLE);
    m_showLibraryCoverArt.setButtonMode(ControlPushButton::TOGGLE);
    m_showMicrophones.setButtonMode(ControlPushButton::TOGGLE);
    m_showPreviewDecks.setButtonMode(ControlPushButton::TOGGLE);
    m_showSamplers.setButtonMode(ControlPushButton::TOGGLE);
    m_show4EffectUnits.setButtonMode(ControlPushButton::TOGGLE);
    m_showCoverArt.setButtonMode(ControlPushButton::TOGGLE);
    m_showMaximizedLibrary.setButtonMode(ControlPushButton::TOGGLE);
    m_showMixer.setButtonMode(ControlPushButton::TOGGLE);
    m_showSettings.setButtonMode(ControlPushButton::TOGGLE);
    m_showSpinnies.setButtonMode(ControlPushButton::TOGGLE);
    m_showVinylControl.setButtonMode(ControlPushButton::TOGGLE);
    m_showWinliveAi.setButtonMode(ControlPushButton::PUSH);


    m_showEffectRack.addAlias(ConfigKey(QStringLiteral("[EffectRack1]"), QStringLiteral("show")));
    m_showLibraryCoverArt.addAlias(ConfigKey(
            QStringLiteral("[Library]"), QStringLiteral("show_coverart")));
    m_showMicrophones.addAlias(ConfigKey(
            QStringLiteral("[Microphone]"), QStringLiteral("show_microphone")));
    m_showPreviewDecks.addAlias(ConfigKey(QStringLiteral("[PreviewDeck]"),
            QStringLiteral("show_previewdeck")));
    m_showSamplers.addAlias(ConfigKey(
            QStringLiteral("[Samplers]"), QStringLiteral("show_samplers")));
    m_showMaximizedLibrary.addAlias(ConfigKey(
            QStringLiteral("[Master]"), QStringLiteral("maximize_library")));

	 // Connect per intercettare il click su WinliveAi
    connect(&m_showWinliveAi,
            &ControlPushButton::valueChanged,
            this,
            [this](double value) {
                if (value > 0.0) {
                    showWinliveAI(false);
                    m_showWinliveAi.set(0.0);
                }
            });
}


 void SkinControls::showWinliveAI() {
    showWinliveAI(true);
 }

 void SkinControls::showWinliveAI(bool registering) {
     CheckLicenseHelper license;

     if (license.loadFromFile() 
         && license.hasValidLicense() == true) {
     
         if (registering == false) {
             qDebug() << "Application licensed to:" << license.personalAccount();

             QUrl url("https://www.promusicsoftware.com/mt020.php");
             QUrlQuery query;
             query.addQueryItem("wl001prac", license.personalAccount());
             query.addQueryItem("wl001wlsn", license.serialNumber());
             url.setQuery(query);

            #ifdef Q_OS_IOS
                QUrl urlToOpen = url;
                if (urlToOpen.scheme() == "file") {
                    urlToOpen.setScheme("shareddocuments");
                }
                QDesktopServices::openUrl(urlToOpen);
            #else
                QDesktopServices::openUrl(url);
            #endif


             return;
         }
         return;
     }

     QScopedPointer<DlgRegister> dialog(new DlgRegister());
     int result = dialog->exec();

     if (result == QDialog::Accepted) {
         qDebug() << "User accepted registration";
     } else {
         qDebug() << "User declined registration";
     }

}

#include "dialog/dlgabout.h"

#include <QDebug>
#include <QFile>
#include <QLocale>

#include "defs_urls.h"
#include "moc_dlgabout.cpp"
#include "util/color/color.h"
#include "util/desktophelper.h"
#include "util/versionstore.h"

DlgAbout::DlgAbout()
        : QDialog(nullptr),
          Ui::DlgAboutDlg() {
    setupUi(this);
    setWindowIcon(QIcon(MIXXX_ICON_PATH));

    mixxx_icon->load(QString(MIXXX_ICON_PATH));
    mixxx_logo->load(QString(MIXXX_LOGO_PATH));

    version_label->setText(VersionStore::applicationName() +
            QStringLiteral(" ") + VersionStore::version());
    git_version_label->setText(VersionStore::gitVersion());
    qt_version_label->setText(VersionStore::qtVersion());
    platform_label->setText(VersionStore::platform());
    QLocale locale;
    date_label->setText(locale.toString(VersionStore::date().toLocalTime(), QLocale::LongFormat));

    QFile licenseFile(":/LICENSE");
    if (!licenseFile.open(QIODevice::ReadOnly)) {
        qWarning() << "LICENSE file not found";
    } else {
        licenseText->setPlainText(licenseFile.readAll());
    }

    QString s_devTeam =
            tr("Mixxx %1.%2 Development Team")
                    .arg(QString::number(
                                 VersionStore::versionNumber().majorVersion()),
                            QString::number(VersionStore::versionNumber()
                                                    .minorVersion()));
    QString s_contributions = tr("With contributions from:");
    QString s_mixxxmentions = tr("GNU platform based:");

    QStringList thisReleaseDevelopers;
    thisReleaseDevelopers
            << "P.G.M"
            << "L:C"
            << "L.D.P"
            << "F.A."
            << "G.D.P";

    QStringList specialThanks;
    specialThanks
            << "This program is based on Mixxx. For more info visit www.mixxx.org";

    QString sectionTemplate = QString(
        "<p align=\"center\"><b>%1</b></p><p align=\"center\">%2</p>");
    QStringList sections;
    sections << sectionTemplate.arg(s_devTeam,
                                    thisReleaseDevelopers.join("<br>"))
             << sectionTemplate.arg(s_mixxxmentions,
                                    specialThanks.join("<br>"));
    textBrowser->setHtml(sections.join(""));

    textWebsiteLink->setText(
            QString("<a style=\"color:%1;\" href=\"%2\">%3</a>")
                    .arg(Color::blendColors(palette().link().color(),
                                 palette().text().color())
                                    .name(),
                            MIXXX_WEBSITE_URL,
                            tr("Official Website")));

    btnDonate->setText(tr("Addons"));
    connect(btnDonate, &QPushButton::clicked, this, [] {
        mixxx::DesktopHelper::openUrl(QUrl(MIXXX_ADDONS_URL));
    });

    connect(buttonBox, &QDialogButtonBox::accepted, this, &DlgAbout::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &DlgAbout::reject);
}

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
    QString s_specialThanks = tr("And special thanks to:");
    QString s_pastDevs = tr("Past Developers");
    QString s_pastContribs = tr("Past Contributors");

    QStringList thisReleaseDevelopers;
    thisReleaseDevelopers
            << "L.C."
            << "P.G.M"
            << "L.D.P"
            << "F.A."
            << "G.D.P";

    // This list should contains all contributors committed
    // code to the Mixxx core within the past two years.
    // New Contributors are added at the end.
    QStringList recentContributors;
    recentContributors
            << "-";
            
    QStringList specialThanks;
    specialThanks
            << "-";

    QStringList pastDevelopers;
    pastDevelopers
            << "-";

    QStringList pastContributors;
    pastContributors
            << "-";

    QString sectionTemplate = QString(
        "<p align=\"center\"><b>%1</b></p><p align=\"center\">%2</p>");
    QStringList sections;
    sections << sectionTemplate.arg(s_devTeam,
                                    thisReleaseDevelopers.join("<br>"))
             << sectionTemplate.arg(s_contributions,
                                    recentContributors.join("<br>"))
             << sectionTemplate.arg(s_pastDevs,
                                    pastDevelopers.join("<br>"))
             << sectionTemplate.arg(s_pastContribs,
                                    pastContributors.join("<br>"))
             << sectionTemplate.arg(s_specialThanks,
                                    specialThanks.join("<br>"));
    textBrowser->setHtml(sections.join(""));

    textWebsiteLink->setText(
            QString("<a style=\"color:%1;\" href=\"%2\">%3</a>")
                    .arg(Color::blendColors(palette().link().color(),
                                 palette().text().color())
                                    .name(),
                            MIXXX_WEBSITE_URL,
                            tr("Official Website")));
    /* if (std::rand() % 6) {
        if (!Color::isDimColor(palette().text().color())) {
            btnDonate->setIcon(QIcon(":/images/heart_icon_light.svg"));
        } else {
            btnDonate->setIcon(QIcon(":/images/heart_icon_dark.svg"));
        }
    } else {
        btnDonate->setIcon(QIcon(":/images/heart_icon_rainbow.svg"));
    }*/
    btnDonate->setText(tr("Addons"));
    connect(btnDonate, &QPushButton::clicked, this, [] {
        mixxx::DesktopHelper::openUrl(QUrl(MIXXX_DONATE_URL));
    });

    connect(buttonBox, &QDialogButtonBox::accepted, this, &DlgAbout::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &DlgAbout::reject);
}

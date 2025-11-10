// WinliveGoldSocket.cpp

#include "winlivegoldsocket.h"
#include "engine/enginebuffer.h"
#include "coreservices.h"
#include "moc_winlivegoldsocket.cpp" 
#include <QHostAddress>
#include <QDesktopServices>


WinliveGoldSocket::WinliveGoldSocket(QObject* parent)
    : QObject(parent), m_port(WGS_SERVER_PORT), m_infoTimer(new QTimer(this)), m_server(new QTcpServer(this)), m_client(nullptr)

{


    // Connect server signal
    connect(m_server, &QTcpServer::newConnection, this, &WinliveGoldSocket::onNewConnection);

    connect(m_infoTimer, &QTimer::timeout, this, [this]() {
        if (hasClient()) {
            // uodate karaoke info every second
            info();
        }
    });
}

WinliveGoldSocket::~WinliveGoldSocket() {
    qDebug() << "Closing Server";
    stopListening();
}

// Nel file cpp (WinliveGoldSocket.cpp)

bool WinliveGoldSocket::startListening() {
    if (isListening()) {
        qDebug() << "Server already listening";
        return true;
    }

    m_port = WGS_SERVER_PORT;

    for (quint16 attempt = 0; attempt < 16; ++attempt) {
        qDebug() << "Attempting to listen on port:" << m_port;


        // Try to listen on TCP port
        if (m_server->listen(QHostAddress::LocalHost, m_port)) {
            qDebug() << "TCP server successfully listening on port:" << m_port;
            return true;
        }

        // Log why it failed
        qDebug() << "Failed to listen on port" << m_port
                 << "- Error:" << m_server->errorString();

        // Try next port
        m_port++;
    }

    // All attempts failed
    QString error = QString("Failed to start server (ports %2-%3)")
                            .arg(WGS_SERVER_PORT)
                            .arg(m_port - 1);
    qDebug() << error;
    emit errorOccurred(error);
    return false;
}

void WinliveGoldSocket::stopListening() {
    if (!isListening()) {
        return;
    }

    qDebug() << "Stopping server...";
    m_infoTimer->stop();

    // Send close command to client if connected
    if (m_client) {
        QThread::msleep(250); // Give time for close command


        m_client->disconnectFromHost();

        m_client->deleteLater();
        m_client = nullptr;
    }

    m_server->close();
}

bool WinliveGoldSocket::isListening() const {
    return m_server->isListening();
}

bool WinliveGoldSocket::hasClient() const {
    return m_client != nullptr;
}

// Media control commands
void WinliveGoldSocket::start(EngineBuffer* deck, const WGSStartParams& params) {
    m_deck = deck;
    m_params = params;
   
    if (hasClient()) {
        m_params.pending = false;
        internalStart(m_params.filename, m_params.tone);
        deck->setKaraoke(true);
    } else {
        if (!m_params.pending) {
            qDebug() << "No client connected, launching client with start mode";
            m_params.pending = true;
            launchClient(deck);
            deck->setKaraoke(true);
        } else {
            qDebug() << "Already waiting to start....";
        }
    }
}

void WinliveGoldSocket::play() {
    qDebug() << "Sending play command";
    sendCommandToClient(WGS_COMMAND_PLAY);
}

void WinliveGoldSocket::pause() {
    qDebug() << "Sending pause command";
    sendCommandToClient(WGS_COMMAND_PAUSE);
}

void WinliveGoldSocket::stop() {
    qDebug() << "Sending stop command";
    sendCommandToClient(WGS_COMMAND_STOP);
}

void WinliveGoldSocket::ff() {
    qDebug() << "Sending fast forward command";
    sendCommandToClient(WGS_COMMAND_FF);
}

void WinliveGoldSocket::rw() {
    qDebug() << "Sending rewind command";
    sendCommandToClient(WGS_COMMAND_RW);
}

void WinliveGoldSocket::melody() {
    qDebug() << "Sending melody command";
    sendCommandToClient(WGS_COMMAND_MELODY);
}

void WinliveGoldSocket::tone(const QString& newTone) {
    QString command = QString(WGS_COMMAND_TONE + "|%1").arg(newTone);
    qDebug() << "Sending tone command:" << command;
    sendCommandToClient(command);
}

void WinliveGoldSocket::close() {
    qDebug() << "Sending close command";
    sendCommandToClient(WGS_COMMAND_CLOSE);
}

void WinliveGoldSocket::info() {
    qDebug() << "Sending info request";
    sendCommandToClient(WGS_COMMAND_INFO);
}


void WinliveGoldSocket::onNewConnection() {
    qDebug() << "New connection attempt";

    // If we already have a client, reject the new connection
    if (m_client) {
        qDebug() << "Rejecting connection - client already connected";

        QTcpSocket* rejectedClient = m_server->nextPendingConnection();
        if (rejectedClient) {
            rejectedClient->write(("dummy " + WGS_COMMAND_CLOSE + "\n").toUtf8());
            rejectedClient->flush();
            rejectedClient->disconnectFromHost();
            rejectedClient->deleteLater();
        }
        return;
    }

    // Accept the first client
    m_client = m_server->nextPendingConnection();
    if (!m_client) {
        qDebug() << "Failed to get pending connection";
        return;
    }

    connect(m_client, &QTcpSocket::disconnected, this, &WinliveGoldSocket::onClientDisconnected);
    connect(m_client, &QTcpSocket::readyRead, this, &WinliveGoldSocket::onClientDataReceived);


    qDebug() << "Client connected successfully";
    emit clientConnected();

    if (m_params.pending) {
        qDebug() << "Client connected after launch. Sending start command.";
        internalStart(m_params.filename, m_params.tone);
        m_params.pending = false;
    }

    // start info timer
    m_infoTimer->start(1000);
}

void WinliveGoldSocket::onClientDisconnected() {
    qDebug() << "Client disconnected";

    if (m_client) {
        m_client->deleteLater();
        m_client = nullptr;     
    }
    m_infoTimer->stop();
    m_deck->setClientInLaunching(false); // reset launching flag
    emit clientDisconnected();
}

void WinliveGoldSocket::onClientDataReceived() {
    if (!m_client || !m_deck) {
        return;
    }

    QByteArray data = m_client->readAll();
    QString response = QString::fromUtf8(data).trimmed();

    if (response.isEmpty()) {
        return;
    }
    
    m_deck->setClientInLaunching(false); // reset launching flag on any response
    qDebug() << "Received from client:" << response;
    processClientResponse(response);
}

void WinliveGoldSocket::processClientResponse(const QString& response) {
    // General acknowledgment or status update
    qDebug() << "Client response:" << response;
    emit clientInfo(m_deck, response);
}

void WinliveGoldSocket::sendCommandToClient(const QString& command) {
    if (!m_deck || !m_client || !m_client->isWritable()) {
        qDebug() << "No client connected - cannot send command:" << command;
        return;
    }

    QString message = m_deck->getGroup() + "|" + command + "\n";
    QByteArray data = message.toUtf8();
    qint64 written = m_client->write(data);

    if (written == -1) {
        qDebug() << "Failed to send command to client:" << command;
        emit errorOccurred(QString("Failed to send command: %1").arg(command));
    } else if (written != data.size()) {
        qDebug() << "Partial write - expected" << data.size() << "bytes, wrote" << written;
    } else {
        qDebug() << "Successfully sent command:" << command;
    }

    // Ensure data is sent immediately
    m_client->flush(); 
}


void WinliveGoldSocket::launchClient(EngineBuffer* deck) {
   
 QString program;
 QStringList arguments;
#if defined(Q_OS_WIN)
   
    if (auto* coreServices = mixxx::CoreServices::getInstance()) {
         program = QDir(coreServices->getResourcePath()).filePath("karaokew/karaokew.exe");
    }


    // Verifica se il file esiste
    if (!QFile::exists(program)) {
        // Avvia il download
        downloadKaraokeClient(program);
        return;
    }

#elif defined(Q_OS_MAC)
     if (auto* coreServices = mixxx::CoreServices::getInstance()) {
         program = "/Applications/WlDjAiKaraoke.app/Contents/MacOS/WlDjAiKaraoke";
         qDebug() << "Launching app:" << program;
         //QMessageBox::information(nullptr, "settingpath", programPath);
     }
    /// Verifica se il file esiste
    if (!QFile::exists(program)) {
        QMessageBox msgBox;
        msgBox.setIcon(QMessageBox::Information);
        msgBox.setWindowTitle(tr("Warning"));
        msgBox.setText(tr("Il plugin is not installed in applications "));
        msgBox.setInformativeText(tr("Do you want download and install Karaoke plugin ?"));
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        msgBox.setDefaultButton(QMessageBox::Yes);

        int ret = msgBox.exec();

        if (ret == QMessageBox::Yes) {
            QUrl downloadUrl("https://www.promusicsoftware.com/download.php?filename=WlDjAiKaraoke.pkg");
            if (!QDesktopServices::openUrl(downloadUrl)) {
                QMessageBox::warning(nullptr,
                    "Error",
                    "Cannot open browser.\nDownload manually from:\n" + downloadUrl.toString());
            }
        }

        return;
    }

#else
    QString program = "/usr/bin/winliveclient";
#endif
    
    if (m_deck->isClientInLaunching()) {
        qDebug() << "Client is already being launched, skipping launchClient call.";
        return;
    }

    arguments << "-refwldjainomidi" << QString::number(m_port);
    
    m_deck->setClientInLaunching(true); // flag to indicate client is being launched
    QProcess* process = new QProcess(this);
    process->start(program, arguments);

    deck->setKaraoke(true);
    
    qDebug() << "Launched client with arguments:" << arguments;
 
   
}

void WinliveGoldSocket::internalStart(const QString& filename, const QString& tone) {
    // avoid recursice, senddirectly command
    sendCommandToClient(QString("%1|%2|%3").arg(WGS_COMMAND_START, filename, tone));
}

void WinliveGoldSocket::downloadKaraokeClient(const QString& destinationPath) {
    // URL del file da scaricare (sostituisci con l'URL reale)
    QUrl url("https://www.promusicsoftware.com/download.php?filename=WlDjAiKaraoke.zip");

    // Crea la directory se non esiste
    QFileInfo fileInfo(destinationPath);
    QDir().mkpath(fileInfo.absolutePath());

    // Crea il progress dialog MODALE
    QProgressDialog* progressDialog = new QProgressDialog(
            "Download karaoke plugin ...",
            "Annulla",
            0,
            100);
    progressDialog->setWindowTitle("Download");
    progressDialog->setWindowModality(Qt::ApplicationModal); // MODALE per tutta l'applicazione
    progressDialog->setWindowFlags(Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint); // <-- AGGIUNGI QUESTA
    progressDialog->setMinimumDuration(0);
    progressDialog->setCancelButton(nullptr); // Opzionale: rimuovi il pulsante annulla
    progressDialog->setAutoClose(false);
    progressDialog->setAutoReset(false);
    progressDialog->show();

    // Setup network manager
    QNetworkAccessManager* manager = new QNetworkAccessManager(this);
    QNetworkRequest request(url);
    QNetworkReply* reply = manager->get(request);

    // File temporaneo per il download
    QFile* file = new QFile(destinationPath + ".zip");
    if (!file->open(QIODevice::WriteOnly)) {
        progressDialog->close();
        delete progressDialog;
        QMessageBox::critical(nullptr, "Errore", "Impossibile creare il file temporaneo");
        delete file;
        return;
    }

    // Aggiorna la progress bar
    connect(reply, &QNetworkReply::downloadProgress, [progressDialog](qint64 bytesReceived, qint64 bytesTotal) {
        if (bytesTotal > 0) {
            int percentage = (bytesReceived * 100) / bytesTotal;
            progressDialog->setValue(percentage);

            // Mostra velocit� e tempo rimanente
            QString label = QString("Download: %1 MB / %2 MB\nAttendere prego...")
                                    .arg(bytesReceived / 1024.0 / 1024.0, 0, 'f', 2)
                                    .arg(bytesTotal / 1024.0 / 1024.0, 0, 'f', 2);
            progressDialog->setLabelText(label);
        }
    });

    // Scrivi i dati nel file
    connect(reply, &QNetworkReply::readyRead, [reply, file]() {
        file->write(reply->readAll());
    });

    // Gestisci il completamento
    connect(reply, &QNetworkReply::finished, [this, reply, file, destinationPath, progressDialog, manager]() {
        file->close();

        if (reply->error() == QNetworkReply::NoError) {
            // Rinomina il file temporaneo
            QFile::remove(destinationPath);
            file->rename(destinationPath);

            progressDialog->setLabelText(tr("Installing karaoke plugin ..."));

            QFileInfo fileInfo(destinationPath);
            QString extractPath = fileInfo.absolutePath();

            // Crea la directory di estrazione
            QDir().mkpath(extractPath);

            // Usa unzip del sistema
            QProcess unzip;
            unzip.start("unzip", QStringList() << "-o" << destinationPath << "-d" << extractPath);

            if (!unzip.waitForFinished(30000)) {  // timeout 30 secondi
                progressDialog->close();
                delete progressDialog;
                QMessageBox::critical(nullptr, "Error", "Timeout while installing!");
                return;
            }

            if (unzip.exitCode() != 0) {
                progressDialog->close();
                delete progressDialog;
                QMessageBox::critical(nullptr, "Error",
                    "Error while extracting:\n" );
                return;
            }

            progressDialog->close();
            delete progressDialog;

            QMessageBox::information(nullptr, "Completed", "Plugin installed!\nNow you can start your karaoke");

        } else {
            progressDialog->close();
            delete progressDialog;

            QMessageBox::critical(nullptr, "Errore", "Errore durante il download: " + reply->errorString());
            file->remove();
        }

        delete file;
        reply->deleteLater();
        manager->deleteLater();
    });
}
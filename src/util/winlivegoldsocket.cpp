// WinliveGoldSocket.cpp

#include "winlivegoldsocket.h"
#include "engine/enginebuffer.h"
#include "coreservices.h"
#include "moc_winlivegoldsocket.cpp" 
#include <QHostAddress>



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
    } else {
        if (!m_params.pending) {
            qDebug() << "No client connected, launching client with start mode";
            m_params.pending = true;
            launchClient();
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


void WinliveGoldSocket::launchClient() {
   
 QString program;
 QStringList arguments;
#if defined(Q_OS_WIN)
   
    if (auto* coreServices = mixxx::CoreServices::getInstance()) {
         program = QDir(coreServices->getResourcePath()).filePath("karaokew/karaokew.exe");
    }
#elif defined(Q_OS_MAC)
     if (auto* coreServices = mixxx::CoreServices::getInstance()) {
         program = QDir(coreServices->getResourcePath()).filePath("WinliveGold_DjAiKaraoke.app/Contents/MacOS/WinliveGold_DjAiKaraoke");
     }
#else
    QString program = "/usr/bin/winliveclient";
#endif

   
    arguments << "-refwldjainomidi" << QString::number(m_port);

    QProcess* process = new QProcess(this);
    process->start(program, arguments);

    qDebug() << "Launched client with arguments:" << arguments;
}

void WinliveGoldSocket::internalStart(const QString& filename, const QString& tone) {
    // avoid recursice, senddirectly command
    sendCommandToClient(QString("%1|%2|%3").arg(WGS_COMMAND_START, filename, tone));
}

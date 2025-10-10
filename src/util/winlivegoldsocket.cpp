// WinliveGoldSocket.cpp

#include "winlivegoldsocket.h"
#include "engine/enginebuffer.h"
#include "moc_winlivegoldsocket.cpp" 
#include <QHostAddress>


#if defined(Q_OS_MAC)
    WinliveGoldSocket::WinliveGoldSocket(QObject* parent)
        : QObject(parent), m_serverName(WGS_SERVER_NAME), m_infoTimer(new QTimer(this)), m_server(new QLocalServer(this)), m_client(nullptr)
#else
    WinliveGoldSocket::WinliveGoldSocket(QObject* parent)
        : QObject(parent), m_port(WGS_SERVER_PORT), m_infoTimer(new QTimer(this)), m_server(new QTcpServer(this)), m_client(nullptr)
#endif
{



    // Connect server signal
#if defined(Q_OS_MAC)
    connect(m_server, &QLocalServer::newConnection, this, &WinliveGoldSocket::onNewConnection);
#else
    connect(m_server, &QTcpServer::newConnection, this, &WinliveGoldSocket::onNewConnection);
#endif

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

#if defined(Q_OS_MAC)
        // For QLocalServer, port doesn't apply - use server name directly
        if (m_server->listen(m_serverName)) {
            qDebug() << "Local server listening on:" << m_serverName;
            return true;
        }
#else
        // Try to listen on TCP port
        if (m_server->listen(QHostAddress::LocalHost, m_port)) {
            qDebug() << "TCP server successfully listening on port:" << m_port;
            return true;
        }

        // Log why it failed
        qDebug() << "Failed to listen on port" << m_port
                 << "- Error:" << m_server->errorString();
#endif

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

#if defined(Q_OS_MAC)
        m_client->disconnectFromServer();
#else
        m_client->disconnectFromHost();
#endif
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
void WinliveGoldSocket::start(EngineBuffer* deck, const QString& filename) {
    m_deck = deck;
    if (hasClient()) {
        sendCommandToClient(WGS_COMMAND_START + " " + filename);
    } else {
        qDebug() << "No client connected, launching client with start mode";
        m_pendingStartFilename = filename;
        launchClient();
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
    QString command = QString(WGS_COMMAND_TONE + " %1").arg(newTone);
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
#if defined(Q_OS_MAC)
        QLocalSocket* rejectedClient = m_server->nextPendingConnection();
        if (rejectedClient) {
            rejectedClient->write("dummy " + WGS_COMMAND_CLOSE +  "\n");
            rejectedClient->flush();
            rejectedClient->disconnectFromServer();
            rejectedClient->deleteLater();
        }
#else
        QTcpSocket* rejectedClient = m_server->nextPendingConnection();
        if (rejectedClient) {
            rejectedClient->write(("dummy " + WGS_COMMAND_CLOSE + "\n").toUtf8());
            rejectedClient->flush();
            rejectedClient->disconnectFromHost();
            rejectedClient->deleteLater();
        }
#endif
        return;
    }

    // Accept the first client
#if defined(Q_OS_MAC)
    m_client = m_server->nextPendingConnection();
    if (!m_client) {
        qDebug() << "Failed to get pending connection";
        return;
    }

    connect(m_client, &QLocalSocket::disconnected, this, &WinliveGoldSocket::onClientDisconnected);
    connect(m_client, &QLocalSocket::readyRead, this, &WinliveGoldSocket::onClientDataReceived);
#else
    m_client = m_server->nextPendingConnection();
    if (!m_client) {
        qDebug() << "Failed to get pending connection";
        return;
    }

    connect(m_client, &QTcpSocket::disconnected, this, &WinliveGoldSocket::onClientDisconnected);
    connect(m_client, &QTcpSocket::readyRead, this, &WinliveGoldSocket::onClientDataReceived);
#endif

    qDebug() << "Client connected successfully";
    emit clientConnected();

    if (m_pendingStartFilename.length() > 0) {
        qDebug() << "Client connected after launch. Sending start command.";
        start(m_deck, m_pendingStartFilename);
        m_pendingStartFilename.clear();
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

    QString message = m_deck->getGroup() + " " + command + "\n";
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
#if defined(Q_OS_WIN)
    QString program = "C:/000/WLGOLDTEST/WinliveGold.exe";
#elif defined(Q_OS_MAC)
    QString program = "/Applications/WinliveClient.app/Contents/MacOS/WinliveClient";
#else
    QString program = "/usr/bin/winliveclient";
#endif

    QStringList arguments;
    arguments << "-refwldjainomidi" << QString::number(m_port);

    QProcess* process = new QProcess(this);
    process->start(program, arguments);

    qDebug() << "Launched client with arguments:" << arguments;
}

// WinliveGoldSocket.cpp

#include "WinliveGoldSocket.h"

#include <QHostAddress>

// Platform-specific constructor implementations
#if defined(Q_OS_MAC)
WinliveGoldSocket::WinliveGoldSocket(const QString& serverName, QObject* parent)
        : QObject(parent), m_serverName(serverName)
#else
WinliveGoldSocket::WinliveGoldSocket(const QString& host, quint16 port, QObject* parent)
        : QObject(parent), m_host(host), m_port(port)
#endif
          ,
          m_socket(nullptr),
          m_messageTimer(new QTimer(this)),
          m_messageInterval(1000),
          m_waitingForInfo(false),
          m_waitingForPing(false),
          m_pingTimer(new QTimer(this)),
          m_isConnected(false),
          m_waitingForStartConnection(false) {
    initializeSocket();

    // Configure message timer for sequential message sending
    m_messageTimer->setSingleShot(true);
    connect(m_messageTimer, &QTimer::timeout, this, &WinliveGoldSocket::sendNextMessage);

    // Configure ping timeout timer
    m_pingTimer->setSingleShot(true);
    m_pingTimer->setInterval(PING_TIMEOUT_MS);
    connect(m_pingTimer, &QTimer::timeout, this, &WinliveGoldSocket::onPingTimeout);
}

WinliveGoldSocket::~WinliveGoldSocket() {
    if (m_socket && m_isConnected) {
        disconnectFromHost();
    }
}

void WinliveGoldSocket::initializeSocket() {
#if defined(Q_OS_MAC)
    // Initialize QLocalSocket for macOS IPC
    m_socket = new QLocalSocket(this);
    connect(m_socket, &QLocalSocket::connected, this, &WinliveGoldSocket::onSocketConnected);
    connect(m_socket, &QLocalSocket::disconnected, this, &WinliveGoldSocket::onSocketDisconnected);
    connect(m_socket, &QLocalSocket::readyRead, this, &WinliveGoldSocket::onDataReceived);
    connect(m_socket, QOverload<QLocalSocket::LocalSocketError>::of(&QLocalSocket::errorOccurred), this, &WinliveGoldSocket::onSocketError);
#else
    // Initialize QTcpSocket for Windows/Linux TCP communication
    m_socket = new QTcpSocket(this);
    connect(m_socket, &QTcpSocket::connected, this, &WinliveGoldSocket::onSocketConnected);
    connect(m_socket, &QTcpSocket::disconnected, this, &WinliveGoldSocket::onSocketDisconnected);
    connect(m_socket, &QTcpSocket::readyRead, this, &WinliveGoldSocket::onDataReceived);
    connect(m_socket, QOverload<QAbstractSocket::SocketError>::of(&QTcpSocket::errorOccurred), this, &WinliveGoldSocket::onSocketError);
#endif
}

// Connection management methods
void WinliveGoldSocket::connectToHost() {
    if (m_isConnected) {
        qDebug() << "Already connected to host";
        return;
    }

    qDebug() << "Attempting to connect to host...";
#if defined(Q_OS_MAC)
    m_socket->connectToServer(m_serverName);
#else
    m_socket->connectToHost(m_host, m_port);
#endif
}

void WinliveGoldSocket::disconnectFromHost() {
    if (!m_isConnected) {
        qDebug() << "Already disconnected from host";
        return;
    }

    qDebug() << "Disconnecting from host...";
#if defined(Q_OS_MAC)
    m_socket->disconnectFromServer();
#else
    m_socket->disconnectFromHost();
#endif
}

bool WinliveGoldSocket::isConnected() const {
    return m_isConnected;
}

// Message queue management
void WinliveGoldSocket::sendMessages(const QStringList& messages, int intervalMs) {
    m_messageQueue = messages;
    m_messageInterval = intervalMs;

    qDebug() << "Queued" << messages.size() << "messages with interval" << intervalMs << "ms";

    if (!m_isConnected) {
        qDebug() << "Not connected - attempting connection before sending messages";
        connectToHost();
    } else {
        sendNextMessage();
    }
}

// Media control command implementations with connection check and auto-launch
void WinliveGoldSocket::start(const QString& filename) {
    qDebug() << "Start command requested with filename:" << filename;

    // If already connected, send command immediately
    if (isConnected()) {
        qDebug() << "Client connected - sending start command immediately";
        sendCommand(QString("start %1").arg(filename));
        return;
    }

    // Store pending command for delayed execution after connection
    m_pendingStartFilename = filename;
    m_waitingForStartConnection = true;

    qDebug() << "Client not connected - initiating auto-launch sequence";

    // First attempt: direct connection (client might be running but disconnected)
    connectToHost();

    // Set timeout to trigger ping check if direct connection fails
    QTimer::singleShot(CONNECTION_RETRY_DELAY_MS, this, [this]() {
        if (m_waitingForStartConnection && !isConnected()) {
            qDebug() << "Direct connection failed - checking client availability with ping";
            ping(); // This will trigger client launch if ping times out
        }
    });
}

void WinliveGoldSocket::play() {
    sendCommand("play");
}

void WinliveGoldSocket::pause() {
    sendCommand("pause");
}

void WinliveGoldSocket::stop() {
    sendCommand("stop");
}

void WinliveGoldSocket::ff() {
    sendCommand("ff");
}

void WinliveGoldSocket::rw() {
    sendCommand("rw");
}

void WinliveGoldSocket::melody() {
    sendCommand("melody");
}

void WinliveGoldSocket::tone(const QString& newTone) {
    sendCommand(QString("tone %1").arg(newTone));
}

void WinliveGoldSocket::close() {
    sendCommand("close");
}

void WinliveGoldSocket::info() {
    qDebug() << "Requesting info from client";
    m_waitingForInfo = true;
    sendCommand("info");
}

void WinliveGoldSocket::ping() {
    qDebug() << "Sending ping to client";
    m_waitingForPing = true;
    m_pingTimer->start();
    sendCommand("ping");
}

// Cross-platform client launcher
void WinliveGoldSocket::launchClient(const QString& mode, const QString& filename) {
    qDebug() << "Launching client with mode:" << mode << "and filename:" << filename;

#if defined(Q_OS_WIN)
    QString program = "C:/000/WLGOLDTEST/WinliveGold.exe"; // Update with actual Windows path
#elif defined(Q_OS_MAC)
    QString program = "/Applications/WinliveClient.app/Contents/MacOS/WinliveClient"; // Update with actual macOS path
#else
    QString program = "/usr/bin/winliveclient"; // Fallback for Linux
#endif

    QStringList arguments;
    arguments << mode << filename;

    QProcess* process = new QProcess(this);

    // Handle successful process start
    connect(process, &QProcess::started, [this, mode, filename]() {
        qDebug() << "Client process started successfully with mode:" << mode << "filename:" << filename;
    });

    // Handle process completion
    connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), [this, process](int exitCode, QProcess::ExitStatus exitStatus) {
        Q_UNUSED(exitStatus)
        qDebug() << "Client process finished with exit code:" << exitCode;
        process->deleteLater();
    });

    // Handle process launch errors
    connect(process, &QProcess::errorOccurred, [this, process, mode, filename](QProcess::ProcessError error) {
        QString errorMsg = QString("Failed to launch client (mode: %1, filename: %2) - Error: %3")
                                   .arg(mode, filename)
                                   .arg(static_cast<int>(error));
        qDebug() << errorMsg;
        emit errorOccurred(errorMsg);
        process->deleteLater();
    });

    // Start the client process
    process->start(program, arguments);
}

// Private slot implementations
void WinliveGoldSocket::onSocketConnected() {
    m_isConnected = true;
    qDebug() << "Socket connection established successfully";
    emit connected();

    // Execute pending start command if waiting
    if (m_waitingForStartConnection && !m_pendingStartFilename.isEmpty()) {
        qDebug() << "Connection established - executing delayed start command with filename:"
                 << m_pendingStartFilename;
        sendCommand(QString("start %1").arg(m_pendingStartFilename));

        // Clear pending state
        m_pendingStartFilename.clear();
        m_waitingForStartConnection = false;
    }

    // Process any queued messages
    if (!m_messageQueue.isEmpty()) {
        qDebug() << "Processing queued messages after connection";
        sendNextMessage();
    }
}

void WinliveGoldSocket::onSocketDisconnected() {
    m_isConnected = false;
    qDebug() << "Socket disconnected";
    emit disconnected();
}

void WinliveGoldSocket::onSocketError() {
    QString errorString;
#if defined(Q_OS_MAC)
    errorString = QString("LocalSocket error: %1").arg(m_socket->errorString());
#else
    errorString = QString("TcpSocket error: %1").arg(m_socket->errorString());
#endif

    qDebug() << errorString;
    emit errorOccurred(errorString);
}

void WinliveGoldSocket::sendNextMessage() {
    if (m_messageQueue.isEmpty()) {
        qDebug() << "Message queue is empty";
        return;
    }

    QString message = m_messageQueue.takeFirst();
    qDebug() << "Sending queued message:" << message << "(" << m_messageQueue.size() << "remaining)";
    sendCommand(message);

    // Schedule next message if queue not empty
    if (!m_messageQueue.isEmpty()) {
        m_messageTimer->start(m_messageInterval);
    }
}

void WinliveGoldSocket::onDataReceived() {
    QByteArray data = m_socket->readAll();
    QString response = QString::fromUtf8(data).trimmed();

    if (response.isEmpty()) {
        qDebug() << "Received empty response - ignoring";
        return;
    }

    qDebug() << "Data received from client:" << response;
    handleResponse(response);
}

void WinliveGoldSocket::onPingTimeout() {
    if (m_waitingForPing) {
        m_waitingForPing = false;
        qDebug() << "Ping timeout occurred";

        // Handle start command auto-launch scenario
        if (m_waitingForStartConnection) {
            qDebug() << "Client not responding to ping - launching client with start mode and filename:"
                     << m_pendingStartFilename;
            launchClient("-refwldjai", m_pendingStartFilename);

            // Attempt connection after allowing time for client startup
            QTimer::singleShot(CLIENT_LAUNCH_DELAY_MS, this, [this]() {
                qDebug() << "Attempting connection to newly launched client";
                connectToHost();
            });
        } else {
            qDebug() << "Ping timeout - client not responding";
            emit pingTimeout();
        }
    }
}

// Private helper methods
void WinliveGoldSocket::sendCommand(const QString& command) {
    if (!m_isConnected) {
        qDebug() << "Cannot send command - socket not connected:" << command;
        emit errorOccurred(QString("Cannot send command '%1' - socket not connected").arg(command));
        return;
    }

    // Format command with newline termination for protocol compliance
    QString formattedCommand = command + "\n";
    QByteArray data = formattedCommand.toUtf8();

    qint64 bytesWritten = m_socket->write(data);
    if (bytesWritten == -1) {
        QString errorMsg = QString("Failed to write command to socket: %1").arg(command);
        qDebug() << errorMsg;
        emit errorOccurred(errorMsg);
    } else if (bytesWritten != data.size()) {
        QString warningMsg = QString("Partial write - expected %1 bytes, wrote %2 bytes for command: %3")
                                     .arg(data.size())
                                     .arg(bytesWritten)
                                     .arg(command);
        qDebug() << warningMsg;
    } else {
        qDebug() << "Successfully sent command:" << command;
    }
}

void WinliveGoldSocket::handleResponse(const QString& response) {
    qDebug() << "Processing server response:" << response;

    if (m_waitingForInfo) {
        m_waitingForInfo = false;
        qDebug() << "Received info response:" << response;
        emit infoReceived(response);
    } else if (m_waitingForPing) {
        m_waitingForPing = false;
        m_pingTimer->stop();

        // Handle case where client responds to ping but socket not connected
        if (m_waitingForStartConnection && !isConnected()) {
            qDebug() << "Client responded to ping but socket not connected - establishing connection";
            connectToHost();
        } else {
            qDebug() << "Received ping response:" << response;
            emit pingResponse(response);
        }
    } else {
        qDebug() << "Received generic response:" << response;
        emit genericResponse(response);
    }
}

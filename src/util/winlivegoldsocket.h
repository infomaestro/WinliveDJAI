// WinliveGoldSocket.h

#ifndef WINLIVEGOLDSOCKET_H
#define WINLIVEGOLDSOCKET_H

#include <QDebug>
#include <QObject>
#include <QProcess>
#include <QTimer>

#if defined(Q_OS_MAC)
#include <QLocalSocket>
#else
#include <QTcpSocket>
#endif

/*!
 * \class WinliveGoldSocket
 * \brief Cross-platform socket client for media control commands
 *
 * Provides cross-platform socket communication using QLocalSocket on macOS
 * and QTcpSocket on Windows/Linux. Supports asynchronous command sending
 * and response handling for media playback control. Automatically launches
 * client if not running when start command is issued.
 */
class WinliveGoldSocket : public QObject {
    Q_OBJECT

  public:
#if defined(Q_OS_MAC)
    explicit WinliveGoldSocket(const QString& serverName, QObject* parent = nullptr);
#else
    explicit WinliveGoldSocket(const QString& host, quint16 port, QObject* parent = nullptr);
#endif

    ~WinliveGoldSocket();

    void sendMessages(const QStringList& messages, int intervalMs = 1000);
    bool isConnected() const;

  public slots:
    // Media control commands
    void start(const QString& filename);
    void play();
    void pause();
    void stop();
    void ff();
    void rw();
    void melody();
    void tone(const QString& newTone);
    void close();
    void info();
    void ping();

    // Client management
    void launchClient(const QString& mode, const QString& filename);
    void connectToHost();
    void disconnectFromHost();

  signals:
    void connected();
    void disconnected();
    void infoReceived(const QString& info);
    void pingResponse(const QString& status);
    void pingTimeout();
    void genericResponse(const QString& message);
    void errorOccurred(const QString& error);

  private slots:
    void onSocketConnected();
    void onSocketDisconnected();
    void onSocketError();
    void sendNextMessage();
    void onDataReceived();
    void onPingTimeout();

  private:
    void initializeSocket();
    void sendCommand(const QString& command);
    void handleResponse(const QString& response);

    // Cross-platform socket implementation
#if defined(Q_OS_MAC)
    QString m_serverName;
    QLocalSocket* m_socket;
#else
    QString m_host;
    quint16 m_port;
    QTcpSocket* m_socket;
#endif

    // Message queue and timing
    QStringList m_messageQueue;
    QTimer* m_messageTimer;
    int m_messageInterval;

    // Response handling
    bool m_waitingForInfo;
    bool m_waitingForPing;
    QTimer* m_pingTimer;

    // Connection state
    bool m_isConnected;

    // Start command with auto-launch support
    QString m_pendingStartFilename;
    bool m_waitingForStartConnection;

    static const int PING_TIMEOUT_MS = 500;
    static const int CLIENT_LAUNCH_DELAY_MS = 2000;
    static const int CONNECTION_RETRY_DELAY_MS = 1000;
};

#endif // WINLIVEGOLDSOCKET_H

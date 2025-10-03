// WinliveGoldSocket.h

#ifndef WINLIVEGOLDSOCKET_H
#define WINLIVEGOLDSOCKET_H

#include <QDebug>
#include <QObject>
#include <QProcess>
#include <QTimer>

#if defined(Q_OS_MAC)
#include <QLocalServer>
#include <QLocalSocket>
#else
#include <QTcpServer>
#include <QTcpSocket>
#endif

/*!
 * \class WinliveGoldSocket
 * \brief Cross-platform socket server for single client media control
 *
 * Simple server that accepts one client connection and sends media control
 * commands. Automatically rejects additional connection attempts.
 */
class WinliveGoldSocket : public QObject {
    Q_OBJECT

  public:
#if defined(Q_OS_MAC)
    explicit WinliveGoldSocket(const QString& serverName, QObject* parent = nullptr);
#else
    explicit WinliveGoldSocket(quint16 port, QObject* parent = nullptr);
#endif

    ~WinliveGoldSocket();

    // Server management
    bool startListening();
    void stopListening();
    bool isListening() const;
    bool hasClient() const;

    // Client launcher
    void launchClient();

  public slots:

    // Media control commands - send to the connected client
    void start(const QString& deck, const QString& filename);
    void play();
    void pause();
    void stop();
    void ff();
    void rw();
    void melody();
    void tone(const QString& newTone);
    void close();
    void info();

  signals:
    void clientConnected();
    void clientDisconnected();
    void clientReady();
    void errorOccurred(const QString& error);   
    void clientInfo(const QString& info);   

  private slots:
    void onNewConnection();
    void onClientDisconnected();
    void onClientDataReceived();

  private:
    void processClientResponse(const QString& response);
    void sendCommandToClient(const QString& command);
    QTimer* m_infoTimer;
    QString m_pendingStartFilename;
    QString m_deck;

#if defined(Q_OS_MAC)
    QString m_serverName;   
    QLocalServer* m_server;
    QLocalSocket* m_client;
#else
    quint16 m_port;
    QTcpServer* m_server;
    QTcpSocket* m_client;
#endif
};

#endif // WINLIVEGOLDSOCKET_H

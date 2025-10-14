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

class EngineBuffer;

const QString WGS_COMMAND_START = QStringLiteral("start");
const QString WGS_COMMAND_PLAY = QStringLiteral("play");
const QString WGS_COMMAND_PAUSE = QStringLiteral("pause");
const QString WGS_COMMAND_STOP = QStringLiteral("stop");
const QString WGS_COMMAND_FF = QStringLiteral("ff");
const QString WGS_COMMAND_RW = QStringLiteral("rw");
const QString WGS_COMMAND_MELODY = QStringLiteral("melody");
const QString WGS_COMMAND_TONE = QStringLiteral("tone");
const QString WGS_COMMAND_CLOSE = QStringLiteral("close");
const QString WGS_COMMAND_INFO = QStringLiteral("info");
const QString WGS_COMMAND_FINISH = QStringLiteral("finish");
const quint16 WGS_SERVER_PORT = 12345;
const QString WGS_SERVER_NAME = QStringLiteral("wldjai");

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
    explicit WinliveGoldSocket(QObject* parent = nullptr);
#else
    explicit WinliveGoldSocket(QObject* parent = nullptr);
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
    void start(EngineBuffer* deck, const QString& filename);
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
    void clientInfo(EngineBuffer* deck, const QString & info);   

  private slots:
    void onNewConnection();
    void onClientDisconnected();
    void onClientDataReceived();

  private:
    void processClientResponse(const QString& response);
    void sendCommandToClient(const QString& command);
    QTimer* m_infoTimer = nullptr;
    QString m_pendingStartFilename = "";
    EngineBuffer* m_deck = nullptr;
   

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

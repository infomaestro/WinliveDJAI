// WinliveGoldSocket.h
#pragma once


#include <QDebug>
#include <QObject>
#include <QProcess>
#include <QTimer>
#include <QTcpServer>
#include <QTcpSocket>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QProgressDialog>
#include <QMessageBox>
#include <QFile>
#include <QFileInfo>

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
const quint16 KP_STATUS_STOPPED = 0;
const quint16 KP_STATUS_PLAYING = 1;
const quint16 KP_STATUS_STALLED = 2;
const quint16 KP_STATUS_PAUSED = 3;

struct WGSStartParams {
    QString filename;
    QString tone;
    bool melody;
    bool pending;

    WGSStartParams(
            const QString& file = "",
            const QString& tone = "",
            bool melody = false)
            : filename(file), tone(tone), melody(melody), pending(false) {
    }
};

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
    explicit WinliveGoldSocket(QObject* parent = nullptr);


    
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
    void start(EngineBuffer* deck, const WGSStartParams& params);
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
    void internalStart(const QString& filename, const QString& tone);
    void downloadKaraokeClient(const QString& destinationPath);
 

    QTimer* m_infoTimer = nullptr;
    WGSStartParams m_params;
    EngineBuffer* m_deck = nullptr;
   
    quint16 m_port;
    QTcpServer* m_server;
    QTcpSocket* m_client;
    

};


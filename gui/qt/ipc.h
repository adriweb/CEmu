#ifndef IPC_H
#define IPC_H

#include <QtCore/QFileInfo>
#include <QtNetwork/QLocalServer>
#include <QtNetwork/QLocalSocket>

// different types of available streams
enum {
    IPC_NONE=0,
    IPC_COMMANDLINEOPTIONS
};

class ipc : public QObject {
    Q_OBJECT
public:
    explicit ipc(QObject *p = Q_NULLPTR);
    void serverSetup(const QString& name);
    void clientSetup(const QString& name);
    void serverListen();
    void idClose();
    void send(const QByteArray& pkt);

    QString getServerName();
    QString getHostName();

    bool ipcSetup(const QString& id, const QString& pid);
    static bool idOpen(const QString& name);

    QByteArray getData();

signals:
    void readDone();

private:
    void accepted();

    // server
    QLocalServer *server;
    bool serverSet = false;
    bool clientSet = false;
    QString serverName;

    QFile file;

    // client
    QLocalSocket *socket;
    QString hostName;

    QByteArray data;
};

#endif

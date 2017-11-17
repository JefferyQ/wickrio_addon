#ifndef OPERATIONDATA_H
#define OPERATIONDATA_H

#include <QCoreApplication>
#include <QDebug>
#include <QFile>
#include <QByteArray>
#include <QObject>

#include "wickrbotlib.h"
#ifndef DO_WICKRBOT
#include "filedownloader.h"
#endif
#include "wickrbotlog.h"
#include "wickrbotdatabase.h"
#include "wickrbotclients.h"
#include "loghandler.h"

class DECLSPEC OperationData : public QObject
{
Q_OBJECT
public:
    OperationData();
    ~OperationData();

public:
    QString processName;
    bool debug;
    bool force;

    QString appDataDir;

    QString outputFile;
    bool gotOutputFile;

    QString inputFile;
    bool gotInputFile;

    QString attachmentsDir;
    QString databaseDir;
    QString messagesDir;

    int messageCount;
    int duration;

    WickrBotDatabase *m_botDB;
    WickrBotClients *m_client;

    bool receiveOn;

    int cleanUpSecs;
    int startRcvSecs;
    int m_appTimeOut;   // number of seconds to wait for potentiall hung application
    int m_pid;          // The process ID of this application

    bool m_handleInbox; // True if the client should handle inbox messaging

private:
#ifndef DO_WICKRBOT
    FileDownloader *m_pImgCtrl;
#endif
    bool m_waiting4image;
    bool m_imageSuccess;
    QString m_imageFileName;
    QList<QString> m_responses;

public:
    LogHandler *log_handler;
#ifndef DO_WICKRBOT
    bool downloadImage(QUrl imageURL, QString fullpath);
#endif
    bool busy();

    void addResponseMessage(QString message) {
        m_responses.append(message);
    }

    QString getReponseMessage() {
        QString response;
        if (m_responses.length() > 0) {
            response = m_responses.first();
            m_responses.removeFirst();
        }
        return response;
    }

    bool setApiKey(const QString &apiKey);
    bool validateApiKey(const QString &apiKey);

    QString getResponseURL();

    bool alreadyActive(bool closeDbOnExit=false);
    bool updateProcessState(int state, bool force=true);

private slots:
#ifndef DO_WICKRBOT
    void loadImage();
    void loadFailed();
#endif
};

#endif // OPERATIONDATA_H

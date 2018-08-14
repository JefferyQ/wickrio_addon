#ifndef WICKRIOECLIENTMAIN_H
#define WICKRIOECLIENTMAIN_H

#include <QObject>
#include <QThread>
#include <QTimer>
#include <QString>

#include "wickriodatabase.h"
#include "wickrIOJson.h"
#include "wickrbotlog.h"
#include "operationdata.h"
#include "wickrIOIPCService.h"
#include "wickrioreceivethread.h"
#include "user/wickrUser.h"
#include "services/wickrTaskService.h"

#include "wickrioconvohdlr.h"
#include "wickrIOClientLoginHdlr.h"

#define WICKRBOT WickrIOEClientMain::theBot

#define WICKRBOT_NEXTMSG_TIME           86400       // Number of seconds to wait before reponding to received message

#define WICKRBOT_CONFIG_USERNAME    "wickruser"
#define WICKRBOT_CONFIG_PASSWORD    "wickrpassword"
#define WICKRBOT_CONFIG_DORECEIVE   "doreceive"
#define WICKRBOT_CONFIG_SERVERNAME  "servername"

class WickrIOEClientMain : public QThread
{
    Q_OBJECT

    friend class WickrIOConvoHdlr;
public:
    WickrIOEClientMain(OperationData *operation);
    ~WickrIOEClientMain();

    bool startTheClient();

    static WickrIOEClientMain *theBot;

    // Parse the settings file (replaces the JSON config file)
    bool parseSettings(QSettings *settings);

    QString getPassword();

    // Function to set connection to the IPC signals
    void setIPC(WickrIOIPCService *ipc);

    WickrIOConvoHdlr m_convoHdlr;

    QString m_user;
    QString m_password;
    QString m_userName;

    bool loadBootstrapFile();
    bool loadBootstrapString(const QString& bootstrapStr);

private:
    OperationData *m_operation;
    WickrIOClientLoginHdlr m_loginHdlr;

    QTimer timer;
    int m_timerStatsTicker;
    QString m_serverName;

    WickrIOIPCService       *m_ipcSvc = nullptr;
    WickrIOReceiveThread    *m_rxThread = nullptr;
    bool                    m_waitingForPassword = false;

    // Timer definitions
    void startTimer()
    {
        connect(&timer, SIGNAL(timeout()), this, SLOT(slotDoTimerWork()));
        timer.start(1000);
    }
    void stopTimer()
    {
        disconnect(&timer, SIGNAL(timeout()), this, SLOT(slotDoTimerWork()));
        if (timer.isActive())
            timer.stop();
    }

    void stopAndExit(int procState);

    bool sendConsoleMsg(const QString& cmd, const QString& value);

private slots:
    void slotDoTimerWork();
    void slotLoginSuccess();
    void slotRxProcessStarted();
    void slotRxProcessReceiving();

    void processStarted();
    void stopAndExitSlot();
    void pauseAndExitSlot();
    void slotReceivedMessage(QString type, QString value);

    void slotSwitchboardServiceState(WickrServiceState state, SBSessionStatus sessionStatus, const QString& text);
    void slotMessageServiceState(WickrServiceState state);

    void slotTaskServiceState(WickrServiceState state);

    void slotOnLoginMsgSynchronizationComplete();

    void slotDatabaseLoadDone(WickrDatabaseLoadContext *context);

    void slotAdminUserSuspend(SBSessionError code, const QString& reason);
    void slotAdminDeviceSuspend();
    void slotSetSuspendError();
    void slotMessageDownloadStatusStart(int msgsToDownload);
    void slotMessageDownloadStatusUpdate(int msgsDownloaded);

    void slotServiceNotLoggedIn();  // Received when Switchboard cannot login for period of time

signals:
    void signalStarted();
    void signalExit();
    void signalLoginSuccess();

    void signalMessageCheck(WickrApplicationState appContext);
};


#endif // WICKRIOECLIENTMAIN_H

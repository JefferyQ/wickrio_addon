#ifndef WICKRIOCALLBACKSERVICE_H
#define WICKRIOCALLBACKSERVICE_H

#include <QObject>
#include <QThread>
#include "common/wickrNetworkUtil.h"

#include "operationdata.h"
#include "wickrioappsettings.h"
#include "SmtpMime"

// Forward declaration
class WickrIOCallbackThread;

///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////

// WickrIOCallback Thread State
enum CBThreadState { CB_UNINITIALIZED = 0, // Unitialized
                     CB_STARTED,           // Thread started state, enteres this state only on initial startup.
                     CB_PROCESSING,        // Currently processing messages
                     CB_FINISHED };        // Disconnected from switchboard

///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////

class WickrIOCallbackService : public QObject
{
    Q_OBJECT

public:
    explicit WickrIOCallbackService();
    virtual ~WickrIOCallbackService();

    void messagesPending();

private:
    // General purpose thread lock used for common threaded related queries/updates(hence ReadWrite).
    // NOTE: Other required service specific locks should be defined and managed by the specialized services.
    mutable QReadWriteLock m_lock;

    WickrServiceState       m_state;
    QThread                 m_thread;
    WickrIOCallbackThread   *m_cbThread;

    void startThreads();
    void stopThreads();

signals:
    void signalMessagesPending();

public slots:
};

///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////

/**
 * @brief The WickrIOCallbackThread class
 */
class WickrIOCallbackThread : public QObject
{
    Q_OBJECT

public:
    static QString cbStringState(CBThreadState state);

    WickrIOCallbackThread(QThread *thread, WickrIOCallbackService *swbSvc);
    virtual ~WickrIOCallbackThread();

private:
    WickrIOCallbackService  *m_parent;
    CBThreadState           m_state;
    OperationData           *m_operation;

    SmtpClient              *m_smtp;
    QNetworkReply           *reply;
    QNetworkAccessManager   *mgr;
    int                     postedMsgID;
    QString                 m_url;

    void startEmailCallback(WickrIOEmailSettings *email);
    bool sendMessages(WickrIOEmailSettings *email);
    MimeFile *getAttachmentFile(const QString &filename);

    void startUrlCallback(QString url);
    bool sendMessage();

private slots:
    void msgSendCallbackResponse(QNetworkReply *thereply, QByteArray);
    void msgSendCallbackError(QNetworkReply *thereply, QByteArray bytes);
    void gotReply(QNetworkReply *thereply);

signals:
    void sendReply(QNetworkReply *reply, QByteArray replyBytes);
    void sendError(QNetworkReply *reply, QByteArray replyBytes);

public slots:
    void slotProcessMessages();
};

#endif // WICKRIOCALLBACKSERVICE_H
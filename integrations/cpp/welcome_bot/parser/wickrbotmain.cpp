#include <QDebug>

#include "wickrbotmain.h"
#include "parseroperationdata.h"

WickrBotMain *WickrBotMain::theBot;

WickrBotMain::WickrBotMain(ParserOperationData *operation) :
    m_operation(operation),
    m_logcountdown(LOG_COUNTDOWN)
{
    this->connect(this, &WickrBotMain::started, this, &WickrBotMain::processStarted);
    connect(&timer, SIGNAL(timeout()), this, SLOT(doTimerWork()));
    timer.start(1000);

    m_ipcService = new WelcomeIpcService();

    connect(m_ipcService, &WelcomeIpcService::signalStateChange,
            this, &WickrBotMain::slotStateChange, Qt::QueuedConnection);

    m_ipcService->startListening();
}

WickrBotMain::~WickrBotMain()
{
    m_ipcService->stopThreads();
    m_ipcService->deleteLater();
    m_ipcService = nullptr;

    qDebug() << "Parser deletion";
    timer.stop();
    timer.deleteLater();
}

bool WickrBotMain::startTheClient(){
    qDebug() << "Entering startTheClient with parser";

    emit signalStarted();
    return true;
}

void WickrBotMain::processStarted()
{
    qDebug() << "Started WickrBotParser main";

    if (! startTheClient())
        stopAndExit(PROCSTATE_DOWN);
}

void WickrBotMain::slotStateChange(bool shutdown)
{
    stopAndExit(PROCSTATE_DOWN);
}

void WickrBotMain::doTimerWork()
{
    m_seccount++;

    if (--m_logcountdown <= 0) {
        qDebug() << "Keep alive message";
        //m_operation->updateProcessState(PROCSTATE_RUNNING);
        m_logcountdown = LOG_COUNTDOWN;
    }

    if (m_qamqp == nullptr) {
        m_qamqp = new WBParse_QAMQPQueue(m_operation);
    } else {
        // If the Queue handler fails then delete the queue handler.
        // It will be recreated on the next iteration.

        if (! m_qamqp->timerCall()) {
            qDebug() << "Message queue handler failed.  Deleting handler.";
            delete  m_qamqp;
            m_qamqp = nullptr;

            m_qfailures++;
        } else if (! m_qamqp->isRunning()) {
            m_qfailures++;
        } else {
            m_qfailures = 0;
        }
    }

    if (m_rxProcess == nullptr) {
        m_rxProcess = new WelcomeRxProcessing(m_operation);
    } else {
        if (! m_rxProcess->timerCall()) {
            qDebug() << "Receive message handler failed.";
            m_qfailures++;
        } else {
            m_qfailures = 0;
        }
    }
}

/**
 * @brief WickrBotMain::pauseAndExitSlot
 * Call this slot to put the state of the parser in the database to the DOWN state,
 * and exit the parser application.
 */
void WickrBotMain::stopAndExitSlot()
{
    stopAndExit(PROCSTATE_DOWN);
}

/**
 * @brief WickrBotMain::pauseAndExitSlot
 * Call this slot to put the state of the parser in the database to the PAUSED state,
 * and exit the parser application.
 */
void WickrBotMain::pauseAndExitSlot()
{
    stopAndExit(PROCSTATE_PAUSED);
}

/**
 * @brief WickrBotMain::stopAndExit
 * This is called to exit the application. The application state is set to the input
 * state, in the process_state table in the database.
 */
void WickrBotMain::stopAndExit(int procState)
{
    QCoreApplication::quit();
}

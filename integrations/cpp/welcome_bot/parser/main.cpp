#include <QDebug>
#include <QJsonDocument>
#include <QJsonValue>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QByteArray>
#include <QDateTime>
#include <QFileInfo>
#include <QThread>
#include <QSettings>
#include <QStandardPaths>
#include <QDir>
#include <QProcess>
#include <QtPlugin>

#include "qtSingleApplication.h"

#include "wickrbotutils.h"
#include "parseroperationdata.h"
#include "wbparse_qamqpqueue.h"
#include "wickrbotmain.h"
#include "wickrbotsettings.h"
#include "bot_iface.h"
#include "welcomeClientConfigInfo.h"

WickrBotMain        *m_wbmain;
ParserOperationData *operation = nullptr;

bool makeDirectory(QString dirname)
{
    QDir tmp(dirname);
    if (!tmp.exists()) {
        if (!tmp.mkpath(".")) {
            qDebug() << "Cannot make directory" << dirname;
            return false;
        }
    }
    return true;
}


static void
usage()
{
    qDebug() << "Usage:";
    qDebug() << "parse_command [-d] [-force] [-dbdir=<db directory>] [-attachdir=<attachment directory>] [-log=<logs directory>]" <<
                "[-qconfig=<config file>] [-qhost=<host:port>|<host>] [-quser=<queue username>] [-qpassword=<queue password>]" <<
                " [-duration=<secs to run>] [-appName=<name of parser>]";
    qDebug() << "The parse_command program will parse the input JSON file and send output to the designed location.";
    exit(0);
}

/** Search the configuration file */
QString
searchConfigFile() {
    QStringList searchList;
    searchList.append(QDir::currentPath());

    // Look for the ini file with the application name
    QString appName=QCoreApplication::applicationName();
    QString fileName(appName+".ini");

    QString retFile = WickrBotUtils::fileInList(fileName, searchList);

    if (retFile.isEmpty()) {
        retFile = WickrBotUtils::fileInList(WBIO_WELCOME_INI, searchList);
        if (retFile.isEmpty()) {
            qFatal("Cannot find config file %s",qPrintable(fileName));
        }
    }
    return retFile;
}

/**
 * @brief parseConfigFile
 * This function will open the input config file and parse out the Rabbit MQ login
 * credentials. The contents of the config file should contain JSON format with the
 * "qhost", "quser" and "qpassword" string values.
 * @return True is returned if successful, false if there was a problem parsing the file
 */
bool parseConfigFile(QString qConfigFile, ParserOperationData *operation)
{
    QSettings settings(qConfigFile, QSettings::NativeFormat);

    // Parse the Rabbit Queue settings
    settings.beginGroup(WBSETTINGS_RBTQ_HEADER);
    operation->queueHost        = settings.value(WBSETTINGS_RBTQ_HOST, "").toString();
    operation->queuePort        = settings.value(WBSETTINGS_RBTQ_PORT, "").toInt();
    operation->queueUsername    = settings.value(WBSETTINGS_RBTQ_USER, "").toString();
    operation->queuePassword    = settings.value(WBSETTINGS_RBTQ_PASSWORD, "").toString();
    operation->queueExchange    = settings.value(WBSETTINGS_RBTQ_EXCHANGE, "").toString();
    operation->queueName        = settings.value(WBSETTINGS_RBTQ_QUEUE, "").toString();
    operation->queueVirtualHost = settings.value(WBSETTINGS_RBTQ_VIRTUALHOST, "").toString();
    settings.endGroup();

    if (operation->queueHost.isEmpty() ||
        operation->queueUsername.isEmpty() ||
        operation->queuePassword.isEmpty() ||
        operation->queuePort == 0) {
        qDebug() << "Rabbit Queue is not set";
        return false;
    }

    /*
     * Parse the general settings: client name, and messages
     */
    settings.beginGroup(WBSETTINGS_GEN_HEADER);
    operation->m_botName = settings.value(WBSETTINGS_GEN_CLIENT, "").toString();
    operation->m_welcomeUserMessage = settings.value(WBSETTINGS_GEN_WELCOMEUSER_MSG, "").toString();
    operation->m_welcomeAdminMessage = settings.value(WBSETTINGS_GEN_WELCOMEADMIN_MSG, "").toString();
    operation->m_newDeviceMessage = settings.value(WBSETTINGS_GEN_NEWDEVICE_MSG, "").toString();
    operation->m_forgotPwMessage = settings.value(WBSETTINGS_GEN_FORGOTPW_MSG, "").toString();
    settings.endGroup();

    if (operation->m_botName.isEmpty())
        return false;
    return true;
}

void redirectedOutput(QtMsgType type, const QMessageLogContext &, const QString & str)
{
    //in this function, you can write the message to any stream!
    switch (type) {
    case QtDebugMsg:
    case QtWarningMsg:
    case QtCriticalMsg:
        if (operation != nullptr && operation->log_handler != nullptr)
            operation->log_handler->output(str);
        break;
    case QtFatalMsg:
        if (operation != nullptr && operation->log_handler != nullptr)
            operation->log_handler->output(str);
//        abort();
    }
}

int main(int argc, char *argv[])
{

#ifdef Q_OS_LINUX
#ifdef WICKR_BETA
    QCoreApplication::addLibraryPath("/usr/lib/wio_welcome_bot-beta");
    QCoreApplication::addLibraryPath("/usr/lib/wio_welcome_bot-beta/plugins");
#elif defined(WICKR_ALPHA)
    QCoreApplication::addLibraryPath("/usr/lib/wio_welcome_bot-alpha");
    QCoreApplication::addLibraryPath("/usr/lib/wio_welcome_bot-alpha/plugins");
#else
    QCoreApplication::addLibraryPath("/usr/lib/wio_welcome_bot");
    QCoreApplication::addLibraryPath("/usr/lib/wio_welcome_bot/plugins");
#endif
#endif


    operation = new ParserOperationData();
    operation->log_handler->setupLog(QString("%1/welcome_parser.log").arg(QDir::currentPath()));
    operation->log_handler->logSetOutput(QString("%1/welcome_parser.output").arg(QDir::currentPath()));

    qInstallMessageHandler(redirectedOutput);

    QString wbConfigFile("");

    for( int argidx = 1; argidx < argc; argidx++ ) {
        QString cmd(argv[argidx]);

        if ( cmd == "-?" || cmd == "-help" || cmd == "--help" ) {
            usage();
        } else if (cmd == "-d") {
            operation->debug = true;
        } else if (cmd == "-force") {
            operation->force = true;
        } else if (cmd.startsWith("-qhost=")) {
            QString temp = cmd.remove("-qhost=");
            QStringList split = temp.split(':');
            if (split.size() == 2) {
                operation->queuePort = split.at(1).toInt();
                operation->queueHost = split.at(0);
            } else if (split.size() == 1) {
                operation->queueHost = temp;
            } else {
                qDebug() << "Invalid format for the -qhost argument";
                return 1;
            }
        } else if (cmd.startsWith("-quser=")) {
            operation->queueUsername = cmd.remove("-quser=");
        } else if (cmd.startsWith("-qpass=")) {
            operation->queuePassword = cmd.remove("-qpass=");
        } else if (cmd.startsWith("-config=")) {
            wbConfigFile = cmd.remove("-config=");
        }
    }

    // If the config file was not specified on command line then try to find it
    if (wbConfigFile.isEmpty()) {
        wbConfigFile = searchConfigFile();
        if (wbConfigFile.isEmpty()) {
            qDebug() << "Cannot determine settings file!";
            exit(1);
        }
    }

    // Parse the configuration file contents
    if (!parseConfigFile(wbConfigFile, operation)) {
        qDebug() << "Failed to configure properly!";
        exit(1);
    }

    QString appName;

#if 0
    QCoreApplication a(argc, argv);
#else
    appName = QString("%1_%2").arg(WELCOMEBOT_PARSER_PROCESS).arg(operation->m_botName);
    QtSingleApplication a(appName, argc, argv);
#endif
    if (!a.isPrimaryInstance()) {
        qDebug() << "PRIMARY APPLICATION INSTANCE ALREADY RUNNING!";
        a.sendMessage(QStringLiteral("Starting..."));
        exit(1);
    }

    a.setOrganizationName("Wickr, LLC");
    a.setApplicationName(appName);

    // Setup the bot interface
    operation->m_botIface = new BotIface(operation->m_botName.toStdString());
    if (operation->m_botIface->init() != BotIface::SUCCESS) {
        qDebug() << "Could not initialize the Bot interface!";
        exit(1);
    }

    WICKRBOT = new WickrBotMain(operation);

    QObject::connect(WICKRBOT, &WickrBotMain::signalStarted, [=]() {
        qDebug() << "Main process starting!";
    });

    WICKRBOT->start();
    int retval = a.exec();

    //clean up resources
    WICKRBOT->deleteLater();

    return retval;
}

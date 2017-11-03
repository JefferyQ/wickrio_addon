#ifndef WICKRIOCOMMON_H
#define WICKRIOCOMMON_H

#include <QString>
#include <QSettings>

/*
 * Definitions of common values used throughout the different
 * WickrIO applications.
 */

#define WBIO_ORGANIZATION       "Wickr, LLC"

// Definitions of server targets
#if defined(WICKR_DEBUG)
#define WBIO_GENERAL_TARGET         "WickrIODebug"
#define WBIO_CLIENTSERVER_TARGET    "WickrIOSvrDebug"
#define WBIO_CONSOLE_TARGET         "WickrIOConsoleDebug"
#define WBIO_CONSOLESERVER_TARGET   "WickrIOCSvrDebug"
#else
#define WBIO_GENERAL_TARGET         "WickrIO"
#define WBIO_CLIENTSERVER_TARGET    "WickrIOSvr"
#define WBIO_CONSOLE_TARGET         "WickrIOConsole"
#define WBIO_CONSOLESERVER_TARGET   "WickrIOCSvr"
#endif


#ifdef Q_OS_WIN
#define WBIO_SERVER_SETTINGS_FORMAT     "HKEY_LOCAL_MACHINE\\SOFTWARE\\%1\\%2"
#define WBIO_CLIENT_SETTINGS_FORMAT     "HKEY_LOCAL_MACHINE\\SOFTWARE\\%1\\%2\\clients\\%3"
#else
#define WBIO_CLIENT_SETTINGS_FORMAT     "%1/clients/%2/WickrIOClient.%2.ini"
#endif
#define WBIO_CLIENT_DBDIR_FORMAT        "%1/clients/%2/client"
#define WBIO_CLIENT_LOGFILE_FORMAT      "%1/clients/%2/logs/WickrIO%2.log"
#define WBIO_CLIENT_OUTFILE_FORMAT      "%1/clients/%2/logs/WickrIO%2.output"
#define WBIO_CLIENT_WORKINGDIR_FORMAT   "%1/clients/%2"
#define WBIO_CLIENT_ATTACHDIR_FORMAT    "%1/clients/%2/attachments"

#define WBIO_PARSER_SETTINGS_FORMAT     "%1/%2.ini"

#ifdef Q_OS_WIN
#define WBIO_DEFAULT_DBLOCATION         TBD
        dbLocation = QString("%1/%2/%3")
                .arg(QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation))
                .arg(WBIO_ORGANIZATION)
                .arg(WBIO_GENERAL_TARGET);
#else
#if defined(WICKR_DEBUG)
#define WBIO_DEFAULT_DBLOCATION         "/opt/WickrIODebug"
#else
#define WBIO_DEFAULT_DBLOCATION         "/opt/WickrIO"
#endif
#endif


/*
 * WickrIO IPC commands
 */
#define WBIO_IPCCMDS_PAUSE          "pause"
#define WBIO_IPCCMDS_STOP           "stop"

#define WBIO_IPCMSGS_PASSWORD       "password"
#define WBIO_IPCMSGS_USERSIGNKEY    "usersignkey"
#define WBIO_IPCMSGS_STATE          "state"
#define WBIO_IPCMSGS_BOTINFO        "botinfo"

#define WBIO_IPCHDR_PROCESSNAME     "hdr_process"   // Name of the sending process

#define WBIO_BOTINFO_CLIENT         "client"
#define WBIO_BOTINFO_PROCESSNAME    "process"
#define WBIO_BOTINFO_PASSWORD       "password"

class WBIOCommon
{
public:
    WBIOCommon() {}

    static bool makeDirectory(QString dirname);
};


#endif // WICKRIOCOMMON_H
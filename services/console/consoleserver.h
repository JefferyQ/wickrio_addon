#ifndef CONSOLESERVER_H
#define CONSOLESERVER_H

#include <QObject>
#include "wickriodatabase.h"
#include "wbio_common.h"
#include "server_common.h"
#include "wickrioappsettings.h"

class ConsoleServer
{
public:
    ConsoleServer(WickrIOClientDatabase *ioDB);

    bool isRunning(const QString &processName = WBIO_CONSOLESERVER_TARGET, int timeout=60);
    void toggleState(const QString &processName = WBIO_CONSOLESERVER_TARGET);
    bool restart();
    bool isConfigured();
    bool setSSL(WickrIOSSLSettings *ssl);

private:
    WickrIOClientDatabase *m_ioDB;
    QSettings *m_settings;
};

#endif // CONSOLESERVER_H
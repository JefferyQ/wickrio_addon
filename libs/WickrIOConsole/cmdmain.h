#ifndef CMDMAIN_H
#define CMDMAIN_H

#include <QObject>
#include <QCoreApplication>
#include <QString>
#include <QSettings>

#include "wickriodatabase.h"
#include "wickrIOAppSettings.h"
#include "cmdbase.h"
#include "cmdoperation.h"
#include "cmdclient.h"
#include "cmdadvanced.h"
#include "cmdconsole.h"
#include "cmdserver.h"
#include "cmdparser.h"
#include "cmdusers.h"

class CmdMain : public CmdBase
{
    Q_OBJECT
public:
    explicit CmdMain(OperationData*pOperation);
    explicit CmdMain(const QString& appName);

    bool runCommands(QString commands=QString());

private:
    CmdOperation m_cmdOperation;
    CmdClient    m_cmdClient;
    CmdConsole   m_cmdConsole;
    CmdAdvanced  m_cmdAdvanced;
    CmdServer    m_cmdServer;
    CmdParser    m_cmdParser;
    CmdUsers     m_cmdUsers;

    CmdOperation *m_operation = nullptr;
    bool         m_hasMotherBotBinary = false;
    bool         m_hasParserBinary = false;

    bool processCommand(QString cmd, QString subcmds);
    bool config(QString args);
};

#endif // CMDMAIN_H

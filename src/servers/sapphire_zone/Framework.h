#ifndef CORE_FRAMEWORK_H
#define CORE_FRAMEWORK_H

#include <boost/shared_ptr.hpp>
#include <map>

#include "Forwards.h"


#include "ServerZone.h"

#include <common/Logging/Logger.h>

#include <common/Exd/ExdDataGenerated.h>

#include "Script/ScriptMgr.h"

#include "Linkshell/LinkshellMgr.h"

#include "Zone/TerritoryMgr.h"

#include "DebugCommand/DebugCommandHandler.h"

#include <common/Database/CharaDbConnection.h>
#include <common/Database/DbWorkerPool.h>

namespace Core
{

class Framework
{
private:

   Logger g_log;
   DebugCommandHandler g_debugCmdHandler;
   Scripting::ScriptMgr g_scriptMgr;
   Data::ExdDataGenerated g_exdDataGen;
   TerritoryMgr g_territoryMgr;
   LinkshellMgr g_linkshellMgr;
   Db::DbWorkerPool< Db::CharaDbConnection > g_charaDb;

public:

   Logger& getLogger();
   DebugCommandHandler& getDebugCommandHandler();
   Scripting::ScriptMgr& getScriptMgr();
   Data::ExdDataGenerated& getExdDataGen();
   TerritoryMgr& getTerritoryMgr();
   LinkshellMgr& getLinkshellMgr();
   Db::DbWorkerPool< Db::CharaDbConnection >& getCharaDb();
   ServerZone& getServerZone();

};

}
#endif // CORE_FRAMEWORK_H

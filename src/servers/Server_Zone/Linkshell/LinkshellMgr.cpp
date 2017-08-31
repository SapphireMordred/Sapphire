#include "LinkshellMgr.h"
#include <Server_Common/Database/Database.h>
#include <Server_Common/Logging/Logger.h>
#include <boost/make_shared.hpp>

#include "Linkshell.h"

extern Core::Logger g_log;
extern Core::Db::Database g_database;

Core::LinkshellMgr::LinkshellMgr()
{

}

bool Core::LinkshellMgr::loadLinkshells()
{

   auto res = g_database.query( "SELECT LinkshellId, MasterCharacterId, CharacterIdList, "
                                        "LinkshellName, LeaderIdList, InviteIdList "
                                        "FROM infolinkshell "
                                "ORDER BY LinkshellId ASC;" );

   // we do not really need linkshells to function...
   if( !res )
      return true;

   Db::Field *field = res->fetch();

   do
   {
      uint32_t linkshellId = field[0].get< uint32_t >();
      uint32_t masterId = field[1].get< uint32_t >();
      std::string name = field[3].getString();

      auto func = []( std::set< uint64_t >& outList, Db::Field * pField )
      {
         if( pField->getLength() )
         {
            std::vector< uint64_t > list( pField->getLength() / 8 );
            pField->getBinary( reinterpret_cast< char * >( &list[0] ), pField->getLength() );
            outList.insert( list.begin(), list.end() );
         }
      };

      std::set< uint64_t > members;
      func( members, &field[2] );

      std::set< uint64_t > leaders;
      func( members, &field[4] );

      std::set< uint64_t > invites;
      func( members, &field[5] );

      auto lsPtr = boost::make_shared< Linkshell >( linkshellId, name, masterId, members, leaders, invites );
      m_linkshellIdMap[linkshellId] = lsPtr;
      m_linkshellNameMap[name] = lsPtr;

   } while( res->nextRow() );

   return true;

}

Core::LinkshellPtr Core::LinkshellMgr::getLinkshellByName( const std::string& name )
{
   auto it = m_linkshellNameMap.find( name );
   if( it == m_linkshellNameMap.end() )
      return nullptr;
   else
      return it->second;
}

Core::LinkshellPtr Core::LinkshellMgr::getLinkshellById( uint64_t lsId )
{
   auto it = m_linkshellIdMap.find( lsId );
   if( it == m_linkshellIdMap.end() )
      return nullptr;
   else
      return it->second;
}

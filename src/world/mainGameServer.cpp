#include <iostream>

#include "ServerMgr.h"

#include <Util/CrashHandler.h>
#include <Service.h>

using namespace Sapphire;
using namespace Sapphire::World;

[[maybe_unused]]
Common::Util::CrashHandler crashHandler;

int main( int32_t argc, char* argv[] )
{
  auto pServer = std::make_shared< ServerMgr >( "world.ini" );

  Common::ServiceContainer::pSvcContainer = new Common::ServiceContainer();
  Common::Service< ServerMgr >::set( pServer );

  pServer->run( argc, argv );
  return 0;
}

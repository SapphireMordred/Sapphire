#include <Service.h>

namespace Sapphire::Common
{
  ServiceContainer* ServiceContainer::pSvcContainer = nullptr;

  std::shared_ptr< void > ServiceContainer::get( size_t id )
  {
    return serviceTable[ id ];
  }
  
  void ServiceContainer::set( size_t id, std::shared_ptr< void > svc )
  {
    serviceTable[ id ] = std::move( svc );
  }
}
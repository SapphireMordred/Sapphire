#include <Util/Util.h>
#include <Logging/Logger.h>
#include <Exd/ExdDataGenerated.h>
#include <Network/CommonActorControl.h>
#include <Service.h>

#include "Network/PacketWrappers/ActorControlPacket.h"
#include "Network/PacketWrappers/ActorControlSelfPacket.h"

#include "Actor/Player.h"

#include "EventItemAction.h"

using namespace Sapphire;
using namespace Sapphire::World;
using namespace Sapphire::Common;
using namespace Sapphire::Network;
using namespace Sapphire::Network::Packets;
using namespace Sapphire::Network::Packets::Server;
using namespace Sapphire::Network::ActorControl;

Action::EventItemAction::EventItemAction( Entity::CharaPtr pActor, uint32_t eventId, uint32_t eventItemId,
                                          EventItemActionCallback finishRef, ActionCallback interruptRef, uint64_t targetId )
{
  m_eventId = eventId;
  m_id = eventItemId;
  m_targetId = targetId;
  auto& exdData = Common::Service< Data::ExdDataGenerated >::ref();
  m_castTimeMs = exdData.get< Sapphire::Data::EventItem >( eventItemId )->castTime * 1000; // TODO: Add security checks.
  m_onActionFinishClb = std::move( finishRef );
  m_onActionInterruptClb = std::move( interruptRef );
  m_pSource = std::move( pActor );
  m_interruptType = Common::ActionInterruptType::None;

  // SPH: Debug
  Logger::warn("EventItemAction constructor: eventId={0}, eventItemId={1}, targetId={2}", eventId, eventItemId, targetId);
}

Action::EventItemAction::~EventItemAction() = default;

void Action::EventItemAction::start()
{
  if( !m_pSource )
    return;
  // SPH: Debug
  Logger::warn("EventItemAction::start()");
  // SPH: Debug

  m_startTime = Common::Util::getTimeMs();

  auto control = makeActorControl( m_pSource->getId(), ActorControlType::CastStart, 1, m_id, 0x4000004E );   // SPH: Does CastStart here have to change?

  if( m_pSource->isPlayer() )
  {
    m_pSource->sendToInRangeSet( control, true );
    if( m_pSource->getAsPlayer()->hasStateFlag( PlayerStateFlag::InNpcEvent ) )
      m_pSource->getAsPlayer()->unsetStateFlag( PlayerStateFlag::InNpcEvent );
  }
  else
    m_pSource->sendToInRangeSet( control );
}

void Action::EventItemAction::execute()
{
  if( !m_pSource )
    return;
  // SPH: Debug
  Logger::warn("EventItemAction::execute()");
  // SPH: Debug

  try
  {
/*
    auto pEvent = m_pSource->getAsPlayer()->getEvent( m_eventId );

    pEvent->setPlayedScene( false );
*/
    if( m_onActionFinishClb )
      m_onActionFinishClb( *m_pSource->getAsPlayer(), m_eventId, m_id, m_targetId );

    auto control = makeActorControl( m_pSource->getId(), ActorControlType::CastStart, 0, m_id );

/*
    if( !pEvent->hasPlayedScene() )
      m_pSource->getAsPlayer()->eventFinish( m_eventId, 1 );
    else
      pEvent->setPlayedScene( false );
*/

    if( m_pSource->isPlayer() )
    {
      //m_pSource->getAsPlayer()->unsetStateFlag( PlayerStateFlag::Occupied2 );
      m_pSource->sendToInRangeSet( control, true );
    }
    else
      m_pSource->sendToInRangeSet( control );
  }
  catch( std::exception& e )
  {
    Logger::error( e.what() );
  }

}

void Action::EventItemAction::interrupt()
{
  if( !m_pSource )
    return;
  // SPH: Debug
  Logger::warn("EventItemAction::interrupt()");
  // SPH: Debug

  try
  {

    auto control = makeActorControl( m_pSource->getId(), ActorControlType::CastInterrupt, 0x219, 0x04, m_id );

    if( m_pSource->isPlayer() )
    {
      auto control1 = makeActorControlSelf( m_pSource->getId(), ActorControlType::FreeEventPos, m_eventId );

      //m_pSource->getAsPlayer()->unsetStateFlag( PlayerStateFlag::NoCombat );
      //m_pSource->getAsPlayer()->unsetStateFlag( PlayerStateFlag::Occupied1 );
      m_pSource->sendToInRangeSet( control );
      m_pSource->sendToInRangeSet( control1 );

      m_pSource->getAsPlayer()->queuePacket( control1 );
      m_pSource->getAsPlayer()->queuePacket( control );
      m_pSource->getAsPlayer()->eventFinish( m_eventId, 1 );

    }
    else
      m_pSource->sendToInRangeSet( control );

    if( m_onActionInterruptClb )
      m_onActionInterruptClb( *m_pSource->getAsPlayer(), m_eventId, 0 );

  }
  catch( std::exception& e )
  {
    Logger::error( e.what() );
  }

}

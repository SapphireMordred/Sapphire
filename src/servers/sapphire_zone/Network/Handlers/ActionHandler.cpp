#include <boost/format.hpp>

#include <common/Common.h>
#include <common/Network/CommonNetwork.h>
#include <common/Network/GamePacketNew.h>
#include <common/Logging/Logger.h>
#include <common/Exd/ExdDataGenerated.h>
#include <common/Network/PacketContainer.h>

#include "Zone/Zone.h"
#include "Zone/ZonePosition.h"

#include "Network/GameConnection.h"
#include "Network/PacketWrappers/InitUIPacket.h"
#include "Network/PacketWrappers/PingPacket.h"
#include "Network/PacketWrappers/MoveActorPacket.h"
#include "Network/PacketWrappers/ChatPacket.h"
#include "Network/PacketWrappers/ServerNoticePacket.h"
#include "Network/PacketWrappers/ActorControlPacket142.h"
#include "Network/PacketWrappers/ActorControlPacket143.h"
#include "Network/PacketWrappers/ActorControlPacket144.h"
#include "Network/PacketWrappers/EventStartPacket.h"
#include "Network/PacketWrappers/EventFinishPacket.h"
#include "Network/PacketWrappers/PlayerStateFlagsPacket.h"

#include "DebugCommand/DebugCommandHandler.h"

#include "Actor/Player.h"

#include "Inventory/Inventory.h"

#include "Event/EventHelper.h"

#include "Action/Action.h"
#include "Action/ActionTeleport.h"

#include "Session.h"
#include "ServerZone.h"
#include "Forwards.h"
#include "Framework.h"

extern Core::Framework g_framework;

using namespace Core::Common;
using namespace Core::Network::Packets;
using namespace Core::Network::Packets::Server;

void Core::Network::GameConnection::actionHandler( const Packets::GamePacket& inPacket,
                                                   Entity::Player& player )
{
    uint16_t commandId = inPacket.getValAt< uint16_t >( 0x20 );
    uint64_t param1 = inPacket.getValAt< uint64_t >( 0x24 );
    uint32_t param11 = inPacket.getValAt< uint32_t >( 0x24 );
    uint32_t param12 = inPacket.getValAt< uint32_t >( 0x28 );
    uint32_t param2 = inPacket.getValAt< uint32_t >( 0x2C );
    uint64_t param3 = inPacket.getValAt< uint64_t >( 0x38 );

    g_framework.getLogger().debug( "[" + std::to_string( m_pSession->getId() ) + "] Incoming action: " +
                 boost::str( boost::format( "%|04X|" ) % ( uint32_t ) ( commandId & 0xFFFF ) ) +
                 "\nparam1: " + boost::str( boost::format( "%|016X|" ) % ( uint64_t ) ( param1 & 0xFFFFFFFFFFFFFFF ) ) +
                 "\nparam2: " + boost::str( boost::format( "%|08X|" ) % ( uint32_t ) ( param2 & 0xFFFFFFFF ) ) +
                 "\nparam3: " + boost::str( boost::format( "%|016X|" ) % ( uint64_t ) ( param3 & 0xFFFFFFFFFFFFFFF ) )
    );


    //g_log.Log(LoggingSeverity::debug, "[" + std::to_string(m_pSession->getId()) + "] " + pInPacket->toString());

    switch( commandId )
    {
        case 0x01:  // Toggle sheathe
        {
            if ( param11 == 1 )
                player.setStance( Entity::Actor::Stance::Active );
            else
            {
                player.setStance( Entity::Actor::Stance::Passive );
                player.setAutoattack( false );
            }

            player.sendToInRangeSet( ActorControlPacket142( player.getId(), 0, param11, 1 ) );

            break;
        }
        case 0x02:  // Toggle auto-attack
        {
            if ( param11 == 1 )
            {
                player.setAutoattack( true );
                player.setStance( Entity::Actor::Stance::Active );
            }
            else
                player.setAutoattack( false );

            player.sendToInRangeSet( ActorControlPacket142( player.getId(), 1, param11, 1 ) );

            break;
        }
        case 0x03: // Change target
        {

            uint64_t targetId = inPacket.getValAt< uint64_t >( 0x24 );
            player.changeTarget( targetId );
            break;
        }
        case 0x65:
        {
           player.dismount();
           break;
        }
        case 0x68: // Remove status (clicking it off)
        {
           // todo: check if status can be removed by client from exd
           player.removeSingleStatusEffectById( static_cast< uint32_t >( param1 ) );
           break;
        }
        case 0x69: // Cancel cast
        {
           if( player.getCurrentAction() )
               player.getCurrentAction()->setInterrupted();
           break;
        }
        case 0x12E: // Set player title
        {
           player.setTitle( static_cast< uint16_t >( param1 ) );
           break;
        }
        case 0x12F: // Get title list
        {
           ZoneChannelPacket< FFXIVIpcPlayerTitleList > titleListPacket( player.getId() );
           memcpy( titleListPacket.data().titleList, player.getTitleList(), sizeof( titleListPacket.data().titleList ) );

           player.queuePacket( titleListPacket );
           break;
        }
        case 0x133: // Update howtos seen
        {
            uint32_t howToId = static_cast< uint32_t >( param1 );
            player.updateHowtosSeen( howToId );
            break;
        }
        case 0x1F4: // emote
        {
            uint64_t targetId = player.getTargetId();
            uint32_t emoteId = inPacket.getValAt< uint32_t >( 0x24 );

            player.sendToInRangeSet( ActorControlPacket144( player.getId(), Emote, emoteId, 0, 0, 0, targetId ) );
            break;
        }
        case 0xC8: // return dead
        {
           switch ( static_cast < ResurrectType >( param1 ) )
           {
              case ResurrectType::RaiseSpell:
                 // todo: handle raise case (set position to raiser, apply weakness status, set hp/mp/tp as well as packet)
                 player.returnToHomepoint();
                 break;
              case ResurrectType::Return:
                 player.returnToHomepoint();
                 break;
              default:
                 break;
           }
            
        }
        case 0xC9: // Finish zoning
        {
            switch( player.getZoningType() )
            {
                case ZoneingType::None:
                    player.sendToInRangeSet( ActorControlPacket143( player.getId(), ZoneIn, 0x01 ), true );
                    break;
                case ZoneingType::Teleport:
                    player.sendToInRangeSet( ActorControlPacket143( player.getId(), ZoneIn, 0x01, 0, 0, 110 ), true );
                    break;
                case ZoneingType::Return:
                case ZoneingType::ReturnDead:
                {
                    if( player.getStatus() == Entity::Actor::ActorStatus::Dead )
                    {
                        player.resetHp();
                        player.resetMp();
                        player.setStatus( Entity::Actor::ActorStatus::Idle );

                        player.sendToInRangeSet( ActorControlPacket143( player.getId(), ZoneIn, 0x01, 0x01, 0, 111 ), true );
                        player.sendToInRangeSet( ActorControlPacket142( player.getId(), SetStatus, static_cast< uint8_t >( Entity::Actor::ActorStatus::Idle ) ), true );
                    }
                    else
                        player.sendToInRangeSet( ActorControlPacket143( player.getId(), ZoneIn, 0x01, 0x00, 0, 111 ), true );
                }
                    break;
                case ZoneingType::FadeIn:
                    break;
            }

            player.setZoningType( Common::ZoneingType::None );

            player.unsetStateFlag( PlayerStateFlag::BetweenAreas );
            player.unsetStateFlag( PlayerStateFlag::BetweenAreas1 );
            break;
        }

        case 0xCA: // Teleport
        {
            // TODO: only register this action if enough gil is in possession
            auto targetAetheryte = g_framework.getExdDataGen().get< Core::Data::Aetheryte >( param11 );

            if( targetAetheryte )
            {
               auto fromAetheryte = g_framework.getExdDataGen().get< Core::Data::Aetheryte >( g_framework.getExdDataGen().get< Core::Data::TerritoryType >( player.getZoneId() )->aetheryte );

                // calculate cost - does not apply for favorite points or homepoints neither checks for aether tickets
                auto cost = static_cast< uint16_t >( ( sqrt( pow( fromAetheryte->aetherstreamX - targetAetheryte->aetherstreamX, 2 ) +
                                    pow( fromAetheryte->aetherstreamY - targetAetheryte->aetherstreamY, 2 ) ) / 2 ) + 100 );

                // cap at 999 gil
                cost = cost > uint16_t{999} ? uint16_t{999} : cost;

                bool insufficientGil = player.getCurrency( Inventory::CurrencyType::Gil ) < cost;
                // todo: figure out what param1 really does
                player.queuePacket( ActorControlPacket143( player.getId(), TeleportStart, insufficientGil ? 2 : 0, param11 ) );

                if( !insufficientGil )
                {
                    auto pActionTeleport = Action::make_ActionTeleport( player.getAsPlayer(), param11, cost );
                    player.setCurrentAction( pActionTeleport );
                }
            }
            break;
        }
        case 0x1B5: // Dye item
        {
           break;
        }
        case 0x321: // Director init finish
        {
           player.getCurrentZone()->onInitDirector( player );
           break;
        }
        default:
        {
           g_framework.getLogger().debug( "[" + std::to_string( m_pSession->getId() ) + "] Unhandled action: " +
              boost::str( boost::format( "%|04X|" ) % (uint32_t) ( commandId & 0xFFFF ) ) );
           break;
        }
    }
}

#include <Common.h>
#include <Exd/ExdDataGenerated.h>
#include <Network/GamePacket.h>
#include <Network/PacketDef/Zone/ClientZoneDef.h>
#include <Logging/Logger.h>

#include <Actor/Player.h>
#include <Service.h>

#include "Network/GameConnection.h"

#include "Manager/ActionMgr.h"
#include "Script/ScriptMgr.h"

using namespace Sapphire::Common;
using namespace Sapphire::Network::Packets;

void Sapphire::Network::GameConnection::actionHandler( const Packets::FFXIVARR_PACKET_RAW& inPacket,
                                                       Entity::Player& player )
{
  const auto packet = ZoneChannelPacket< Client::FFXIVIpcSkillHandler >( inPacket );

  const auto type = packet.data().type;
  const auto actionId = packet.data().actionId;
  const auto sequence = packet.data().sequence;
  const auto targetId = packet.data().targetId;
  const auto itemSourceSlot = packet.data().itemSourceSlot;
  const auto itemSourceContainer = packet.data().itemSourceContainer;

  player.sendDebug( "Skill type: {0}, sequence: {1}, actionId: {2}, targetId: {3}", type, sequence, actionId, targetId );

  auto& exdData = Common::Service< Data::ExdDataGenerated >::ref();
  auto& actionMgr = Common::Service< World::Manager::ActionMgr >::ref();

  switch( type )
  {
    default:
    {
      player.sendDebug( "Skill type {0} not supported. Defaulting to normal action", type );
    }
    case Common::SkillType::Normal:
    {
      auto action = exdData.get< Data::Action >( actionId );

      // ignore invalid actions
      if( !action )
        return;

      actionMgr.handleTargetedPlayerAction( player, actionId, action, targetId, sequence );
      break;
    }

    case Common::SkillType::ItemAction:
    {
      auto item = exdData.get< Data::Item >( actionId );
      if( !item )
        return;

      if( item->itemAction == 0 )
        return;

      auto itemAction = exdData.get< Data::ItemAction >( item->itemAction );
      if( !itemAction )
        return;

      actionMgr.handleItemAction( player, actionId, itemAction, itemSourceSlot, itemSourceContainer );

      break;
    }

    case Common::SkillType::EventItem:
    {
      auto eitem = exdData.get< Data::EventItem >( actionId );
      if ( !eitem )
        return;

      if ( itemSourceSlot != 0 || itemSourceContainer != 0 ) {
        // possible unimplemented functionality but continue anyway
        player.sendDebug( "itemSourceSlot = {0}, itemSourceSlot = {1}: Unexpected deviance from 0, possible unimplemented functionality", itemSourceSlot, itemSourceContainer );
      }

      if ( eitem->action == 0 )
        return;

      if ( eitem->action != 1 )
      {
        player.sendDebug( "EventItem action {0} is not handled", eitem->action );
        return;
      }

      auto eventId = eitem->quest;

      player.sendDebug( "Using EventItem '{0}', eventId {1}", eitem->name, eventId );

      actionMgr.handleEventItemAction( player, actionId, eventId, targetId );

      break;
    }

    case Common::SkillType::MountSkill:
    {
      auto action = exdData.get< Data::Action >( 4 );
      assert( action );
      actionMgr.handleMountAction( player, static_cast< uint16_t >( actionId ), action, targetId, sequence );
      break;
    }
  }



}

void Sapphire::Network::GameConnection::placedActionHandler( const Packets::FFXIVARR_PACKET_RAW& inPacket,
                                                             Entity::Player& player )
{
  const auto packet = ZoneChannelPacket< Client::FFXIVIpcAoESkillHandler >( inPacket );

  const auto type = packet.data().type;
  const auto actionId = packet.data().actionId;
  const auto sequence = packet.data().sequence;
  const auto pos = packet.data().pos;

  // todo: find out if there are any other action types which actually use this handler
  if( type != Common::SkillType::Normal )
  {
    player.sendDebug( "Skill type {0} not supported by aoe action handler!", type );
    return;
  }

  player.sendDebug( "Skill type: {0}, sequence: {1}, actionId: {2}, x:{3}, y:{4}, z:{5}",
                    type, sequence, actionId, pos.x, pos.y, pos.z );

  auto& exdData = Common::Service< Data::ExdDataGenerated >::ref();

  auto action = exdData.get< Data::Action >( actionId );

  // ignore invalid actions
  if( !action )
    return;

  auto& actionMgr = Common::Service< World::Manager::ActionMgr >::ref();
  actionMgr.handlePlacedPlayerAction( player, actionId, action, pos, sequence );
}

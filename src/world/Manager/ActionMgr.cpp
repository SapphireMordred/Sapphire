#include "ActionMgr.h"

#include "Action/Action.h"
#include "Action/ItemAction.h"
#include "Action/MountAction.h"
#include "Script/ScriptMgr.h"
#include "Actor/Player.h"

#include <Exd/ExdDataGenerated.h>

#include <Network/PacketWrappers/EffectPacket.h>

#include <Service.h>

using namespace Sapphire;

void World::Manager::ActionMgr::handlePlacedPlayerAction( Entity::Player& player, uint32_t actionId,
                                                          Data::ActionPtr actionData, Common::FFXIVARR_POSITION3 pos,
                                                          uint16_t sequence )
{
  player.sendDebug( "got aoe act: {0}", actionData->name );


  auto action = Action::make_Action( player.getAsPlayer(), actionId, sequence, actionData );

  action->setPos( pos );

  if( !action->init() )
    return;

  if( !actionData->targetArea )
  {
    // not an action that has an aoe, cancel it
    action->interrupt();
    return;
  }

  bootstrapAction( player, action, *actionData );
}

void World::Manager::ActionMgr::handleTargetedPlayerAction( Entity::Player& player, uint32_t actionId,
                                                            Data::ActionPtr actionData, uint64_t targetId,
                                                            uint16_t sequence )
{
  auto action = Action::make_Action( player.getAsPlayer(), actionId, sequence, actionData );

  action->setTargetId( targetId );

  action->setPos( player.getPos() );

  if( !action->init() )
    return;

  // cancel any aoe actions casted with this packet
  if( actionData->targetArea )
  {
    action->interrupt();
    return;
  }

  bootstrapAction( player, action, *actionData );
}

void World::Manager::ActionMgr::handleItemAction( Sapphire::Entity::Player& player, uint32_t itemId,
                                                  Data::ItemActionPtr itemActionData,
                                                  uint16_t itemSourceSlot, uint16_t itemSourceContainer )
{
  player.sendDebug( "got item act: {0}, slot: {1}, container: {2}", itemId, itemSourceSlot, itemSourceContainer );

  auto action = Action::make_ItemAction( player.getAsChara(), itemId, itemActionData,
                                         itemSourceSlot, itemSourceContainer );

  // todo: item actions don't have cast times? if so we can just start it and we're good
  action->start();
}

void World::Manager::ActionMgr::handleEventItemAction( Sapphire::Entity::Player& player, uint32_t eventItemId,
                                                       uint32_t eventId, uint64_t targetId )
{
  player.sendDebug( "got eventitem act: {0}, eventId: {1}", eventItemId, eventId );

  player.eventItemActionStart( eventId, eventItemId,
    [ & ]( Entity::Player& player, uint32_t eventId, uint32_t eventItemId, uint64_t actorId )
    {
      player.sendDebug( "finishcallback called: eventId={0}, eventItemId={1}, actorId={2} ", eventId, eventItemId, actorId);
      auto& scriptMgr = Common::Service< Scripting::ScriptMgr >::ref();
      scriptMgr.onEventItem( player, eventId, eventItemId, actorId );

      player.checkEvent( eventId );
    }, nullptr, targetId );
}

void World::Manager::ActionMgr::handleMountAction( Entity::Player& player, uint16_t mountId,
                                                   Data::ActionPtr actionData, uint64_t targetId,
                                                   uint16_t sequence )
{
  player.sendDebug( "mount: {0}", mountId );

  auto action = Action::make_MountAction( player.getAsPlayer(), mountId, sequence, actionData );

  action->setTargetId( targetId );

  if( !action->init() )
    return;

  bootstrapAction( player, action, *actionData );
}

void World::Manager::ActionMgr::bootstrapAction( Entity::Player& player,
                                                 Action::ActionPtr currentAction,
                                                 Data::Action& actionData )
{
  if( !currentAction->preCheck() )
  {
    // forcefully interrupt the action and reset the cooldown
    currentAction->interrupt();
    return;
  }

  if( player.getCurrentAction() )
  {
    player.sendDebug( "Skill queued: {0}", currentAction->getId() );
    player.setQueuedAction( currentAction );
  }
  else
  {
    // if we have a cast time we want to associate the action with the player so update is called
    if( currentAction->hasCastTime() )
    {
      player.setCurrentAction( currentAction );
    }

    // todo: what do in cases of swiftcast/etc? script callback?
    currentAction->start();
  }
}

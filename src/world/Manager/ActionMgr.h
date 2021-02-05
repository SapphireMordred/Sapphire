#ifndef SAPPHIRE_ACTIONMGR_H
#define SAPPHIRE_ACTIONMGR_H

#include "ForwardsZone.h"

namespace Sapphire::Data
{
  struct Action;
  using ActionPtr = std::shared_ptr< Action >;

  struct ItemAction;
  using ItemActionPtr = std::shared_ptr< ItemAction >;
}

namespace Sapphire::World::Manager
{
  class ActionMgr
  {
  public:
    ActionMgr() = default;
    ~ActionMgr() = default;

    void handleTargetedPlayerAction( Entity::Player& player, uint32_t actionId,
                                     Data::ActionPtr actionData, uint64_t targetId, uint16_t sequence );
    void handlePlacedPlayerAction( Entity::Player& player, uint32_t actionId,
                                   Data::ActionPtr actionData, Common::FFXIVARR_POSITION3 pos, uint16_t sequence );

    void handleItemAction( Entity::Player& player, uint32_t itemId, Data::ItemActionPtr itemActionData,
                           uint16_t itemSourceSlot, uint16_t itemSourceContainer );
    void handleEventItemAction( Entity::Player& player, uint32_t eventItemId, uint32_t eventId, uint64_t targetId );

    void handleMountAction( Entity::Player& player, uint16_t mountId,
                            Data::ActionPtr actionData, uint64_t targetId, uint16_t sequence );

  private:
    void bootstrapAction( Entity::Player& player, Action::ActionPtr currentAction, Data::Action& actionData );

    // item action handlers
    void handleItemActionVFX( Entity::Player& player, uint32_t itemId, uint16_t vfxId );
  };
}

#endif //SAPPHIRE_ACTIONMGR_H

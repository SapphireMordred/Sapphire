#ifndef _PLAYERSPAWN_H
#define _PLAYERSPAWN_H

#include <src/servers/Server_Common/Network/PacketDef/Zone/ServerZoneDef.h>
#include <src/servers/Server_Common/Network/GamePacketNew.h>
#include <src/servers/Server_Common/Util/UtilMath.h>
#include "src/servers/Server_Zone/Actor/Player.h"
#include "src/servers/Server_Zone/Forwards.h"
#include "src/servers/Server_Zone/Inventory/Inventory.h"
#include "src/servers/Server_Zone/Inventory/Item.h"

namespace Core {
namespace Network {
namespace Packets {
namespace Server {

   /**
   * @brief The packet sent to spawn a player.
   */
   class PlayerSpawnPacket :
      public GamePacketNew< FFXIVIpcPlayerSpawn, ServerZoneIpcType >
   {
   public:
      PlayerSpawnPacket( Entity::PlayerPtr pPlayer, Entity::PlayerPtr pTarget ) :
         GamePacketNew< FFXIVIpcPlayerSpawn, ServerZoneIpcType >( pPlayer->getId(), pTarget->getId() )
      {
         initialize( pPlayer, pTarget );
      };

   private:
      void initialize( Entity::PlayerPtr pPlayer, Entity::PlayerPtr pTarget )
      {
         // todo: figure out unkown offsets
         // TODO: temporary gm rank
         //m_data.gmRank = 0xff;


         m_data.currentMount = 0;
         m_data.classJob = pPlayer->getClass();
         //m_data.status = static_cast< uint8_t >( pPlayer->getStatus() );
         m_data.hPCurr = pPlayer->getHp();
         m_data.mPCurr = pPlayer->getMp();
         m_data.tPCurr = pPlayer->getTp();
         m_data.hPMax = pPlayer->getMaxHp();
         m_data.mPMax = pPlayer->getMaxMp();
         m_data.gmRank = 0xff;
         //m_data.tPMax = 3000;
         m_data.level = pPlayer->getLevel();
         memcpy( m_data.look, pPlayer->getLookArray(), 26 );
         auto item = pPlayer->getInvetory()->getItemAt( Inventory::GearSet0, 0 );
         if( item )
            m_data.mainWeaponModel = item->getModelId1();
         m_data.secWeaponModel = pPlayer->getModelSubWeapon();
         m_data.models[0] = pPlayer->getModelForSlot( Inventory::EquipSlot::Head );
         m_data.models[1] = pPlayer->getModelForSlot( Inventory::EquipSlot::Body );
         m_data.models[2] = pPlayer->getModelForSlot( Inventory::EquipSlot::Hands );
         m_data.models[3] = pPlayer->getModelForSlot( Inventory::EquipSlot::Legs );
         m_data.models[4] = pPlayer->getModelForSlot( Inventory::EquipSlot::Feet );
         strcpy( m_data.name, pPlayer->getName().c_str() );
         m_data.pos.x = pPlayer->getPos().x;
         m_data.pos.y = pPlayer->getPos().y;
         m_data.pos.z = pPlayer->getPos().z;
         m_data.voice = pPlayer->getVoiceId();

         m_data.rotation = Math::Util::floatToUInt16Rot( pPlayer->getRotation() );

         m_data.onlineStatus = static_cast< uint8_t >( pPlayer->getOnlineStatus() );

         //m_data.u23 = 0x04;
         //m_data.u24 = 256;
         m_data.state = 1;
         m_data.type = 1;
         if( pTarget == pPlayer )
         {
            m_data.spawnIndex = 0x00;
         }
         else
         {
            m_data.spawnIndex = pTarget->getSpawnIdForActorId( pPlayer->getId() );
         }
         // 0x20 == spawn hidden to be displayed by the spawneffect control
         m_data.displayFlags = pPlayer->getStance();

         if( pPlayer->getZoningType() != Common::ZoneingType::None )
         {
            m_data.displayFlags |= 0x20;
         }

         m_data.targetId = pPlayer->getTargetId();
         //m_data.type = 1;
         //m_data.unknown_33 = 4;
         //m_data.unknown_38 = 0x70;
         //m_data.unknown_60 = 3;
         //m_data.unknown_61 = 7;

         for( auto const& effect : pPlayer->getStatusEffectContainer()->getEffectMap() )
         {
            m_data.effect[effect.first].effect_id = effect.second->getId();
            m_data.effect[effect.first].duration = effect.second->getDuration();
            m_data.effect[effect.first].sourceActorId = effect.second->getSrcActorId();
            m_data.effect[effect.first].unknown1 = effect.second->getParam();
         }


      };
   };

}
}
}
}

#endif /*_PlayerSpawn_H*/
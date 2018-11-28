#include <Common.h>
#include <Logging/Logger.h>
#include <Util/Util.h>
#include <Util/UtilMath.h>
#include <Database/DatabaseDef.h>
#include <Exd/ExdDataGenerated.h>
#include <Network/GamePacketNew.h>
#include <Network/PacketDef/Zone/ServerZoneDef.h>

#include "Actor/Player.h"
#include "Actor/Actor.h"
#include "Land.h"
#include "House.h"

#include "Forwards.h"
#include "HousingZone.h"
#include "HousingMgr.h"
#include "Framework.h"

extern Core::Framework g_fw;

using namespace Core::Common;
using namespace Core::Network::Packets;
using namespace Core::Network::Packets::Server;

Core::HousingZone::HousingZone( uint8_t wardNum,
                                uint16_t territoryTypeId,
                                uint32_t guId,
                                const std::string& internalName,
                                const std::string& contentName ) :
  Zone( territoryTypeId, guId, internalName, contentName ),
  m_wardNum( wardNum ),
  m_zoneId( territoryTypeId ),
  m_landSetId( ( static_cast< uint32_t >( territoryTypeId ) << 16 ) | wardNum )
{

}

bool Core::HousingZone::init()
{

  auto pDb = g_fw.get< Db::DbWorkerPool< Db::ZoneDbConnection > >();
  auto res = pDb->query( "SELECT * FROM landset WHERE landsetid = " + std::to_string( m_landSetId ) );
  if( !res->next() )
  {
    pDb->directExecute( "INSERT INTO landset ( landsetid ) VALUES ( " + std::to_string( m_landSetId ) + " );" );
  }

  int housingIndex;
  if( m_zoneId == 339 )
    housingIndex = 0;
  else if( m_zoneId == 340 )
    housingIndex = 1;
  else if( m_zoneId == 341 )
    housingIndex = 2;
  else if( m_zoneId == 641 )
    housingIndex = 3;

  auto pExdData = g_fw.get< Data::ExdDataGenerated >();
  auto info = pExdData->get< Core::Data::HousingLandSet >( housingIndex );

  uint32_t landId;
  for( landId = 0; landId < 60; landId++ )
  {
    auto pObject = make_Land( m_territoryTypeId, getWardNum(), landId, m_landSetId, info );
    m_landPtrMap[ landId ] = pObject;
  }

  return true;
}

Core::HousingZone::~HousingZone()
{

}

void Core::HousingZone::onPlayerZoneIn( Entity::Player& player )
{
  auto pLog = g_fw.get< Logger >();
  pLog->debug(
    "HousingZone::onPlayerZoneIn: Zone#" + std::to_string( getGuId() ) + "|" + std::to_string( getTerritoryTypeId() ) +
    ", Entity#" + std::to_string( player.getId() ) );

  uint32_t yardPacketNum;
  uint32_t yardPacketTotal = 8;

  sendLandSet( player );

  for( yardPacketNum = 0; yardPacketNum < yardPacketTotal; yardPacketNum++ )
  {
    auto landsetYardInitializePacket = makeZonePacket< FFXIVIpcLandSetYardInitialize >( player.getId() );
    landsetYardInitializePacket->data().unknown1 = 0xFFFFFFFF;
    landsetYardInitializePacket->data().unknown2 = 0xFFFFFFFF;
    landsetYardInitializePacket->data().unknown3 = 0xFF;
    landsetYardInitializePacket->data().packetNum = yardPacketNum;
    landsetYardInitializePacket->data().packetTotal = yardPacketTotal;

    //TODO: Add Objects here

    player.queuePacket( landsetYardInitializePacket );
  }

  auto landSetMap = makeZonePacket< FFXIVIpcLandSetMap >( player.getId() );
  landSetMap->data().subdivision = isPlayerSubInstance( player ) == false ? 2 : 1;
  uint8_t startIndex = isPlayerSubInstance( player ) == false ? 0 : 30;
  for( uint8_t i = startIndex, count = 0; i < ( startIndex + 30 ); i++, count++ )
  {
    landSetMap->data().landInfo[ count ].status = 1;
  }

  player.queuePacket( landSetMap );

}

void Core::HousingZone::sendLandSet( Entity::Player& player )
{
  auto landsetInitializePacket = makeZonePacket< FFXIVIpcLandSetInitialize >( player.getId() );

  landsetInitializePacket->data().landIdent.wardNum = m_wardNum;
  //landsetInitializePacket->data().landIdent.landSetId = m_landSetId;
  landsetInitializePacket->data().landIdent.territoryTypeId = m_territoryTypeId;
  //TODO: get current WorldId
  landsetInitializePacket->data().landIdent.worldId = 67;
  landsetInitializePacket->data().subInstance = isPlayerSubInstance( player ) == false ? 1 : 2;

  uint8_t startIndex = isPlayerSubInstance( player ) == false ? 0 : 30;

  for( uint8_t i = startIndex, count = 0; i < ( startIndex + 30 ); ++i, ++count )
  {
    auto pLand = getLand( i );

    // todo: move this and sendLandUpdate building logic to its own function
    auto& landData = landsetInitializePacket->data().land[ count ];

    landData.plotSize = pLand->getSize();
    landData.houseState = pLand->getState();
    landData.type = static_cast< uint8_t >( pLand->getLandType() );
    landData.iconAddIcon = pLand->getSharing();
    landData.fcId = pLand->getFcId();
    landData.fcIcon = pLand->getFcIcon();
    landData.fcIconColor = pLand->getFcColor();

    if( auto house = pLand->getHouse() )
    {
      auto& parts = house->getHouseParts();

      for( auto i = 0; i != parts.size(); i++ )
      {
        landData.housePart[ i ] = parts[ i ].first;
        landData.houseColour[ i ] = parts[ i ].second;
      }
    }
  }

  player.queuePacket( landsetInitializePacket );
}

void Core::HousingZone::sendLandUpdate( uint8_t landId )
{
  auto pLand = getLand( landId );
  for( const auto& playerIt : m_playerMap )
  {
    auto pPlayer = playerIt.second;

    auto landUpdatePacket = makeZonePacket< FFXIVIpcLandUpdate >( pPlayer->getId() );
    landUpdatePacket->data().landIdent.landId = landId;

    auto& landData = landUpdatePacket->data().land;

    landData.plotSize = pLand->getSize();
    landData.houseState = pLand->getState();
    landData.type = static_cast< uint8_t >( pLand->getLandType() );
    landData.iconAddIcon = pLand->getSharing();
    landData.fcId = pLand->getFcId();
    landData.fcIcon = pLand->getFcIcon();
    landData.fcIconColor = pLand->getFcColor();


    if( auto house = pLand->getHouse() )
    {
      auto& parts = house->getHouseParts();

      for( auto i = 0; i != parts.size(); i++ )
      {
        landData.housePart[ i ] = parts[ i ].first;
        landData.houseColour[ i ] = parts[ i ].second;
      }
    }

    pPlayer->queuePacket( landUpdatePacket );
  }
}

bool Core::HousingZone::isPlayerSubInstance( Entity::Player& player )
{
  return player.getPos().x < -15000.0f; //ToDo: get correct pos
}

void Core::HousingZone::onUpdate( uint32_t currTime )
{
  for( auto pLandItr : m_landPtrMap )
  {
    pLandItr.second->update( currTime );
  }
}

uint8_t Core::HousingZone::getWardNum() const
{
  return m_wardNum;
}

uint32_t Core::HousingZone::getLandSetId() const
{
  return m_landSetId;
}

Core::LandPtr Core::HousingZone::getLand( uint8_t id )
{
  auto it = m_landPtrMap.find( id );
  if( it == m_landPtrMap.end() )
    return nullptr;

  return it->second;
}

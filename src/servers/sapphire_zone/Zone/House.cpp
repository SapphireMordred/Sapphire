#include <set>
#include <string.h>

#include <Logging/Logger.h>
#include <Exd/ExdDataGenerated.h>
#include <Database/DatabaseDef.h>
#include <Database/ZoneDbConnection.h>

#include "House.h"

#include <unordered_map>
#include "Framework.h"

extern Core::Framework g_fw;

Core::House::House( uint32_t houseId, uint32_t landSetId, uint8_t landId, uint8_t wardNum, uint16_t territoryTypeId ) :
  m_houseId( houseId ),
  m_landSetId( landSetId ),
  m_landId( landId ),
  m_wardNum( wardNum ),
  m_territoryTypeId( territoryTypeId )
{
  auto pDB = g_fw.get< Db::DbWorkerPool< Db::ZoneDbConnection > >();

  // todo: convert to prepared statement
  auto res = pDB->query( "SELECT * FROM house WHERE HouseId = " + std::to_string( houseId ) );

  if( !res->next() )
  {
    g_fw.get< Core::Logger >()->info( "Creating house House#" + std::to_string( houseId ) + " in LandSet#" + std::to_string( landSetId ) );

    auto stmt = pDB->getPreparedStatement( Db::HOUSING_HOUSE_INS );

    stmt->setUInt( 1, m_landSetId );
    stmt->setUInt( 2, m_houseId );

    pDB->execute( stmt );
  }
  else
  {
    m_estateMessage = res->getString( "Comment" );
    m_houseName = res->getString( "HouseName" );

    auto housePartModels = res->getBlobVector( "HousePartModels" );
    auto housePartColours = res->getBlobVector( "HousePartColours" );

    auto models = reinterpret_cast< uint32_t* >( &housePartModels[ 0 ] );
    auto colours = &housePartColours[ 0 ];

    for( auto i = 0; i < 8; i++ )
    {
      m_houseParts[ i ] = { models[ i ], colours[ i ] };
    }
  }

  auto pExdData = g_fw.get< Data::ExdDataGenerated >();
  auto mapMarker = pExdData->get< Core::Data::HousingMapMarkerInfo >( getTerritoryTypeId(), getLandId() );

  if( mapMarker )
  {
    m_doorPosition.x = mapMarker->x;
    m_doorPosition.y = mapMarker->y;
    m_doorPosition.z = mapMarker->z;
  }
}

Core::House::~House()
{
}

void Core::House::updateHouseDb()
{
  auto pDB = g_fw.get< Db::DbWorkerPool< Db::ZoneDbConnection > >();

  // BuildTime = 1, Aetheryte = 2, Comment = 3, HouseName = 4, Endorsements = 5,
  // HousePartModels = 6, HousePartColours = 7, HouseId = 8
  auto stmt = pDB->getPreparedStatement( Db::HOUSING_HOUSE_UP );
  stmt->setUInt( 8, m_houseId );

  stmt->setInt64( 1, m_buildTime );
  stmt->setInt( 2, 0 );

  stmt->setString( 3, m_estateMessage );
  stmt->setString( 4, m_houseName );

  stmt->setUInt64( 5, 0 );

  std::vector< uint32_t > models;
  std::vector< uint8_t > colours;

  for( auto i = 0; i < 8; i++ )
  {
    auto& part = m_houseParts[ i ];
    models.push_back( part.first );
    colours.push_back( part.second );
  }

  // todo: this is shit
  std::vector< uint8_t > tmpModels( models.size() * 4 );
  memcpy( tmpModels.data(), models.data(), tmpModels.size() );

  stmt->setBinary( 6, tmpModels );
  stmt->setBinary( 7, colours );

  pDB->execute( stmt );
}

uint32_t Core::House::getLandSetId() const
{
  return m_landSetId;
}

uint8_t Core::House::getLandId() const
{
  return m_landId;
}

uint8_t Core::House::getWardNum() const
{
  return m_wardNum;
}

uint16_t Core::House::getTerritoryTypeId() const
{
  return m_territoryTypeId;
}

uint32_t Core::House::getHouseId() const
{
  return m_houseId;
}

Core::Common::FFXIVARR_POSITION3 Core::House::getDoorPosition() const
{
  return m_doorPosition;
}

uint8_t Core::House::getHousePartColor( Common::HousePartSlot slot ) const
{
  return m_houseParts[ slot ].second;
}

void Core::House::setHousePart( Common::HousePartSlot slot, uint32_t id )
{
  m_houseParts[ slot ].first = id;
}

void Core::House::setHousePartColor( Common::HousePartSlot slot, uint32_t id )
{
  m_houseParts[ slot ].second = id;
}

uint32_t Core::House::getHousePart( Common::HousePartSlot slot ) const
{
  return m_houseParts[ slot ].first;
}

Core::House::HousePartsArray const& Core::House::getHouseParts() const
{
  return m_houseParts;
}
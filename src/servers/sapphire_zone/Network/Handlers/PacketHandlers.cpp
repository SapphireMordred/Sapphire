#include <boost/format.hpp>

#include <Common.h>
#include <Network/CommonNetwork.h>
#include <Network/GamePacketNew.h>
#include <Logging/Logger.h>
#include <Network/PacketContainer.h>
#include <Network/PacketDef/Chat/ServerChatDef.h>
#include <Database/DatabaseDef.h>
#include <Database/DbWorkerPool.h>
#include <Database/CharaDbConnection.h>

#include <boost/format.hpp>
#include <unordered_map>
#include "Network/GameConnection.h"

#include "Zone/TerritoryMgr.h"
#include "Zone/Zone.h"
#include "Zone/ZonePosition.h"

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
#include "Social/Manager/SocialMgr.h"
#include "Social/FriendList.h"

#include "Session.h"
#include "ServerZone.h"
#include "Forwards.h"
#include "Framework.h"

extern Core::Framework g_fw;

using namespace Core::Common;
using namespace Core::Network::Packets;
using namespace Core::Network::Packets::Server;

void Core::Network::GameConnection::fcInfoReqHandler( const Packets::GamePacket& inPacket,
                                                      Entity::Player& player )
{
   GamePacketPtr pPe( new GamePacket( 0xDD, 0x78, player.getId(), player.getId() ) );
   pPe->setValAt< uint8_t >( 0x48, 0x01 );
   queueOutPacket( pPe );
}

void Core::Network::GameConnection::setSearchInfoHandler( const Packets::GamePacket& inPacket,
                                                          Entity::Player& player )
{
   uint32_t inval = inPacket.getValAt< uint32_t >( 0x20 );
   uint32_t inval1 = inPacket.getValAt< uint32_t >( 0x24 );
   uint64_t status = inPacket.getValAt< uint64_t >( 0x20 );

   uint8_t selectRegion = inPacket.getValAt< uint8_t >( 0x31 );

   player.setSearchInfo( selectRegion, 0, inPacket.getStringAt( 0x32 ) );

   player.setOnlineStatusMask( status );

   if( player.isNewAdventurer() && !( inval & 0x01000000 ) )
      // mark player as not new adventurer anymore
      player.setNewAdventurer( false );
   else if( inval & 0x01000000 )
      // mark player as new adventurer
      player.setNewAdventurer( true );

   ZoneChannelPacket< FFXIVIpcSetOnlineStatus > statusPacket( player.getId() );
   statusPacket.data().onlineStatusFlags = status;
   queueOutPacket( statusPacket );

   ZoneChannelPacket< FFXIVIpcSetSearchInfo > searchInfoPacket( player.getId() );
   searchInfoPacket.data().onlineStatusFlags = status;
   searchInfoPacket.data().selectRegion = player.getSearchSelectRegion();
   strcpy( searchInfoPacket.data().searchMessage, player.getSearchMessage() );
   queueOutPacket( searchInfoPacket );

   player.sendToInRangeSet( ActorControlPacket142( player.getId(), SetStatusIcon,
                                                   static_cast< uint8_t >( player.getOnlineStatus() ) ), true );
}

void Core::Network::GameConnection::reqSearchInfoHandler( const Packets::GamePacket& inPacket,
                                                          Entity::Player& player )
{
   ZoneChannelPacket< FFXIVIpcInitSearchInfo > searchInfoPacket( player.getId() );
   searchInfoPacket.data().onlineStatusFlags = player.getOnlineStatusMask();
   searchInfoPacket.data().selectRegion = player.getSearchSelectRegion();
   strcpy( searchInfoPacket.data().searchMessage, player.getSearchMessage() );
   queueOutPacket( searchInfoPacket );
}

void Core::Network::GameConnection::linkshellListHandler( const Packets::GamePacket& inPacket,
                                                          Entity::Player& player )
{
   ZoneChannelPacket< FFXIVIpcLinkshellList > linkshellListPacket( player.getId() );
   queueOutPacket( linkshellListPacket );
}

void Core::Network::GameConnection::updatePositionHandler( const Packets::GamePacket& inPacket,
                                                           Entity::Player& player )
{
   // if the player is marked for zoning we no longer want to update his pos
   if( player.isMarkedForZoning() )
      return;

   struct testMov
   {
      uint32_t specialMovement : 23; // 0x00490FDA
      uint32_t strafe : 7;
      uint32_t moveBackward : 1;
      uint32_t strafeRight : 1; // if 0, strafe left.
   } IPC_OP_019A;

   struct testMov1
   {
      uint16_t bit1 : 1; // 0x00490FDA
      uint16_t bit2 : 1;
      uint16_t bit3 : 1;
      uint16_t bit4 : 1;
      uint16_t bit5 : 1;
      uint16_t bit6 : 1;
      uint16_t bit7 : 1;
      uint16_t bit8 : 1;
      uint16_t bit9 : 1; // 0x00490FDA
      uint16_t bit10 : 1;
      uint16_t bit11 : 1;
      uint16_t bit12 : 1;
      uint16_t bit13 : 1;
      uint16_t bit14 : 1;
      uint16_t bit15 : 1;
      uint16_t bit16 : 1;
   } IPC_OP_019AB;

   uint16_t flags = inPacket.getValAt<uint16_t>( 0x28 );
   memcpy( &IPC_OP_019AB, &flags, 2 );

   uint32_t flags1 = inPacket.getValAt<uint32_t>( 0x24 );
   memcpy( &IPC_OP_019A, &flags1, 4 );

   //g_log.Log(LoggingSeverity::debug, "" + boost::lexical_cast<std::string>((int)IPC_OP_019AB.bit1)
   //                                     + boost::lexical_cast<std::string>((int)IPC_OP_019AB.bit2)
   //                                     + boost::lexical_cast<std::string>((int)IPC_OP_019AB.bit3)
   //                                     + boost::lexical_cast<std::string>((int)IPC_OP_019AB.bit4)
   //                                     + boost::lexical_cast<std::string>((int)IPC_OP_019AB.bit5)
   //                                     + boost::lexical_cast<std::string>((int)IPC_OP_019AB.bit6)
   //                                     + boost::lexical_cast<std::string>((int)IPC_OP_019AB.bit7)
   //                                     + boost::lexical_cast<std::string>((int)IPC_OP_019AB.bit8)
   //                                     + boost::lexical_cast<std::string>((int)IPC_OP_019AB.bit9)
   //                                     + boost::lexical_cast<std::string>((int)IPC_OP_019AB.bit10)
   //                                     + boost::lexical_cast<std::string>((int)IPC_OP_019AB.bit11)
   //                                     + boost::lexical_cast<std::string>((int)IPC_OP_019AB.bit12)
   //                                     + boost::lexical_cast<std::string>((int)IPC_OP_019AB.bit13)
   //                                     + boost::lexical_cast<std::string>((int)IPC_OP_019AB.bit14)
   //                                     + boost::lexical_cast<std::string>((int)IPC_OP_019AB.bit15)
   //                                     + boost::lexical_cast<std::string>((int)IPC_OP_019AB.bit16)
   //                                     + " " + boost::lexical_cast<std::string>((int)flags));

   //g_log.Log(LoggingSeverity::debug, "\n" + boost::lexical_cast<std::string>((int)IPC_OP_019A.specialMovement) + "\n"
   //                                     + boost::lexical_cast<std::string>((int)IPC_OP_019A.strafe) + "\n"
   //                                     + boost::lexical_cast<std::string>((int)IPC_OP_019A.moveBackward) + "\n"
   //                                     + boost::lexical_cast<std::string>((int)IPC_OP_019A.strafeRight));

   //g_log.Log(LoggingSeverity::debug, pInPacket->toString());

   //pInPacket->debugPrint();

   bool bPosChanged = false;
   if( ( player.getPos().x != inPacket.getValAt< float >( 0x2c ) ) ||
       ( player.getPos().y != inPacket.getValAt< float >( 0x30 ) ) ||
       ( player.getPos().z != inPacket.getValAt< float >( 0x34 ) ) )
      bPosChanged = true;
   if( !bPosChanged  && player.getRot() == inPacket.getValAt< float >( 0x20 ) )
      return;

   player.setRot( inPacket.getValAt< float >( 0x20 ) );
   player.setPos( inPacket.getValAt< float >( 0x2c ),
                  inPacket.getValAt< float >( 0x30 ),
                  inPacket.getValAt< float >( 0x34 ) );

   if( ( player.getCurrentAction() != nullptr ) && bPosChanged )
      player.getCurrentAction()->setInterrupted();

   // if no one is in range, don't bother trying to send a position update
   if( !player.hasInRangeActor() )
      return;

   uint8_t unk = inPacket.getValAt< uint8_t >( 0x29 );

   uint16_t moveType = inPacket.getValAt< uint16_t >( 0x28 );

   uint8_t unk1 = 0;
   uint8_t unk2 = 0;
   uint8_t unk3 = unk;
   uint16_t unk4 = 0;

   // HACK: This part is hackish, we need to find out what all theese things really do.
   switch( moveType )
   {
   case MoveType::Strafe:
   {
      if( IPC_OP_019A.strafeRight == 1 )
         unk1 = 0xbf;
      else
         unk1 = 0x5f;
      unk4 = 0x3C;
      break;
   }

   case 6:
   {
      unk1 = 0xFF;
      unk2 = 0x06;
      unk4 = 0x18;
      break;
   }

   //   case MoveType::Land:
   //	{
   //           unk1 = 0x7F;
   //		//unk2 = 0x40;
   //		unk4 = 0x3C;
   //		break;
   //	}

   //   case MoveType::Jump:
   //	{
   //		unk1 = 0x7F;
   //		if(unk == 0x01)
   //           {
   //		//	unk2 = 0x20;
   //			//unk4 = 0x32;
   //               unk4 = 0x32;
   //		}
   //           else
   //           {
   //		//	unk2 = 0xA0;
   //			unk4 = 0x3C;
   //		}
   //
   //		break;
   //	}
   //   case MoveType::Fall:
   //	{
   //		unk1 = 0x7F;
   //		//unk2 = 0xA0;
   //		unk4 = 0x3C;
   //
   //		break;
   //	}
   default:
   {
      if( static_cast< int32_t >( IPC_OP_019A.moveBackward ) )
      {
         unk1 = 0xFF;
         unk2 = 0x06;
         unk4 = 0x18; // animation speed?
      }
      else
      {
         unk1 = 0x7F;
         unk4 = 0x3C; // animation speed?
      }

      break;
   }
   }

   MoveActorPacket movePacket( player, unk1, unk2, unk3, unk4 );
   player.sendToInRangeSet( movePacket );

}

void Core::Network::GameConnection::reqEquipDisplayFlagsHandler( const Packets::GamePacket& inPacket,
                                                                 Entity::Player& player )
{
   player.setEquipDisplayFlags( inPacket.getValAt< uint8_t >( 0x20 ) );
}

void Core::Network::GameConnection::zoneLineHandler( const Packets::GamePacket& inPacket,
                                                     Entity::Player& player )
{
   auto pTeriMgr = g_fw.get< TerritoryMgr >();
   uint32_t zoneLineId = inPacket.getValAt< uint32_t >( 0x20 );

   player.sendDebug( "Walking ZoneLine " + std::to_string( zoneLineId ) );

   auto pZone = player.getCurrentZone();

   auto pLine = pTeriMgr->getTerritoryPosition( zoneLineId );

   Common::FFXIVARR_POSITION3 targetPos{};
   uint32_t targetZone;
   float rotation = 0.0f;

   if( pLine != nullptr )
   {
      player.sendDebug( "ZoneLine " + std::to_string( zoneLineId ) + " found." );
      targetPos = pLine->getTargetPosition();
      targetZone = pLine->getTargetZoneId();
      rotation = pLine->getTargetRotation();

      ZoneChannelPacket< FFXIVIpcPrepareZoning > preparePacket( player.getId() );
      preparePacket.data().targetZone = targetZone;

      //ActorControlPacket143 controlPacket( pPlayer, ActorControlType::DespawnZoneScreenMsg,
      //                                     0x03, player.getId(), 0x01, targetZone );
      player.queuePacket( preparePacket );
   }
   else
   {
      // No zoneline found, revert to last zone
      player.sendUrgent( "ZoneLine " + std::to_string( zoneLineId ) + " not found." );
      targetPos.x = 0;
      targetPos.y = 0;
      targetPos.z = 0;
      targetZone = pZone->getTerritoryId();
   }

   player.performZoning( targetZone, targetPos, rotation);
}


void Core::Network::GameConnection::discoveryHandler( const Packets::GamePacket& inPacket,
                                                      Entity::Player& player )
{
   uint32_t ref_position_id = inPacket.getValAt< uint32_t >( 0x20 );

   auto pDb = g_fw.get< Db::DbWorkerPool< Db::CharaDbConnection > >();

   auto pQR = pDb->query( "SELECT id, map_id, discover_id "
                               "FROM discoveryinfo "
                               "WHERE id = " + std::to_string( ref_position_id ) + ";" );

   if( !pQR->next() )
   {
      player.sendNotice( "Discovery ref pos ID: " + std::to_string( ref_position_id ) + " not found. " );
      return;
   }

   ZoneChannelPacket< FFXIVIpcDiscovery > discoveryPacket( player.getId() );
   discoveryPacket.data().map_id = pQR->getUInt( 2 );
   discoveryPacket.data().map_part_id = pQR->getUInt( 3 );

   player.queuePacket( discoveryPacket );
   player.sendNotice( "Discovery ref pos ID: " + std::to_string( ref_position_id ) );

   player.discover( pQR->getUInt16( 2 ), pQR->getUInt16( 3 ) );

}


void Core::Network::GameConnection::playTimeHandler( const Packets::GamePacket& inPacket,
                                                     Entity::Player& player )
{
   ZoneChannelPacket< FFXIVIpcPlayTime > playTimePacket( player.getId() );
   playTimePacket.data().playTimeInMinutes = player.getPlayTime() / 60;
   player.queuePacket( playTimePacket );
}


void Core::Network::GameConnection::initHandler( const Packets::GamePacket& inPacket,
                                                 Entity::Player& player )
{
   // init handler means this is a login procedure
   player.setIsLogin( true );

   player.sendZonePackets();
}


void Core::Network::GameConnection::blackListHandler( const Packets::GamePacket& inPacket,
                                                      Entity::Player& player )
{
   uint8_t count = inPacket.getValAt< uint8_t >( 0x21 );

   ZoneChannelPacket< FFXIVIpcBlackList > blackListPacket( player.getId() );
   blackListPacket.data().sequence = count;
   // TODO: Fill with actual blacklist data
   //blackListPacket.data().entry[0].contentId = 1;
   //sprintf( blackListPacket.data().entry[0].name, "Test Test" );
   queueOutPacket( blackListPacket );

}


void Core::Network::GameConnection::pingHandler( const Packets::GamePacket& inPacket,
                                                 Entity::Player& player )
{
   int32_t inVal = inPacket.getValAt< int32_t >( 0x20 );
   PingPacket pingPacket( player, inVal );
   queueOutPacket( pingPacket );

   player.setLastPing( static_cast< uint32_t >( time( nullptr ) ) );
}


void Core::Network::GameConnection::finishLoadingHandler( const Packets::GamePacket& inPacket,
                                                          Entity::Player& player )
{
   player.getCurrentZone()->onFinishLoading( player );

   // player is done zoning
   player.setLoadingComplete( true );

   // if this is a login event
   if( player.isLogin() )
   {
      // fire the onLogin Event
      player.onLogin();
      player.setIsLogin( false );
   }

   // spawn the player for himself
   player.spawn( player.getAsPlayer() );

   // notify the zone of a change in position to force an "inRangeActor" update
   player.getCurrentZone()->updateActorPosition(player);
}

void Core::Network::GameConnection::socialListHandler( const Packets::GamePacket& inPacket,
                                                       Entity::Player& player )
{
    
   uint8_t type = inPacket.getValAt< uint8_t >( 0x2A );
   uint8_t count = inPacket.getValAt< uint8_t >( 0x2B );

   if( type == SocialListType::PartyList )
   { // party list

      ZoneChannelPacket< FFXIVIpcSocialList > listPacket( player.getId() );

      listPacket.data().type = 2;
      listPacket.data().sequence = count;

      int32_t entrysizes = sizeof( listPacket.data().entries );
      memset( listPacket.data().entries, 0, sizeof( listPacket.data().entries ) );

      listPacket.data().entries[0].bytes[2] = player.getCurrentZone()->getTerritoryId();
      listPacket.data().entries[0].bytes[3] = 0x80;
      listPacket.data().entries[0].bytes[4] = 0x02;
      listPacket.data().entries[0].bytes[6] = 0x3B;
      listPacket.data().entries[0].bytes[11] = 0x10;
      listPacket.data().entries[0].classJob = player.getClass();
      listPacket.data().entries[0].contentId = player.getContentId();
      listPacket.data().entries[0].level = player.getLevel();
      listPacket.data().entries[0].zoneId = player.getCurrentZone()->getTerritoryId();
      // listPacket.data().entries[0].zoneId1 = 0x0100;
      // TODO: no idea what this does
      //listPacket.data().entries[0].one = 1;

      memcpy( listPacket.data().entries[0].name, player.getName().c_str(), strlen( player.getName().c_str() ) );

      // TODO: actually store and read language from somewhere
      listPacket.data().entries[0].bytes1[0] = 0x01;//flags (lang)
                                                    // TODO: these flags need to be figured out
                                                    //listPacket.data().entries[0].bytes1[1] = 0x00;//flags
      listPacket.data().entries[0].onlineStatusMask = player.getOnlineStatusMask();

      queueOutPacket( listPacket );

   }
   else if( type == SocialListType::FriendList )
   { // friend list

      ZoneChannelPacket< FFXIVIpcSocialList > listPacket( player.getId() );
      listPacket.data().type = 0x0B;
      listPacket.data().sequence = count;
      memset( listPacket.data().entries, 0, sizeof( listPacket.data().entries ) );

      uint16_t i = 0;

      auto playerFriendsList = g_fw.get< Social::SocialMgr< Social::FriendList > >()->findGroupById( player.getFriendsListId() );

      // todo: move this garbage else fucking where
      for ( auto member : playerFriendsList->getMembers() )
      {
         // more elegant way to break over list entries pls
         if ( i == 10 )
            break;

         g_fw.get< Logger >()->debug( "aaa" + std::to_string( i ) );
         // todo: replace this with call for generating the entire vector
         listPacket.data().entries[i] = playerFriendsList->generatePlayerEntry( member );
         i++;
      }

      queueOutPacket( listPacket );

   }
   else if( type == SocialListType::SearchList )
   { // player search result
     // TODO: implement player search
   }

}

void Core::Network::GameConnection::socialReqResponseHandler( const Packets::GamePacket& inPacket,
                                                              Entity::Player& player )
{
   auto targetId = inPacket.getValAt< uint32_t >( 0x20 );
   auto category = inPacket.getValAt< Common::SocialCategory >( 0x28 );
   auto action = inPacket.getValAt< Common::SocialRequestAction >( 0x29 );

   ZoneChannelPacket< FFXIVIpcSocialRequestError > info( targetId, player.getId() );
   ZoneChannelPacket< FFXIVIpcSocialRequestResponse > response( targetId, player.getId() );

   info.data().category = category;
   response.data().category = category;

   //auto pQR = g_database.query( "SELECT Name FROM dbchara WHERE CharacterId = " + to_string( targetId ) );
   auto name = player.getName();
   /*
   if( pQR->getRowCount() > 0 )
   {
      name = pQR->fetch()->getString();
   }
   else
   {
      // todo: enumerate these messages
      std::array< uint32_t, 5> categoryError = { 0,
         320, // Unable to process party command.
         310, // Unable to process friend list command.
         3035, // Unable to reply. Free company invite invalid. // todo: find actual message
         3035, // Unable to reply. Free company invite invalid.
      };

      response.data().messageId = categoryError[category];
      response.data().category = category; // client only uses messageId if this is 1 for some reason
      pPlayer->queuePacket( response );
      return;
   }*/

   g_fw.get< Logger >()->debug( std::to_string( static_cast<uint8_t>( action ) ) );

   auto pSession = g_fw.get< ServerZone >()->getSession( targetId );

   // todo: notify both inviter/invitee with 0x00CB packet

   if( pSession )
   {
      g_fw.get< Logger >()->debug( std::to_string( static_cast< uint8_t >( action ) ) );
   }
   response.data().response = Common::SocialRequestResponse::Accept;
   memcpy( &( response.data().name ), name.c_str(), 32 );
   player.queuePacket( response );
}

void Core::Network::GameConnection::socialReqSendHandler( const Packets::GamePacket& inPacket,
                                                          Entity::Player& player )
{
      // todo: handle all social request packets here
   auto category = inPacket.getValAt< Common::SocialCategory >( 0x20 );
   auto name = std::string( inPacket.getStringAt( 0x21 ) );

   auto pSession = g_fw.get< ServerZone >()->getSession( name );

   // Variable for error logging and control
   uint32_t logMessage = 0;

   // only the requester needs the response

   // todo: enumerate and move each of these cases into their classes?
   if( pSession )
   {
      Entity::PlayerPtr pRecipient = pSession->getPlayer();

      std::array< std::string, 5 > typeVar{ "", "PartyInvite", "FriendInvite", "FreeCompanyPetition", "FreeCompanyInvite" };

      // todo: proper handling of invites already sent
      // todo: move this to world server
      // check if an invite has already been sent by me
      /*
      if( pRecipient->getTempVariable( typeVar[category] + "Id" ) )
      {
         if( pRecipient->getTempVariable( typeVar[category] + "Id" ) == pPlayer->getId() )
         {
            response.data().messageId = 328; // That player has already been invited.
            pPlayer->queuePacket( response );
         }
         return;
      }*/
      
      /*
      if( pRecipient->getId() == player.getId() )
      {
         response.data().messageId = 321; // Unable to invite.
      }
      */
      switch( category )
      {
         // party invite
         case Core::Common::SocialCategory::Party:
         {
            /*
            if( pRecipient->getParty() )
            {
               response.data().messageId = 326; // That player is already in another party.
            }
            else if( pRecipient->getTempVariable( "PartyInviteId" ) == pPlayer->getId() )
            {
               response.data().messageId = 328; // That player has already been invited.
            }
            else if( pPlayer->getParty() && pPlayer->getParty()->getPartyMemberCount() >= pPlayer->getParty()->getType() )
            {
               response.data().messageId = 329; // Unable to invite. The party is full.
            }*/
            if( !pRecipient->isLoadingComplete() ) // || pRecipient->getDuty() )
            {
               logMessage = 331; // Unable to invite. That player is currently bound by duty or in a different area.
            }
            else if( pRecipient->getOnlineStatus() == Common::OnlineStatus::Busy )
            {
               logMessage = 334; // Unable to send party invite. Player's online status is set to Busy.
            }
            else if( pRecipient->getOnlineStatus() == Common::OnlineStatus::ViewingCutscene )
            {
               logMessage = 336; // Unable to invite. That player is currently watching a cutscene.
            }
            // response.data().messageId = 62; // <name> declines the party invite.
         }
         break;

         case Common::SocialCategory::Friends:
         {
            // todo: check if already on friends list or invite pending

            auto recipientId = pRecipient->getId();
            auto senderId = player.getId();

            auto senderFriendsList = g_fw.get< Social::SocialMgr< Social::FriendList > >()->findGroupById( player.getFriendsListId() );
            auto recipientFriendsList = g_fw.get< Social::SocialMgr< Social::FriendList > >()->findGroupById( pRecipient->getFriendsListId() );

            // If any of these are true, an error has occured.
            if( senderFriendsList->hasMember( recipientId ) )
            {
               logMessage = 312; // That player is already a friend or has been sent a request.
            }
            else if( senderFriendsList->getTotalSize() >= senderFriendsList->getCapacity() )
            {
               logMessage = 313; // Your friend list is full.
            }
            else if( recipientFriendsList->getTotalSize() >= recipientFriendsList->getCapacity() )
            {
               logMessage = 314; // Unable to send friend request. The other player's friend list is full.
            }
            else if( pRecipient->getOnlineStatus() == Common::OnlineStatus::Busy )
            {
               logMessage = 316; // Unable to send friend request. Player's online status is set to Busy.
            }
            else
            {
               // Catch any other, unreported mess
               //logMessage = senderFriendsList->addInvite( recipientId );
            }

         }
         break;

         default:
            break;
      }

      // No errors reported
      if( logMessage == 0 )
      {
         ZoneChannelPacket< FFXIVIpcSocialRequestReceive > packet( player.getId(), pRecipient->getId() );

         std::array< uint16_t, 5 > typeMessage{ 0,
            1, // You invite <name> to a party.
            2, // You send a friend request to <name>.
            4, // Free company petition signature request sent to <name>
            5, // You invite <name> to your free company.
         };

         // TODO: confirm the timers on retail
         auto expireTime = time( nullptr ) + 120;

         // todo: fix this for cross zone parties (move to world server)
         /*
         pRecipient->setTempVariable( typeVar[category] + "Id", pPlayer->getId() );
         pRecipient->setTempVariable( typeVar[category] + "Timer", expireTime );*/

         packet.data().actorId = player.getId();
         packet.data().category = category;
         packet.data().action = Core::Common::SocialRequestAction::Invite;
         packet.data().unknown3 = 80; // these seem like bitmasks!
         packet.data().unknown = 46;
         packet.data().unknown2 = 64;
         memcpy( &( packet.data().name ), player.getName().c_str(), 32 );

         pRecipient->queuePacket( packet );
         pRecipient->sendDebug( "ding ding" );

         auto recipientFriendsList = g_fw.get< Social::SocialMgr< Social::FriendList > >()->findGroupById( pRecipient->getFriendsListId() );

         //recipientFriendsList->addInvite( player.getId() );
         
         auto senderResultPacket = GamePacketNew< Server::FFXIVIpcSocialRequestResponse, ServerZoneIpcType >( pRecipient->getId(), player.getId() );
         senderResultPacket.data().contentId = pRecipient->getId();
         senderResultPacket.data().category = Common::SocialCategory::Friends;
         senderResultPacket.data().response = Common::SocialRequestResponse::Cancel;

         memcpy( &( senderResultPacket.data().name ), pRecipient->getName().c_str(), 32 );

         //todo: build packet from packetresult here
         player.queuePacket( senderResultPacket );

         if( recipientFriendsList->isFriendList() )
         {
            g_fw.get< Logger >()->debug( "he HAA HAAA" );
         }

         //response.data().messageId = typeMessage[category];
      }
      else
      {
         ZoneChannelPacket< FFXIVIpcSocialRequestError > error( player.getId() );
         memcpy( &( error.data().name ), name.c_str(), 32 );

         // todo: enumerate log messages
         error.data().messageId = logMessage; // That name does not exist. Please confirm the spelling.
         error.data().category = category;
         // Errors have happened. Send it back only to requester

         player.queuePacket( error );
      }
   }

   //player.queuePacket( response );
   // todo: handle party, friend request
   g_fw.get< Logger >()->debug( "sent to " + name );
}

void Core::Network::GameConnection::chatHandler( const Packets::GamePacket& inPacket,
                                                 Entity::Player& player )
{

   auto pDebugCom = g_fw.get< DebugCommandHandler >();
   std::string chatString( inPacket.getStringAt( 0x3a ) );

   uint32_t sourceId = inPacket.getValAt< uint32_t >( 0x24 );

   if( chatString.at( 0 ) == '!' )
   {
      // execute game console command
      pDebugCom->execCommand( const_cast< char * >( chatString.c_str() ) + 1, player );
      return;
   }

   ChatType chatType = static_cast< ChatType >( inPacket.getValAt< uint8_t >( 0x38 ) );

   //ToDo, need to implement sending GM chat types.
   ChatPacket chatPacket( player, chatType, chatString );

   switch( chatType )
   {
   case ChatType::Say:
   {
      player.getCurrentZone()->queueOutPacketForRange( player, 50, chatPacket );
      break;
   }
   case ChatType::Yell:
   {
      player.getCurrentZone()->queueOutPacketForRange( player, 6000, chatPacket );
      break;
   }
   case ChatType::Shout:
   {
      player.getCurrentZone()->queueOutPacketForRange( player, 6000, chatPacket );
      break;
   }
   default:
   {
      player.getCurrentZone()->queueOutPacketForRange( player, 50, chatPacket );
      break;
   }
   }

}

// TODO: this handler needs to be improved for timed logout, also the session should be instantly removed
// currently we wait for the session to just time out after logout, this can be a problem is the user tries to
// log right back in.
// Also the packet needs to be converted to an ipc structure
void Core::Network::GameConnection::logoutHandler( const Packets::GamePacket& inPacket,
                                                   Entity::Player& player )
{
   ZoneChannelPacket< FFXIVIpcLogout > logoutPacket( player.getId() );
   logoutPacket.data().flags1 = 0x02;
   logoutPacket.data().flags2 = 0x2000;
   queueOutPacket( logoutPacket );

   player.setMarkedForRemoval();
}


void Core::Network::GameConnection::tellHandler( const Packets::GamePacket& inPacket,
                                                 Entity::Player& player )
{
   std::string targetPcName = inPacket.getStringAt( 0x21 );
   std::string msg = inPacket.getStringAt( 0x41 );

   auto pZoneServer = g_fw.get< ServerZone >();

   auto pSession = pZoneServer->getSession( targetPcName );

   if( !pSession )
   {
      ChatChannelPacket< FFXIVIpcTellErrNotFound > tellErrPacket( player.getId() );
      strcpy( tellErrPacket.data().receipientName, targetPcName.c_str() );
      sendSinglePacket( tellErrPacket );
      return;
   }

   auto pTargetPlayer = pSession->getPlayer();

   if( pTargetPlayer->hasStateFlag( PlayerStateFlag::BetweenAreas ) )
   {
      // send error for player between areas
      // TODO: implement me
      return;
   }

   if( pTargetPlayer->hasStateFlag( PlayerStateFlag::BoundByDuty ) )
   {
      // send error for player bound by duty
      // TODO: implement me
      return;
   }

   if( pTargetPlayer->getOnlineStatus() == OnlineStatus::Busy )
   {
      // send error for player being busy
      // TODO: implement me ( i've seen this done with packet type 67 i think )
      return;
   }

   ChatChannelPacket< FFXIVIpcTell > tellPacket( player.getId() );
   strcpy( tellPacket.data().msg, msg.c_str() );
   strcpy( tellPacket.data().receipientName, player.getName().c_str() );
   // TODO: do these have a meaning?
   //tellPacket.data().u1 = 0x92CD7337;
   //tellPacket.data().u2a = 0x2E;
   //tellPacket.data().u2b = 0x40;
   pTargetPlayer->queueChatPacket( tellPacket );

}

void Core::Network::GameConnection::performNoteHandler( const Packets::GamePacket& inPacket,
                                                        Entity::Player& player )
{
   ZoneChannelPacket< FFXIVIpcPerformNote > performPacket( player.getId() );

   auto inVal = inPacket.getDataAt( 0x20 );
   memcpy( &performPacket.data().data[0], inVal, 32 );

   player.sendToInRangeSet( performPacket );
}

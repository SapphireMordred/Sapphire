#include <common/Common.h>
#include <common/Exd/ExdDataGenerated.h>

#include "Framework.h"
#include "EventHelper.h"
#include "EventHandler.h"


extern Core::Framework g_framework;

using namespace Core::Common;

std::string Core::Event::getEventName( uint32_t eventId )
{
   uint16_t eventType = eventId >> 16;

   auto unknown = std::string{ "unknown" };

   switch( eventType )
   {
   case Event::EventHandler::EventHandlerType::Quest:
   {
      auto questInfo = g_framework.getExdDataGen().get< Core::Data::Quest >( eventId );
      if( !questInfo )
         return unknown + "Quest";

      std::string name = questInfo->id;
      std::size_t pos = name.find_first_of( "_" );

      return name.substr( 0, pos );
   }
   case Event::EventHandler::EventHandlerType::CustomTalk:
   {
      auto customTalkInfo = g_framework.getExdDataGen().get< Core::Data::CustomTalk >( eventId );
      if( !customTalkInfo )
         return unknown + "CustomTalk";

      std::string name = customTalkInfo->name;
      std::size_t pos = name.find_first_of( "_" );

      return customTalkInfo->name.substr( 0, pos );
   }
   case Event::EventHandler::EventHandlerType::Opening:
   {
      auto openingInfo = g_framework.getExdDataGen().get< Core::Data::Opening >( eventId );
      if( openingInfo )
         return openingInfo->name;
      return unknown + "Opening";
   }
   case Event::EventHandler::EventHandlerType::Aetheryte:
   {
      auto aetherInfo = g_framework.getExdDataGen().get< Core::Data::Aetheryte >( eventId & 0xFFFF );
      if( aetherInfo->isAetheryte )
         return "Aetheryte";
      return "Aethernet";
   }

   case Event::EventHandler::EventHandlerType::Warp:
   {
      return "ChocoboTaxi";
   }
   default:
   {
      return unknown;
   }
   }
}

uint32_t Core::Event::mapEventActorToRealActor( uint32_t eventActorId )
{
   auto levelInfo = g_framework.getExdDataGen().get< Core::Data::Level >( eventActorId );
   if( levelInfo )
      return levelInfo->objectKey;

   return 0;
}

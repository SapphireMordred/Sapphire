// This is an automatically generated C++ script template
// Content needs to be added by hand to make it function
// In order for this script to be loaded, move it to the correct folder in <root>/scripts/

#include <Actor/Player.h>
#include "Manager/EventMgr.h"
#include <ScriptObject.h>
#include <Service.h>

// Quest Script: SubFst000_00020
// Quest Name: Incense and Sensibility
// Quest ID: 65556
// Start NPC: 1000786
// End NPC: 1000705

using namespace Sapphire;

class SubFst000 : public Sapphire::ScriptAPI::EventScript
{
  private:
    // Basic quest information
    // Quest vars / flags used
    // GetQuestBitFlag8
    // GetQuestUI8AL
    // GetQuestUI8BH

    // Steps in this quest ( 0 is before accepting,
    // 1 is first, 255 means ready for turning it in
    enum Sequence : uint8_t
    {
      Seq0 = 0,
      Seq1 = 1,
      Seq2 = 2,
      SeqFinish = 255,
    };

    // Entities found in the script data of the quest
    static constexpr auto Actor0 = 1000786;
    static constexpr auto Actor1 = 1000705;
    static constexpr auto Eobject0 = 2000070;
    static constexpr auto Eobject1 = 2000071;
    static constexpr auto Item0 = 2000101;
    static constexpr auto Seq0Actor0 = 0;
    static constexpr auto Seq1Actor1 = 1;
    static constexpr auto Seq2Eobject0 = 2;
    static constexpr auto Seq2Eobject0Useitemno = 99;
    static constexpr auto Seq2Eobject0Useitemok = 100;
    static constexpr auto Seq2Eobject1 = 3;
    static constexpr auto Seq2Eobject1Useitemno = 97;
    static constexpr auto Seq2Eobject1Useitemok = 98;
    static constexpr auto Seq3Actor1 = 4;

  public:
    SubFst000() : Sapphire::ScriptAPI::EventScript( 65556 ){};
    ~SubFst000() = default;

  //////////////////////////////////////////////////////////////////////
  // Event Handlers
  void onTalk( uint32_t eventId, Entity::Player& player, uint64_t actorId ) override
  {
    auto& eventMgr = Common::Service< World::Manager::EventMgr >::ref();
    auto actor = eventMgr.mapEventActorToRealActor( static_cast< uint32_t >( actorId ) );

    if ( actor == Actor0 )
      Scene00000( player );
    else if ( actor == Actor1 && player.getQuestSeq( getId() ) == Seq1 )
      Scene00001( player );
    else if ( actor == Actor1 && player.getQuestSeq( getId() ) == SeqFinish )
      Scene00004( player );
    else if ( actor == Eobject0 )
      Scene00002( player );
    else if ( actor == Eobject1 )
      Scene00003( player );
  }

  void onEventItem( Entity::Player& player, uint32_t eventId,
                    uint32_t eventItemId, uint64_t actorId ) override
  {
    auto& eventMgr = Common::Service< World::Manager::EventMgr >::ref();
    auto actor = eventMgr.mapEventActorToRealActor( static_cast< uint32_t >( actorId ) );

    player.sendDebug( "onEventItem called for this quest: eventId={0}, eventItemId={1}, actor={2}", eventId, eventItemId, actor );

    if ( actor == Eobject0 )
      Scene00100( player );
    else if ( actor == Eobject1 )
      Scene00098( player );
  }

  private:

  void checkQuestCompletion( Entity::Player& player )
  {
    auto currentUsed  = player.getQuestUI8AL( getId() );
    auto currentItems = player.getQuestUI8BH( getId() );

    player.sendQuestMessage( getId(), 1, 2, currentUsed + 1, 2 );

    if( currentUsed + 1 >= 2 )
    {
      player.updateQuest( getId(), SeqFinish );
    }
    else
    {
      player.setQuestUI8AL( getId(), currentUsed  + 1 );
      player.setQuestUI8BH( getId(), currentItems - 1 );
    }
  }

  //////////////////////////////////////////////////////////////////////
  // Available Scenes in this quest, not necessarly all are used
  void Scene00000( Entity::Player& player )
  {
    auto callback = [ & ]( Entity::Player& player, const Event::SceneResult& result )
    {
      if ( result.param2 == 1 ) // accept quest
      {
        player.updateQuest( getId(), Seq1 );
      }
    };

    player.playScene( getId(), 0, HIDE_HOTBAR, callback );
  }

  void Scene00001( Entity::Player& player )
  {
    auto callback = [ & ]( Entity::Player& player, const Event::SceneResult& result )
    {
      player.setQuestUI8AL( getId(), 0 ); // number of censers used
      player.setQuestUI8BH( getId(), 2 ); // Azeyma Rose Oil key item
      player.sendQuestMessage( getId(), 0, 0, 0, 0 );
      player.updateQuest( getId(), Seq2 );
    };

    player.playScene( getId(), 1, HIDE_HOTBAR, callback );
  }

  void Scene00002( Entity::Player& player )
  {
    auto callback = [ & ]( Entity::Player& player, const Event::SceneResult& result )
    {
    };

    player.playScene( getId(), 2, NO_DEFAULT_CAMERA, callback );
  }

  void Scene00003( Entity::Player& player )
  {
    auto callback = [ & ]( Entity::Player& player, const Event::SceneResult& result )
    {
    };

    player.playScene( getId(), 3, NO_DEFAULT_CAMERA, callback );
  }

  void Scene00004( Entity::Player& player )
  {
    auto callback = [ & ]( Entity::Player& player, const Event::SceneResult& result )
    {
      if( result.param2 == 1 )
      {
        if( player.giveQuestRewards( getId(), 0 ) )
        {
          player.setQuestUI8AL( getId(), 0 );
          player.setQuestUI8BH( getId(), 0 );
          player.finishQuest( getId() );
        }
      }
    };

    player.playScene( getId(), 4, NONE, callback );
  }

  void Scene00097( Entity::Player& player )
  {
    auto callback = [ & ]( Entity::Player& player, const Event::SceneResult& result )
    {
    };

    player.playScene( getId(), 97, NONE, callback );
  }

  void Scene00098( Entity::Player& player )
  {
    auto callback = [ & ]( Entity::Player& player, const Event::SceneResult& result )
    {
      checkQuestCompletion( player );
      player.setQuestBitFlag8( getId(), 2, true );
    };

    player.playScene( getId(), 98, NONE, callback );
  }

  void Scene00099( Entity::Player& player )
  {
    auto callback = [ & ]( Entity::Player& player, const Event::SceneResult& result )
    {
    };

    player.playScene( getId(), 99, NONE, callback );
  }

  void Scene00100( Entity::Player& player )
  {
    auto callback = [ & ]( Entity::Player& player, const Event::SceneResult& result )
    {
      checkQuestCompletion( player );
      player.setQuestBitFlag8( getId(), 1, true );
    };

    player.playScene( getId(), 100, NONE, callback );
  }

};

EXPOSE_SCRIPT( SubFst000 );

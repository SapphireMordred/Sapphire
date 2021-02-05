#ifndef _PLAYER_H
#define _PLAYER_H

#include "ForwardsZone.h"

#include <Common.h>
#include <Util/SpawnIndexAllocator.h>
#include <spdlog/fmt/fmt.h>

#include "Chara.h"
#include "Event/EventHandler.h"
#include <map>
#include <queue>
#include <array>

namespace Sapphire::Entity
{

  struct QueuedZoning
  {
    uint16_t m_targetZone;
    Common::FFXIVARR_POSITION3 m_targetPosition;
    float m_targetRotation;
    uint64_t m_queueTime;

    QueuedZoning( uint16_t targetZone, const Common::FFXIVARR_POSITION3& targetPosition,
                  uint64_t queuedTime, float targetRotation ) :
      m_targetZone( targetZone ),
      m_targetPosition( targetPosition ),
      m_queueTime( queuedTime ),
      m_targetRotation( targetRotation )
    {
    }
  };

  struct ShopBuyBackEntry
  {
    ItemPtr item;
    uint32_t amount;
    uint32_t pricePerItem;
  };

  /** Class representing the Player
  *  Inheriting from Actor
  *
  */
  class Player : public Chara
  {
  public:
    /*! Contructor */
    Player();

    /*! Destructor */
    ~Player();

    void autoAttack( CharaPtr pTarget ) override;

    void injectPacket( const std::string& path );

    // EventHandlers
    //////////////////////////////////////////////////////////////////////////////////////////////////////
    /*! start an event action */
    void eventActionStart( uint32_t eventId, uint32_t action, World::Action::ActionCallback finishCallback,
                           World::Action::ActionCallback interruptCallback, uint64_t additional );

    /*! start an event item action */
    void eventItemActionStart( uint32_t eventId, uint32_t eventItemId, World::Action::EventItemActionCallback finishCallback,
                               World::Action::ActionCallback interruptCallback, uint64_t targetId );

    /*! start/register a normal event */
    void eventStart( uint64_t actorId, uint32_t eventId, Event::EventHandler::EventType eventParam, uint8_t eventParam1,
                     uint32_t eventParam2, Event::EventHandler::EventFinishCallback callback = nullptr );

    /*! play a subevent */
    void playScene( uint32_t eventId, uint32_t scene, uint32_t flags, uint32_t eventParam2, uint32_t eventParam3 );

    void playGilShop( uint32_t eventId, uint32_t flags, uint32_t param1,
                      Event::EventHandler::SceneReturnCallback eventCallback );

    void directorPlayScene( uint32_t eventId, uint32_t scene, uint32_t flags, uint32_t eventParam3,
                            uint32_t eventParam4, uint32_t eventParam5 = 0 );

    /*! play a subevent */
    void playScene( uint32_t eventId, uint32_t scene, uint32_t flags,
                    uint32_t eventParam2, uint32_t eventParam3,
                    Event::EventHandler::SceneReturnCallback eventReturnCallback );

    /*! play a subevent */
    void playScene( uint32_t eventId, uint32_t scene, uint32_t flags,
                    uint32_t eventParam2, uint32_t eventParam3, uint32_t eventParam4,
                    Event::EventHandler::SceneReturnCallback eventReturnCallback );

    /*! play a subevent */
    void playScene( uint32_t eventId, uint32_t scene, uint32_t flags,
                    Event::EventHandler::SceneReturnCallback eventReturnCallback );

    /*! play a subevent */
    void playScene( uint32_t eventId, uint32_t scene, uint32_t flags );

    /*! play a subevent */
    void playSceneChain( uint32_t eventId, uint32_t scene, uint32_t flags,
                         uint32_t eventParam2, uint32_t eventParam3, uint32_t eventParam4,
                         Event::EventHandler::SceneChainCallback sceneChainCallback );

    /*! play a subevent */
    void playSceneChain( uint32_t eventId, uint32_t scene, uint32_t flags,
                         uint32_t eventParam2, uint32_t eventParam3,
                         Event::EventHandler::SceneChainCallback sceneChainCallback );

    /*! play a subevent */
    void playSceneChain( uint32_t eventId, uint32_t scene, uint32_t flags,
                         Event::EventHandler::SceneChainCallback sceneChainCallback );

    /*! setup the event and return a ptr to it */
    Event::EventHandlerPtr bootstrapSceneEvent( uint32_t eventId, uint32_t flags );

    /*! finish / unregister an event */
    void eventFinish( uint32_t eventId, uint32_t freePlayer );

    /*! add an event to the event array */
    void addEvent( Event::EventHandlerPtr pEvent );

    /*! retrieve an event from the event array */
    Event::EventHandlerPtr getEvent( uint32_t eventId );

    /*! get number of active events */
    size_t getEventCount();

    /*! remove an event from the event array */
    void removeEvent( uint32_t eventId );

    /*! return the eventlist */
    std::map< uint32_t, Event::EventHandlerPtr >& eventList();

    void checkEvent( uint32_t eventId );



    // Events
    //////////////////////////////////////////////////////////////////////////////////////////////////////
    /*! Event to be called when zoning process gets triggered */
    void onZoneStart();

    /*! Event to be called when zoning process is finished */
    void onZoneDone();

    /*! Event to be called on login */
    void onLogin();

    /*! Event to be called on update tick */
    void onTick() override;

    /*! Event to be called upon player death */
    void onDeath() override;

    /*! Event called on every session iteration */
    void update( uint64_t tickCount ) override;

    /*! Event to be called upon Bnpc kill */
    void onMobKill( uint16_t nameId );


    // Quest
    //////////////////////////////////////////////////////////////////////////////////////////////////////
    /*! load data for currently active quests */
    bool loadActiveQuests();

    /*! update quest ( register it as active quest if new ) */
    void updateQuest( uint16_t questId, uint8_t sequence );

    /*! return true if quest is currently active */
    bool hasQuest( uint32_t questId );

    /*! return the current quest sequence */
    uint8_t getQuestSeq( uint16_t questId );

    /*! send the quest tracker packet */
    void sendQuestTracker();

    /*! set quest tracker flag for a specified slot */
    void setQuestTracker( uint16_t index, int16_t flag );

    /*! return the index of a given quest in the players quest list */
    int8_t getQuestIndex( uint16_t questId );

    /*! finish a given quest */
    void finishQuest( uint16_t questId );

    /*! finish a given quest */
    void unfinishQuest( uint16_t questId );

    /*! remove a given quest */
    void removeQuest( uint16_t questId );

    /*! add a quest to the completed quests mask */
    void updateQuestsCompleted( uint32_t questId );

    /*! remove a quest from the completed quest mask */
    void removeQuestsCompleted( uint32_t questId );

    /*! get the curent opening sequence */
    uint8_t getOpeningSequence() const;

    /*! set te current opening sequence */
    void setOpeningSequence( uint8_t seq );

    bool giveQuestRewards( uint32_t questId, uint32_t optionalChoice );

    std::shared_ptr< Common::QuestActive > getQuestActive( uint16_t index );

    uint8_t getQuestUI8A( uint16_t questId );

    uint8_t getQuestUI8B( uint16_t questId );

    uint8_t getQuestUI8C( uint16_t questId );

    uint8_t getQuestUI8D( uint16_t questId );

    uint8_t getQuestUI8E( uint16_t questId );

    uint8_t getQuestUI8F( uint16_t questId );

    uint8_t getQuestUI8AH( uint16_t questId );

    uint8_t getQuestUI8BH( uint16_t questId );

    uint8_t getQuestUI8CH( uint16_t questId );

    uint8_t getQuestUI8DH( uint16_t questId );

    uint8_t getQuestUI8EH( uint16_t questId );

    uint8_t getQuestUI8FH( uint16_t questId );

    uint8_t getQuestUI8AL( uint16_t questId );

    uint8_t getQuestUI8BL( uint16_t questId );

    uint8_t getQuestUI8CL( uint16_t questId );

    uint8_t getQuestUI8DL( uint16_t questId );

    uint8_t getQuestUI8EL( uint16_t questId );

    uint8_t getQuestUI8FL( uint16_t questId );

    uint16_t getQuestUI16A( uint16_t questId );

    uint16_t getQuestUI16B( uint16_t questId );

    uint16_t getQuestUI16C( uint16_t questId );

    uint32_t getQuestUI32A( uint16_t questId );

    bool getQuestBitFlag8( uint16_t questId, uint8_t index );

    bool getQuestBitFlag16( uint16_t questId, uint8_t index );

    bool getQuestBitFlag24( uint16_t questId, uint8_t index );

    bool getQuestBitFlag32( uint16_t questId, uint8_t index );

    bool getQuestBitFlag40( uint16_t questId, uint8_t index );

    bool getQuestBitFlag48( uint16_t questId, uint8_t index );


    void setQuestUI8A( uint16_t questId, uint8_t val );

    void setQuestUI8B( uint16_t questId, uint8_t val );

    void setQuestUI8C( uint16_t questId, uint8_t val );

    void setQuestUI8D( uint16_t questId, uint8_t val );

    void setQuestUI8E( uint16_t questId, uint8_t val );

    void setQuestUI8F( uint16_t questId, uint8_t val );

    void setQuestUI8AH( uint16_t questId, uint8_t val );

    void setQuestUI8BH( uint16_t questId, uint8_t val );

    void setQuestUI8CH( uint16_t questId, uint8_t val );

    void setQuestUI8DH( uint16_t questId, uint8_t val );

    void setQuestUI8EH( uint16_t questId, uint8_t val );

    void setQuestUI8FH( uint16_t questId, uint8_t val );

    void setQuestUI8AL( uint16_t questId, uint8_t val );

    void setQuestUI8BL( uint16_t questId, uint8_t val );

    void setQuestUI8CL( uint16_t questId, uint8_t val );

    void setQuestUI8DL( uint16_t questId, uint8_t val );

    void setQuestUI8EL( uint16_t questId, uint8_t val );

    void setQuestUI8FL( uint16_t questId, uint8_t val );

    void setQuestUI16A( uint16_t questId, uint16_t val );

    void setQuestUI16B( uint16_t questId, uint16_t val );

    void setQuestUI16C( uint16_t questId, uint16_t val );

    void setQuestUI32A( uint16_t questId, uint32_t val );

    void setQuestBitFlag8( uint16_t questId, uint8_t index, bool val );

    void setQuestBitFlag16( uint16_t questId, uint8_t index, bool val );

    void setQuestBitFlag24( uint16_t questId, uint8_t index, bool val );

    void setQuestBitFlag32( uint16_t questId, uint8_t index, bool val );

    void setQuestBitFlag40( uint16_t questId, uint8_t index, bool val );

    void setQuestBitFlag48( uint16_t questId, uint8_t index, bool val );

    // Inventory / Item / Currency
    //////////////////////////////////////////////////////////////////////////////////////////////////////
    /*! equip an item to a specified slot */
    void equipItem( Common::GearSetSlot equipSlotId, ItemPtr pItem, bool sendModel );

    /*! remove an item from an equipment slot */
    void unequipItem( Common::GearSetSlot equipSlotId, ItemPtr pItem, bool sendModel );

    /*! equip a weapon, possibly forcing a job change */
    void equipWeapon( ItemPtr pItem, bool updateClass );

    /*! equip a soul crystal, possibly forcing a job change*/
    void equipSoulCrystal( ItemPtr pItem, bool updateClass );

    /*! unequip a soul crystal, returning to the base class*/
    void unequipSoulCrystal( ItemPtr pItem );

    /*! get player ilvl */
    uint16_t getItemLevel() const;

    /*! send player ilvl */
    void sendItemLevel();

    /*! get the current main hand model */
    uint64_t getModelMainWeapon() const;

    /*! get the current off hand model */
    uint64_t getModelSubWeapon() const;

    /*! get the current system hand model */
    uint64_t getModelSystemWeapon() const;

    /*! return a const pointer to the model array */
    const uint32_t* getModelArray() const;

    /*! return the equipment model in a specified equipment slot */
    uint32_t getModelForSlot( Common::GearModelSlot slot );

    /*! add amount to the currency of type */
    void addCurrency( Common::CurrencyType type, uint32_t amount, bool sendLootMessage = false );

    /*! remove amount from the currency of type */
    void removeCurrency( Common::CurrencyType type, uint32_t amount );

    /*! return the current amount of crystals of type */
    uint32_t getCrystal( uint8_t type ) const;

    void updateModels( Common::GearSetSlot equipSlotId, const Sapphire::ItemPtr& pItem, bool updateClass );

    Common::GearModelSlot equipSlotToModelSlot( Common::GearSetSlot slot );

    bool getFreeInventoryContainerSlot( Inventory::InventoryContainerPair& containerPair ) const;

    void insertInventoryItem( Common::InventoryType type, uint16_t slot, const Sapphire::ItemPtr item );

    /*!
    * Collect real item handins from container
    * @param itemIds a vector of each catalog id to collect
    * @return true if all items were handed in
    */
    bool collectHandInItems( std::vector< uint32_t > itemIds );

    // Class / Job / Exp
    //////////////////////////////////////////////////////////////////////////////////////////////////////
    /*! returns the level of the currently active class / job */
    uint8_t getLevel() const override;

    /*! returns the level of the provided class / job */
    uint8_t getLevelForClass( Common::ClassJob pClass ) const;

    /*! returns if the classjob is unlocked */
    bool isClassJobUnlocked( Common::ClassJob classJob ) const;

    /*! returns the exp of the currently active class / job */
    uint32_t getExp() const;

    /*! sets the exp of the currently active class / job */
    void setExp( uint32_t amount );

    /*! adds exp to the currently active class / job */
    void gainExp( uint32_t amount );

    /*! gain a level on the currently active class / job */
    void gainLevel();

    /*! set level on the currently active class / job to given level */
    void setLevel( uint8_t level );

    /*! set level on the provided class / job to given level */
    void setLevelForClass( uint8_t level, Common::ClassJob classjob );

    /*! change class or job to given class / job */
    void setClassJob( Common::ClassJob classJob );

    /*! returns a pointer to the class array */
    uint16_t* getClassArray();

    /*! returns a const pointer to the class array */
    const uint16_t* getClassArray() const;

    /*! returns a pointer to the exp array */
    uint32_t* getExpArray();

    /*! returns a const pointer to the exp array */
    const uint32_t* getExpArray() const;

    // Base Look / Stats / Params
    //////////////////////////////////////////////////////////////////////////////////////////////////////
    /*! return the birth day */
    uint8_t getBirthDay() const;

    /*! return the birth month */
    uint8_t getBirthMonth() const;

    /*! return the guardian diety Id */
    uint8_t getGuardianDeity() const;

    /*! get look at specified index */
    uint8_t getLookAt( uint8_t index ) const;

    /*! return the race */
    uint8_t getRace() const;

    /*! return gender 0 male, 1 female */
    uint8_t getGender() const;

    /*! return the id of the home town */
    uint8_t getStartTown() const;

    /*! return the voice id */
    uint8_t getVoiceId() const;

    /*! return the grand company */
    uint8_t getGc() const;

    /*! return the grand company rank */
    const uint8_t* getGcRankArray() const;

    /*! set look at index */
    void setLookAt( uint8_t index, uint8_t value );

    /*! set the voice Id */
    void setVoiceId( uint8_t voiceId );

    /*! set the grand company */
    void setGc( uint8_t gc );

    /*! set the grand company rank */
    void setGcRankAt( uint8_t index, uint8_t rank );

    /*! returns true if the player is currently in combat */
    bool isInCombat() const;

    /*! sets players combat state */
    void setInCombat( bool mode );

    /*! return current online status depending on current state / activity */
    Common::OnlineStatus getOnlineStatus() const;

    /*! sets the players zone, initiating a zoning process */
    void setZone( uint32_t zoneId );

    /*! sets the players instance & initiates zoning process */
    bool setInstance( uint32_t instanceContentId );

    /*! sets the players instance & initiates zoning process */
    bool setInstance( TerritoryPtr instance );

    /*! sets the players instance & initiates zoning process */
    bool setInstance( Sapphire::TerritoryPtr instance, Sapphire::Common::FFXIVARR_POSITION3 pos );

    /*! returns the player to their position before zoning into an instance */
    bool exitInstance();

    /*! sets the players territoryTypeId */
    void setTerritoryTypeId( uint32_t territoryTypeId );

    /*! gets the players territoryTypeId */
    uint32_t getTerritoryTypeId() const;

    void forceZoneing( uint32_t zoneId );

    /*! return player to preset homepoint */
    void returnToHomepoint();

    /*! change position, sends update too */
    void changePosition( float x, float y, float z, float o );

    /*! return the contentId */
    uint64_t getContentId() const;

    /*! return max hp */
    uint32_t getMaxHp();

    /*! return max mp */
    uint32_t getMaxMp();

    /*! return a players total play time */
    uint32_t getPlayTime() const;

    /*! return true if the player has "new adventurere" status */
    bool isNewAdventurer() const;

    /*! change the players "new adventurere" status */
    void setNewAdventurer( bool state );

    /*! sets the list of current online status */
    void setOnlineStatusMask( uint64_t status );

    /*! returns the current online status */
    uint64_t getOnlineStatusMask() const;

    /*! perform a teleport of a specified type ( teleport,return,aethernet ) */
    void teleport( uint16_t aetheryteId, uint8_t type = 1 );

    /*! query teleport of a specified type */
    void teleportQuery( uint16_t aetheryteId );

    Common::PlayerTeleportQuery getTeleportQuery() const;

    void clearTeleportQuery();

    void setDyeingInfo( uint32_t itemToDyeContainer, uint32_t itemToDyeSlot, uint32_t dyeBagContainer, uint32_t dyeBagSlot );
    void dyeItemFromDyeingInfo();

    /*! prepares zoning / fades out the screen */
    void prepareZoning( uint16_t targetZone, bool fadeOut, uint8_t fadeOutTime = 0, uint16_t animation = 0 );

    /*! get player's title list (available titles) */
    uint8_t* getTitleList();

    const uint8_t* getTitleList() const;

    /*! get player's active title */
    uint16_t getTitle() const;

    /*! add title to player title list */
    void addTitle( uint16_t titleId );

    /*! change player's active title */
    void setTitle( uint16_t titleId );

    /*! send the players title list */
    void sendTitleList();

    /*! change gear param state */
    void setEquipDisplayFlags( uint8_t state );

    /*! get gear param state */
    uint8_t getEquipDisplayFlags() const;

    /*! mount the specified mount and send the packets */
    void mount( uint32_t id );

    /*! dismount the current mount and send the packets */
    void dismount();

    void spawnCompanion( uint16_t id );
    void despawnCompanion();

    uint16_t getCurrentCompanion() const;

    /*! get the current mount */
    uint8_t getCurrentMount() const;

    /*! set current persistent emote */
    void setPersistentEmote( uint32_t emoteId );

    /*! get current persistent emote */
    uint32_t getPersistentEmote() const;

    void calculateStats() override;

    void sendStats();


    // Aetheryte / Action / Attribute bitmasks
    //////////////////////////////////////////////////////////////////////////////////////////////////////
    /*! register aetheryte aetheryteId and send update */
    void registerAetheryte( uint8_t aetheryteId );

    /*! check if aetheryte is already registered */
    bool isAetheryteRegistered( uint8_t aetheryteId ) const;

    /*! return a const pointer to the aetheryte unlock bitmask array */
    int8_t getAetheryteMaskAt( uint8_t index ) const;

    /*! return a pointer to the aetheryte unlock bitmask array */
    uint8_t* getAetheryteArray();

    /*! set homepoint */
    void setHomepoint( uint8_t aetheryteId );

    /*! get homepoint */
    uint8_t getHomepoint() const;

    /*! discover subarea subid fo map map_id, also send udpate packet */
    void discover( int16_t map_id, int16_t sub_id );

    /*! return a pointer to the discovery bitmask array */
    uint8_t* getDiscoveryBitmask();

    /*! helper/debug function to reset all discovered areas */
    void resetDiscovery();

    /*! get a pointer to the howto bitmask array */
    uint8_t* getHowToArray();

    /*! get a const pointer to the howto bitmask array */
    const uint8_t* getHowToArray() const;

    /*! update bitmask for how-to's seen */
    void updateHowtosSeen( uint32_t howToId );

    /*! learn an action / update the unlock bitmask. */
    void learnAction( uint16_t actionId );

    /*! learn a song / update the unlock bitmask. */
    void learnSong( uint8_t songId, uint32_t itemId );

    /*! check if an action is already unlocked in the bitmask. */
    bool isActionLearned( uint8_t actionId ) const;

    /*! return a const pointer to the unlock bitmask array */
    const uint8_t* getUnlockBitmask() const;

    /*! return a const pointer to the orchestrion bitmask array */
    const uint8_t* getOrchestrionBitmask() const;

    /*! return a const pointer to the mount guide bitmask array */
    const uint8_t* getMountGuideBitmask() const;

    bool checkAction() override;

    bool hasQueuedAction() const;

    void setQueuedAction( World::Action::ActionPtr pAction );

    // Spawn handling
    //////////////////////////////////////////////////////////////////////////////////////////////////////
    /*! initialize the spawnId queue */
    void initSpawnIdQueue();

    /*! get the spawn id mapped to a specific actorId */
    uint8_t getSpawnIdForActorId( uint32_t actorId );

    /*! frees the spawnId assigned to the given actor */
    void freePlayerSpawnId( uint32_t actorId );

    /*! checks if the given spawn id is valid */
    bool isActorSpawnIdValid( uint8_t spawnId );

    /*! send spawn packets to pTarget */
    void spawn( PlayerPtr pTarget ) override;

    /*! send despawn packets to pTarget */
    void despawn( PlayerPtr pTarget ) override;

    // Player State Handling
    //////////////////////////////////////////////////////////////////////////////////////////////////////
    /* return a const pointer to the state flag array */
    const uint8_t* getStateFlags() const;

    /* set a specified state flag */
    void setStateFlag( Common::PlayerStateFlag flag );

    /* set a specified state flag */
    void setStateFlags( std::vector< Common::PlayerStateFlag > flags );

    /* check if a specified flag is set */
    bool hasStateFlag( Common::PlayerStateFlag flag ) const;

    /* reset a specified flag */
    void unsetStateFlag( Common::PlayerStateFlag flag );

    // Player Session Handling
    //////////////////////////////////////////////////////////////////////////////////////////////////////
    /*! return the userlevel */
    uint8_t getUserLevel() const;

    /*! set timestamp for last received ping */
    void setLastPing( uint32_t ping );

    /*! get timestamp of last received ping */
    uint32_t getLastPing() const;

    // Player Database Handling
    //////////////////////////////////////////////////////////////////////////////////////////////////////
    /*! generate the update sql based on update flags */
    void updateSql();

    /*! load player from db, by id */
    bool load( uint32_t charId, World::SessionPtr pSession );

    /*! load active class data */
    bool loadClassData();

    /*! load search info */
    bool loadSearchInfo();

    /*! load hunting log entries */
    bool loadHuntingLog();

    // Player Network Handling
    //////////////////////////////////////////////////////////////////////////////////////////////////////
    /*! send current models ( equipment ) */
    void sendModel();

    /*! send active state flags */
    void sendStateFlags();

    /*! send status update */
    void sendStatusUpdate() override;

    /*! send the entire inventory sequence */
    void sendInventory();

    /*! send active quest list */
    void sendQuestInfo();

    /*! send a quest specific message */
    void sendQuestMessage( uint32_t questId, int8_t msgId, uint8_t type, uint32_t var1, uint32_t var2 );

    /*! queue a packet for the player */
    void queuePacket( Network::Packets::FFXIVPacketBasePtr pPacket );

    /*! queue a char connection packet for the player */
    void queueChatPacket( Network::Packets::FFXIVPacketBasePtr pPacket );

    /*! returns true if loading is complete ( 0x69 has been received ) */
    bool isLoadingComplete() const;

    /*! set the loading complete bool */
    void setLoadingComplete( bool bComplete );

    /*! mark this player for zoning, notify worldserver */
    void performZoning( uint16_t zoneId, const Common::FFXIVARR_POSITION3& pos, float rotation );

    /*! return true if the player is marked for zoning */
    bool isMarkedForZoning() const;

    void emote( uint32_t emoteId, uint64_t targetId, bool isSilent );

    void emoteInterrupt();


    void sendZoneInPackets( uint32_t param1, uint32_t param2, uint32_t param3, uint32_t param4, bool pSetStatus );

    void finishZoning();

    void sendZonePackets();

    Common::ZoneingType getZoningType() const;

    void setZoningType( Common::ZoneingType zoneingType );

    void setSearchInfo( uint8_t selectRegion, uint8_t selectClass, const char* searchMessage );

    const char* getSearchMessage() const;

    uint8_t getSearchSelectRegion() const;

    uint8_t getSearchSelectClass() const;

    void sendNotice( const std::string& message );
    template< typename... Args >
    void sendNotice( const std::string& message, const Args&... args )
    {
      sendNotice( fmt::format( message, args... ) );
    }

    void sendUrgent( const std::string& message );
    template< typename... Args >
    void sendUrgent( const std::string& message, const Args&... args )
    {
      sendUrgent( fmt::format( message, args... ) );
    }

    void sendDebug( const std::string& message );

    template< typename... Args >
    void sendDebug( const std::string& message, const Args&... args )
    {
      sendDebug( fmt::format( message, args... ) );
    }

    void sendLogMessage( uint32_t messageId, uint32_t param2 = 0, uint32_t param3 = 0, uint32_t param4 = 0, uint32_t param5 = 0, uint32_t param6 = 0 );

    bool isDirectorInitialized() const;

    void setDirectorInitialized( bool isInitialized );

    // Housing Handling
    //////////////////////////////////////////////////////////////////////////////////////////////////////
    void setLandFlags( uint8_t permissionSet, uint32_t landFlags, Common::LandIdent ident );

    void sendLandFlags();
    void sendLandFlagsSlot( Common::LandFlagsSlot slot );

    // Player Battle Handling
    //////////////////////////////////////////////////////////////////////////////////////////////////////
    void initHateSlotQueue();

    void hateListAdd( BNpcPtr pBNpc );
    void hateListRemove( BNpcPtr pBNpc );

    bool hateListHasEntry( BNpcPtr pBNpc );

    void sendHateList();

    bool actionHasCastTime( uint32_t actionId );

    Sapphire::Entity::ActorPtr lookupTargetById( uint64_t targetId );

    bool isLogin() const;

    void setIsLogin( bool bIsLogin );

    uint16_t getZoneId() const;

    uint32_t getTerritoryId() const;
    void setTerritoryId( uint32_t territoryId );

    uint8_t getGmRank() const;

    void setGmRank( uint8_t rank );

    bool getGmInvis() const;

    void setGmInvis( bool invis );

    bool isActingAsGm() const;

    uint8_t getMode() const;

    void setMode( uint8_t mode );

    void setAutoattack( bool mode );

    bool isAutoattackOn() const;

    void onMobAggro( BNpcPtr pBNpc );
    void onMobDeaggro( BNpcPtr pBNpc );

    // Content Finder handling
    //////////////////////////////////////////////////////////////////////////////////////////////////////
    /*! Get an unix time when the player can register into content finder again. */
    uint32_t getCFPenaltyTimestamp() const;

    /*! Set an unix time when the player can register into content finder again. */
    void setCFPenaltyTimestamp( uint32_t timestamp );

    uint32_t getCFPenaltyMinutes() const;

    void setCFPenaltyMinutes( uint32_t minutes );

    void setEorzeaTimeOffset( uint64_t timestamp );

    // Database
    void updateDbAllQuests() const;

    void deleteQuest( uint16_t questId ) const;

    void insertQuest( uint16_t questId, uint8_t index, uint8_t seq ) const;

    void updateDbSearchInfo() const;

    void updateDbClass() const;

    void insertDbClass( const uint8_t classJobIndex ) const;

    void setMarkedForRemoval();

    bool isMarkedForRemoval() const;

    void setOnEnterEventDone( bool isDone );

    bool isOnEnterEventDone() const;

    /*! gets the next available obj count */
    uint8_t getNextObjSpawnIndexForActorId( uint32_t actorId );

    /*! resets the players obj count */
    void resetObjSpawnIndex();

    /*! frees an obj count to be used by another eobj */
    void freeObjSpawnIndexForActorId( uint32_t actorId );

    /*! checks if a spawn index is valid */
    bool isObjSpawnIndexValid( uint8_t index );


    // Inventory Handling
    //////////////////////////////////////////////////////////////////////////////////////////////////////
    void initInventory();

    using InvSlotPair = std::pair< uint16_t, int8_t >;
    using InvSlotPairVec = std::vector< InvSlotPair >;

    bool loadInventory();

    InvSlotPairVec getSlotsOfItemsInInventory( uint32_t catalogId );

    InvSlotPair getFreeBagSlot();

    ItemPtr addItem( uint32_t catalogId, uint32_t quantity = 1, bool isHq = false, bool silent = false, bool canMerge = true, bool sendLootMessage = false );
    ItemPtr addItem( ItemPtr itemToAdd, bool silent = false, bool canMerge = true, bool sendLootMessage = false );

    void moveItem( uint16_t fromInventoryId, uint8_t fromSlotId, uint16_t toInventoryId, uint8_t toSlot );

    void swapItem( uint16_t fromInventoryId, uint8_t fromSlotId, uint16_t toInventoryId, uint8_t toSlot );

    void discardItem( uint16_t fromInventoryId, uint8_t fromSlotId );

    void splitItem( uint16_t fromInventoryId, uint8_t fromSlotId, uint16_t toInventoryId, uint8_t toSlot,
                    uint16_t splitCount );

    void mergeItem( uint16_t fromInventoryId, uint8_t fromSlotId, uint16_t toInventoryId, uint8_t toSlot );

    ItemPtr getItemAt( uint16_t containerId, uint8_t slotId );

    bool updateContainer( uint16_t storageId, uint8_t slotId, ItemPtr pItem );

    /*! calculate and return player ilvl based off equipped gear */
    uint16_t calculateEquippedGearItemLevel();

    ItemPtr getEquippedWeapon();

    /*! return the current amount of currency of type */
    uint32_t getCurrency( Common::CurrencyType type );

    void writeInventory( Common::InventoryType type );

    ItemPtr createTempItem( uint32_t catalogId, uint32_t quantity = 1 );

    void updateItemDb( ItemPtr pItem ) const;

    void writeItemDb( ItemPtr pItem ) const;

    void deleteItemDb( ItemPtr pItem ) const;

    /*! return the crystal amount of currency of type */
    uint32_t getCrystal( Common::CrystalType type );

    /*! add amount to the crystal of type */
    void addCrystal( Common::CrystalType type, uint32_t amount, bool sendLootMessage = false );

    /*! remove amount from the crystals of type */
    void removeCrystal( Common::CrystalType type, uint32_t amount );

    bool isObtainable( uint32_t catalogId, uint8_t quantity );

    uint32_t getNextInventorySequence();

    bool findFirstItemWithId( uint32_t catalogId, Inventory::InventoryContainerPair& location );

    uint8_t getFreeSlotsInBags();

    void setActiveLand( uint8_t land, uint8_t ward );
    Common::ActiveLand getActiveLand() const;

    Sapphire::ItemPtr dropInventoryItem( Common::InventoryType type, uint16_t slotId, bool silent = false );

    // Job UI
    //////////////////////////////////////////////////////////////////////////////////////////////////////
    void gaugeClear();
    void sendActorGauge();
    void gaugeSetRaw( uint8_t* pData );

    //////////////////////////////////////////////////////////////////////////////////////////////////////

    Common::HuntingLogEntry& getHuntingLogEntry( uint8_t index );

    void sendHuntingLog();

    void updateDbMonsterNote();

    void updateHuntingLog( uint16_t id );

    World::SessionPtr getSession();

    uint64_t m_lastMoveTime;
    uint8_t m_lastMoveflag;
    bool m_falling;

    std::vector< ShopBuyBackEntry >& getBuyBackListForShop( uint32_t shopId );
    void addBuyBackItemForShop( uint32_t shopId, const ShopBuyBackEntry& entry );
    void clearBuyBackMap();

  private:
    uint32_t m_lastWrite;
    uint32_t m_lastPing;

    World::SessionPtr m_pSession;

    bool m_bIsLogin;

    uint64_t m_contentId; // This id will be the name of the folder for character settings in "My Games"

    uint8_t m_mode;

    bool m_markedForRemoval;

    bool m_directorInitialized;

    bool m_onEnterEventDone;

    uint32_t m_inventorySequence;

    World::Action::ActionPtr m_pQueuedAction;

  private:
    using InventoryMap = std::map< uint16_t, Sapphire::ItemContainerPtr >;

    InventoryMap m_storageMap;

    Common::FFXIVARR_POSITION3 m_prevPos;
    uint32_t m_prevTerritoryTypeId;
    uint32_t m_prevTerritoryId;
    float m_prevRot;

    uint8_t m_voice;

    uint64_t m_modelMainWeapon;
    uint64_t m_modelSubWeapon;
    uint64_t m_modelSystemWeapon;

    bool m_bNewGame;

    uint8_t m_guardianDeity;
    uint8_t m_birthDay;
    uint8_t m_birthMonth;

    struct RetainerInfo
    {
      uint32_t retainerId;
      char retainerName[32];
      uint32_t createUnixTime;
      bool isActive;
      bool isRename;
      uint8_t status;
    } m_retainerInfo[8];

    uint16_t m_activeTitle;
    uint8_t m_titleList[48];
    uint8_t m_howTo[34];
    uint8_t m_minions[40];
    uint8_t m_mountGuide[22];
    uint8_t m_homePoint;
    uint8_t m_startTown;
    uint16_t m_townWarpFstFlags;
    uint8_t m_questCompleteFlags[487];
    uint8_t m_discovery[445];
    uint32_t m_playTime;

    uint16_t m_classArray[28];
    uint32_t m_expArray[28];
    uint8_t m_aetheryte[21];
    uint8_t m_unlocks[64];
    uint8_t m_orchestrion[40];

    uint8_t m_openingSequence;

    uint16_t m_itemLevel;
    std::map< uint32_t, Event::EventHandlerPtr > m_eventHandlerMap;

    std::queue< uint8_t > m_freeHateSlotQueue; // queue with "hate slots" free to be assigned
    std::map< uint32_t, uint8_t > m_actorIdTohateSlotMap;

    std::map< uint32_t, uint8_t > m_questIdToQuestIdx; // quest mapping, quest id to quest container index
    std::map< uint8_t, uint32_t > m_questIdxToQuestId; // quest mapping, quest container index to questId
    std::shared_ptr< Common::QuestActive > m_activeQuests[30];
    int16_t m_questTracking[5];

    uint8_t m_stateFlags[12];
    uint8_t m_gmRank;
    uint16_t zoneId;
    bool m_gmInvis = false;

    uint8_t m_equipDisplayFlags;

    bool m_bInCombat;
    bool m_bLoadingComplete;
    bool m_bAutoattack;

    Common::JobGauge m_gauge;

    Common::ZoneingType m_zoningType;
    uint32_t m_territoryId;

    bool m_bMarkedForZoning;
    bool m_bNewAdventurer;
    uint64_t m_onlineStatus;
    std::shared_ptr< QueuedZoning > m_queuedZoneing;

    // search info
    char m_searchMessage[193]; // searchmessage to show in profile
    uint8_t m_searchSelectRegion; // regions selected to show up in profile
    uint8_t m_searchSelectClass; // class selected to show up in profile

    // housing info
    Common::LandFlagSet m_landFlags[5];

    Common::ActiveLand m_activeLand;

    // gc info
    uint8_t m_gc;
    uint8_t m_gcRank[3];

    // content finder info
    uint32_t m_cfPenaltyUntil; // unix time

    uint16_t m_companionId;
    uint32_t m_mount;
    uint32_t m_emoteMode;

    Common::PlayerTeleportQuery m_teleportQuery;

    struct PlayerDyeingInfo
    {
      uint32_t itemToDyeContainer;
      uint32_t itemToDyeSlot;
      uint32_t dyeBagContainer;
      uint32_t dyeBagSlot;
    } m_dyeingInfo;

    Common::Util::SpawnIndexAllocator< uint8_t > m_objSpawnIndexAllocator;
    Common::Util::SpawnIndexAllocator< uint8_t > m_actorSpawnIndexAllocator;

    std::array< Common::HuntingLogEntry, 12 > m_huntingLogEntries;
    std::unordered_map< uint32_t, std::vector< ShopBuyBackEntry > > m_shopBuyBackMap;
  };

}

#endif

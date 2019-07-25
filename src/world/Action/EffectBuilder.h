#ifndef SAPPHIRE_EFFECTBUILDER_H
#define SAPPHIRE_EFFECTBUILDER_H

#include <ForwardsZone.h>
#include <Common.h>

namespace Sapphire::World::Action
{
  class EffectBuilder
  {
  public:
    enum HitSeverity
    {
      Normal,
      Critical,
      DirectHit,
      CriticalDirectHit
    };

    EffectBuilder( Entity::CharaPtr source, uint32_t actionId );


    void healTarget( Entity::CharaPtr& target, uint32_t amount,
                     Common::ActionHitSeverityType severity = Common::ActionHitSeverityType::NormalHeal );

    void damageTarget( Entity::CharaPtr& target, uint32_t amount,
                       Common::ActionHitSeverityType severity = Common::ActionHitSeverityType::NormalDamage );

    void buildAndSendPackets();


  private:
    EffectResultPtr getResult( Entity::CharaPtr& chara );

    uint32_t getResultDelayMs();


    Entity::CharaPtr m_sourceChara;

    uint32_t m_actionId;

    std::unordered_map< uint32_t, EffectResultPtr > m_resolvedEffects;
  };

}

#endif //SAPPHIRE_EFFECTBUILDER_H
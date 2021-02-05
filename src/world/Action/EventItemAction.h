#ifndef _EVENTITEMACTION_H_
#define _EVENTITEMACTION_H_

#include <Common.h>

#include "ForwardsZone.h"
#include "Action.h"

namespace Sapphire::World::Action
{

class EventItemAction : public Action
{

public:
  virtual ~EventItemAction();

  EventItemAction( Entity::CharaPtr pActor, uint32_t eventId, uint32_t eventItemId,
                   EventItemActionCallback finishRef, ActionCallback interruptRef, uint64_t targetId );

  void start() override;

  void execute() override;

  void interrupt() override;

private:
  uint32_t m_eventId;
  uint64_t m_targetId;

  EventItemActionCallback m_onActionFinishClb;
  ActionCallback m_onActionInterruptClb;

};

}

#endif

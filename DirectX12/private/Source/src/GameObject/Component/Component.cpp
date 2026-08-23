#include "GameObject/Component/Component.h"
#include "GameObject/Actor.h"

Component::Component(Actor& owner, int updateOrder)
	: mOwner(owner),
	mUpdateOrder(updateOrder)
{
}

#include "PCH.h"

#include "ecs/system/GameSystem.h"
#include "ecs/ECS_Utils.h"

#include "ecs/comp/CollisionComponent.h"

namespace cs
{
	GameSystem::GameSystem(ECSContext* cxt)
		: BaseSystem(cxt)
		, player(nullptr)
		, lastComponent(nullptr)
	{
		this->subscribeForComponent<GameComponent>(this->parentContext);
		this->subscribeForComponent<CollisionComponent>(this->parentContext);
	}

	GameSystem::~GameSystem()
	{
        this->removeComponentSubscription<GameComponent>(this->parentContext);
        this->removeComponentSubscription<CollisionComponent>(this->parentContext);
	}

	void GameSystem::setPlayer(EntityPtr& actor, LuaCallbackPtr enterCollision, LuaCallbackPtr exitCollision)
	{
		if (actor.get())
		{
			log::info("Setting player ", actor->getName());
			this->player = actor;
			this->onEnterCollision = enterCollision;
			this->onExitCollision = exitCollision;
		}
	}

	void GameSystem::clearPlayer()
	{
		if (this->player.get())
		{
			log::info("Clearing player ", this->player->getName());
			this->player = nullptr;
			this->onEnterCollision = nullptr;
			this->onExitCollision = nullptr;
		}
	}

	void GameSystem::reset()
	{
		this->clearPlayer();
	}

	void GameSystem::processImpl(SystemUpdateParams* params)
	{
		if (!this->player.get())
			return;

		vec3 worldPos = this->player->getWorldPosition();
		this->testPoint(worldPos);

		BaseSystem::ComponentIdMap volumes;
		this->getEnabledComponents<CollisionComponent>(volumes);
		for (auto& it : volumes)
		{
			CollisionComponentPtr collision =
				std::static_pointer_cast<CollisionComponent>(it.second);

			if (collision->getNotifyOnMoved() && !collision->getHasMoved())
			{
				// notify that we've been touched
				Entity* parent = collision->getParent();
				if (parent && parent->hasMoved())
				{
					collision->onMoved();
					collision->setHasMoved();
				}
			}
		}
	}


	bool GameSystem::testInternal(const vec3& pos, const CollisionComponentPtr& collision)
	{
		vec3 adjPos = vec3(collision->getInverseMatrix() * vec4(pos, 1.0f));
		return collision->test(adjPos);
	}

	void GameSystem::testPoint(const vec3& pos)
	{
		if (this->lastComponent.get())
		{
			if (!GameSystem::testInternal(pos, this->lastComponent))
			{
				if (this->onExitCollision)
				{
					(*this->onExitCollision)(this->lastComponent->getParent());
				}
				this->lastComponent = nullptr;
			}
			else
			{
				return;
			}
		}

		BaseSystem::ComponentIdMap volumes;
		this->getEnabledComponents<CollisionComponent>(volumes);
		for (const auto& it : volumes)
		{
			
			const CollisionComponentPtr& collision =
				std::static_pointer_cast<CollisionComponent>(it.second);
			
			// put position into object space
			if (GameSystem::testInternal(pos, collision))
			{
				this->lastComponent = collision;
				if (this->onEnterCollision)
				{
					(*this->onEnterCollision)(collision->getParent());
				}
			}

		}
	}
}

#include "PCH.h"

#include "ecs/comp/ComponentList.h"
#include "ecs/comp/ComponentHash.h"

namespace cs
{
	std::unordered_map<size_t, uint32> gComponentHashMap;

	template <class T>
	static void addComponentHash(uint32 adjusted_index)
	{
		std::type_index index(typeid(T));
		gComponentHashMap[index.hash_code()] = adjusted_index;
	}

	void initComponentHash()
	{
		/*
		gComponentHashMap[std::type_index(typeid(DrawableComponent)).hash_code()]	= 588440388;
		gComponentHashMap[std::type_index(typeid(PhysicsComponent)).hash_code()]	= 2910804559;
		gComponentHashMap[std::type_index(typeid(ScriptComponent)).hash_code()]		= 4106753787;
		gComponentHashMap[std::type_index(typeid(AnimationComponent)).hash_code()]	= 3074921166;
		gComponentHashMap[std::type_index(typeid(ParticleComponent)).hash_code()]	= 1065994208;
		gComponentHashMap[std::type_index(typeid(GameComponent)).hash_code()]		= 1594589860;
		gComponentHashMap[std::type_index(typeid(LiquidComponent)).hash_code()]		= 3229050158;
		gComponentHashMap[std::type_index(typeid(CollisionComponent)).hash_code()]	= 931596328;
		*/

		log::info(std::type_index(typeid(AudioComponent)).hash_code());


		addComponentHash<DrawableComponent>(588440388);
		addComponentHash<PhysicsComponent>(2910804559);
		addComponentHash<ScriptComponent>(4106753787);
		addComponentHash<AnimationComponent>(3074921166);
		addComponentHash<ParticleComponent>(1065994208);
		addComponentHash<GameComponent>(1594589860);
		addComponentHash<LiquidComponent>(3229050158);
		addComponentHash<CollisionComponent>(931596328);
		addComponentHash<AudioComponent>(896335202);

        /*
		log::info("gComponentHashMap Initialized!");
		for (auto& it : gComponentHashMap)
		{
			log::info(it.first);
		}
        */
	}

	uint32 getComponentHash(const std::type_index& index)
	{
		size_t hash_index = index.hash_code();
		return getComponentHash(hash_index);
	}

	uint32 getComponentHash(size_t hash_index)
	{
		std::unordered_map<size_t, uint32>::iterator it = gComponentHashMap.find(hash_index);
		if (it != gComponentHashMap.end())
		{
			return it->second;
		}
		log::error("Missing key for Component ", hash_index);
		assert(false);

		return 0;
	}
}

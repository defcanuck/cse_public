#pragma once

#include "ClassDef.h"

#include "scene/ReferenceNode.h"
#include "scene/SceneData.h"

namespace cs
{
	CLASS_DEFINITION(SceneCreator)
	public:

		SceneCreator();
		SceneCreator(SceneDataPtr& data);
		SceneCreator(SceneDataPtr& data, EntityPtr& p);
		SceneCreator(SceneSelectedPtr& selected);

		std::string name;
		std::string resource_name;
		
		vec3 position;
		int lightIndex;

		LoadFlagMask loadFlags;
		SceneCreateComponentMask createMask;

		static EntityPtr createReferenceNode(SceneCreatorPtr& params);
		static EntityPtr createSprite(SceneCreatorPtr& params);
		static EntityPtr createLight(SceneCreatorPtr& params);

		SceneDataPtr& getSceneData() { return this->sceneData; }
		EntityPtr& getParent() { return this->parent; }

		void setSceneData(SceneDataPtr& data) { this->sceneData = data; }
		void setParent(EntityPtr& p) { this->parent = p; }

	private:

		static void postAddFix(SceneCreatorPtr& params, EntityPtr& entity);

		SceneDataPtr sceneData;
		EntityPtr parent;

	END_CLASS()

}

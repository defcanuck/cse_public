#pragma once

#include "ClassDef.h"
#include "math/GLM.h"
#include "math/Transform.h"
#include "ecs/Entity.h"
#include "gfx/Color.h"

namespace cs
{
	enum LightType
	{
		LightNone = -1,
		LightPoint,
		LightSpot,
		LightArea,
		LightMAX
	};

	class SceneData;

	CLASS_DEFINITION_DERIVED_REFLECT(Light, Entity)
	public:
		
		Light(ECSContext* entityContext);
		Light(const std::string& name, ECSContext* entityContext, int32 idx = 0);
		virtual ~Light();

		uint32 getIndex() const { return index; }

		void setEnabled(bool e) { this->enabled = e; }
		bool getEnabled() const { return this->enabled; }

		void setColor(const ColorF& c) { this->color = c; }
		const ColorF& getColor() const { return this->color; }

		virtual void onSceneAdd(SceneData* scene);
		virtual void onSceneRemove(SceneData* scene);

		void setDiffuseIntensity(float32 intensity) { this->diffuse_intensity = intensity; }
		void setAmbientIntensity(float32 intensity) { this->ambient_intensity = intensity; }

		float getDiffuseIntensity() const { return this->diffuse_intensity; }
		float getAmbientIntensity() const { return this->ambient_intensity; }

		size_t getSelectableVolume(SelectableVolumeList& volumes);

	private:

		Light() { }

		bool enabled;
		uint32 index;
		ColorF color;
		float32 ambient_intensity;
		float32 diffuse_intensity;
		
	};
}
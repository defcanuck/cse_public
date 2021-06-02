#pragma once

#include "ecs/comp/Component.h"
#include "gfx/Color.h"

namespace cs
{
	CLASS_DEFINITION_DERIVED_REFLECT(RayMaterialComponent, Component)
	public:

		typedef std::function<void(const vec3&, ColorF&)> DynamicTextureCallback;

		RayMaterialComponent() :
			color(1.0f, 1.0f, 1.0f, 1.0f),
			transparency(0.0f),
			reflection(0.0f) { }
		RayMaterialComponent(const ColorF& c) :
			color(c),
			transparency(0.0f),
			reflection(0.0f) { }

		void setCallback(DynamicTextureCallback& call) { this->callback = call; }
		void invokeCallback(const vec3& pos, ColorF& callbackColor) {
			if (callback)
				callback(pos, callbackColor);
		}

		const ColorF& getColor() const { return color; }
		
		void setTransparency(float32 trans) { this->transparency = trans; }
		float getTransparency() const { return this->transparency; }

		void setReflection(float32 ref) { this->reflection = ref; }
		float getReflection() const { return this->reflection; }

	private:

		DynamicTextureCallback callback;
		ColorF color;

		float32 transparency;
		float32 reflection;

	};
}
#pragma once

#include "gfx/Renderable.h"

#include "gfx/BatchRenderable.h"
#include "gfx/EnvironmentRenderable.h"
#include "animation/spine/SpineRenderable.h"

#include "physics/PhysicsShape.h"

namespace cs
{

	namespace PhysicsUtils
	{
		PhysicsShapePtr createShapeFromRenderable(RenderablePtr& renderable);

		template <class T>
		PhysicsShapePtr createShape(std::shared_ptr<T> renderable)
		{
			const std::string name = renderable->getMetaData()->getName();
			log::info("PhysicsUtils::createShape Not yet implemented for type:", name);
			return PhysicsShapePtr();
		}

		template <>
		PhysicsShapePtr createShape(std::shared_ptr<BatchRenderable> renderable);

		template <>
		PhysicsShapePtr createShape(std::shared_ptr<EnvironmentRenderable> renderable);

		template <>
		PhysicsShapePtr createShape(std::shared_ptr<SpineRenderable> renderable);
	}
}
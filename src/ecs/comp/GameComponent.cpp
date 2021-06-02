#include "PCH.h"

#include "ecs/comp/GameComponent.h"
#include "ecs/Entity.h"

#include <queue>

namespace cs
{

	BEGIN_META_CLASS(GameSelectableBehavior)
		
	END_META()

	BEGIN_META_CLASS(GameSelectableMoveBehavior)
	END_META()

	BEGIN_META_CLASS(GameSelectableRotateBehavior)
	END_META()

	BEGIN_META_CLASS(GameComponent)
		ADD_MEMBER_PTR(volume);
		ADD_MEMBER_PTR(behavior);
			ADD_COMBO_META_LABEL(GameSelectableMoveBehavior, "Move");
			ADD_COMBO_META_LABEL(GameSelectableRotateBehavior, "Rotate");
		ADD_META_FUNCTION("Compute Quad Volume", &GameComponent::computeQuadVolume);
		ADD_META_FUNCTION("Compute Radius Volume", &GameComponent::computeRadiusVolume);
	END_META()

	GameComponent::GameComponent()
		: volume(CREATE_CLASS_CAST(Volume, QuadVolume))
	{

	}

	void GameComponent::process(float32 dt)
	{
		//...
	}

	void GameComponent::computeQuadVolume()
	{

		struct Vec3Extent
		{
			FloatExtentCalculator x;
			FloatExtentCalculator y;
			
			std::deque<Transform> prevTransforms;
			Transform currentTransform;
		};

		struct compute_volume
		{
			static void start(Entity* entity, void* data)
			{
				Vec3Extent* range = reinterpret_cast<Vec3Extent*>(data);
				SelectableVolumeList volumes;
				entity->getSelectableVolume(volumes);

				mat4 transform = range->currentTransform.getCurrentMatrix();

				for (auto& it : volumes)
				{
					SelectableVolume& vol = it;
					std::vector<vec3> positions;

					vol.volume->getPositions(positions);
					for (auto& pos : positions)
					{
						vec4 adj_pos = transform * vec4(pos, 1.0f);
						range->x.evaluate(adj_pos.x);
						range->y.evaluate(adj_pos.y);
					}
				}

				range->prevTransforms.push_back(range->currentTransform);
				range->currentTransform = entity->getLocalInitialTransform().concatenate(range->currentTransform);
			}

			static void end(Entity* entity, void* data)
			{
				Vec3Extent* range = reinterpret_cast<Vec3Extent*>(data);
				range->currentTransform = range->prevTransforms.back();
				range->prevTransforms.pop_back();
			}
		};

		Vec3Extent range;
		
		this->getParent()->traverseScope(&compute_volume::start, &compute_volume::end, (void*) &range);

		this->volume = CREATE_CLASS(QuadVolume, range.x.minValue, range.y.minValue, range.x.span(), range.y.span());

	}

	void GameComponent::computeRadiusVolume()
	{

	}
}
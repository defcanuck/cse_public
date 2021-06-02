#include "PCH.h"

#include "scene/behaviors/SelectionBehavior.h"
#include "os/LogManager.h"
#include "scene/Scene.h"
#include "ecs/comp/PhysicsComponent.h"
#include "ecs/comp/DrawableComponent.h"
#include "liquid/LiquidContext.h"

#include "math/Plane.h"

namespace cs
{

	const vec3 kPlanePos(0.0f, 0.0f, 0.0f);
	const vec3 kPlaneNorm(0.0f, 0.0f, -1.0f);

	void SelectionBehavior::reset()
	{
		if (this->selected)
		{
			this->deselectEntity();
		}
	}


	const vec3& SelectionBehavior::getPosition() const
	{
		return this->selected->getPosition();
	}

	vec3 SelectionBehavior::adjustPosition(const vec3& pos)
	{
		vec3 adj_pos = pos;
		if (this->gridDimm > 0)
		{
			adj_pos.x = float32((int32(pos.x) / int32(this->gridDimm)) * int32(this->gridDimm));
			adj_pos.y = float32((int32(pos.y) / int32(this->gridDimm)) * int32(this->gridDimm));
			adj_pos.z = float32((int32(pos.z) / int32(this->gridDimm)) * int32(this->gridDimm));
		}
		return adj_pos;
	}

	void SelectionBehavior::setPosition(const vec3& pos)
	{
		vec3 adj_pos = this->adjustPosition(pos);
		this->setPosition(adj_pos);
	}

	bool SelectionBehavior::onInputMove(Scene* scene, const BehaviorData& data)
	{
		if (this->selected)
		{
			// the selection is invalid - no longer useful
			if (this->selected->getSelectionDirty())
			{
				this->selected->setSelectionDirty(false);
				this->deselectEntity();
				return false;
			}

			const Plane plane(kPlanePos, kPlaneNorm);
			float t;
			if (plane.intersects(data.ray, t))
			{
				const vec3 hitPt = data.ray.getPointAt(t);
				vec3 adj_pos = hitPt + this->selectedOffsetWorld;

				switch (this->selectionType)
				{
					case SelectableVolumeTypeDraw:
					case SelectableVolumeTypePhysics:
					case SelectableVolumeTypeReference:
					{
						if (this->selected->getParent())
						{
							SceneNodePtr& parent = this->selected->getParent();
							adj_pos = parent->getWorldTransform().rotate(adj_pos);
						}
						adj_pos.z = this->selectedHitPosWorld.z;
						this->setPosition(adj_pos);
						this->selected->onEdit();

					} break;

					case SelectableVolumeTypeFace:
					case SelectableVolumeTypeVertex:
					case SelectableVolumeTypeTriangle:
					{
						adj_pos = hitPt;
						
						vec4 trans = glm::inverse(this->selectedTransform) * vec4(adj_pos, 1.0f);
						adj_pos = vec3(trans.x, trans.y, trans.z);
						
						this->onSelectedChangedCallback.invoke(adj_pos, this->selectedIndex);

					} break;
					case SelectableVolumeTypeNone:
						// ignore 
						break;
					default:
						assert(false);
						log::error("No move behavior defined for type");
				}
			}
		}

		return this->selected != nullptr;
	}

	bool SelectionBehavior::onInputPressed(Scene* scene, const BehaviorData& data)
	{
		struct
		{
			bool operator()(const std::pair<EntityPtr, EntityIntersection>& a, const std::pair<EntityPtr, EntityIntersection>& b)
			{
				return a.second.distance < b.second.distance;
			}
		} EntityIntersectionSort;

		if (data.input == ClickSecondary)
			return false;

		SceneDataPtr scene_data = scene->getSceneData();
		vec3 hit_pos;

		if (!this->selected)
		{
			Entity::EntityList entities;
			scene_data->getAllEntities(entities);
			std::vector<std::pair<EntityPtr, EntityIntersection>> hit_volumes;

			for (auto it : entities)
			{

				if (!it->getSelectable())
					continue;

				EntityPtr entity = std::static_pointer_cast<Entity>(it);
				EntityIntersection intersectionData;
				

				if (SceneData::intersects(entity.get(), data.ray, intersectionData))
				{
					hit_volumes.push_back({ entity, intersectionData });
				}
			}

			if (hit_volumes.size() > 0)
			{
				std::sort(hit_volumes.begin(), hit_volumes.end(), EntityIntersectionSort);
				EntityIntersection intersectionData = hit_volumes[0].second;
				EntityPtr entity = hit_volumes[0].first;

				this->selectEntity(entity);
				this->selectedOffsetWorld = this->selected->getPosition() - intersectionData.hitPosWorld;
				this->selectedHitPosWorld = intersectionData.hitPosWorld;
				this->selectedHitPosLocal = intersectionData.hitPosLocal;
				this->selectionType = intersectionData.type;

				if (isSelectableSubType(intersectionData.type))
				{
					this->selectedTransform = entity->getWorldTransform().getCurrentMatrix();
					this->selectedIndex = intersectionData.index;
					this->onSelectedChangedCallback = intersectionData.onChangedCallback;
				}
			}
			
		} else {

			EntityIntersection intersectionData;
			bool intersects = SceneData::intersects(this->selected.get(), data.ray, intersectionData);
			if (isSelectableSubType(intersectionData.type))
			{
				this->selectedTransform = this->selected->getWorldTransform().getCurrentMatrix();
				this->selectedIndex = intersectionData.index;
				this->onSelectedChangedCallback = intersectionData.onChangedCallback;
			}

			this->onIntersects(intersects, intersectionData.hitPosWorld);
		}

		return this->selected != nullptr;
	}

	bool SelectionBehavior::onIntersects(bool intersect, const vec3& hit_pos)
	{
		if (intersect)
		{
			this->deselectEntity();
			return false;
		}
		
		this->selectedOffsetWorld = this->getPosition() - hit_pos;
		return true;
	}

	bool SelectionBehavior::onInputReleased(Scene* scene, const BehaviorData& data)
	{
		if (data.input == ClickSecondary)
			return false;

		if (this->selected)
		{
			EntityIntersection intersectionData;
			if (!SceneData::intersects(this->selected.get(), data.ray, intersectionData))
			{
				this->deselectEntity();
			}
		}

		return this->selected != nullptr;
	}

	void SelectionBehavior::selectEntity(EntityPtr& entity)
	{
		assert(this->selected == nullptr);
		this->selected = entity;
		this->onSelectEntity();
		this->clearGrid();
	}

	void SelectionBehavior::deselectEntity()
	{
		this->onDeselectEntity();
		this->selected = nullptr;
		this->selectedIndex = -1;
		this->selectionType = SelectableVolumeTypeNone;
	}

	void SelectionBehavior::moveSelected(const vec3& pos)
	{

	}

	void SelectionBehavior::setToGrid(uint32 grid_dimm)
	{
		this->gridDimm = grid_dimm;
	}

	void SelectionBehavior::clearGrid()
	{
		this->gridDimm = 0;
	}


}

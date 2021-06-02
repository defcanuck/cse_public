#pragma once

#include "scene/behaviors/Behavior.h"
#include "scene/Actor.h"

namespace cs
{
	
	CLASS_DEFINITION_DERIVED(SelectionBehavior, Behavior)
	
	public:

		SelectionBehavior() 
			: selected(nullptr)
			, selectionType(SelectableVolumeTypeNone)
			, gridDimm(0)
			, selectedIndex(-1)
		{ }

		virtual ~SelectionBehavior() { }

		virtual bool onInputMove(Scene* scene, const BehaviorData& data);
		virtual bool onInputPressed(Scene* scene, const BehaviorData& data);
		virtual bool onInputReleased(Scene* scene, const BehaviorData& data);
		virtual bool onIntersects(bool intersect, const vec3& hit_pos);

		virtual void reset();

		virtual void setPosition(const vec3& pos);
		virtual const vec3& getPosition() const;

		bool hasSelected() const { return this->selected != nullptr; }
		void deselectEntity();

		EntityPtr& getSelected() { return this->selected; }

		void setToGrid(uint32 grid_dimm);
		void clearGrid();

		size_t getSelectedSubIndex() const { return this->selectedIndex; }
		void resetSelectedSubIndex() { this->selectedIndex = -1; }

	protected:

		vec3 adjustPosition(const vec3& pos);

		virtual void selectEntity(EntityPtr& actor);
		virtual void moveSelected(const vec3& pos);

		virtual void onSelectEntity() { }
		virtual void onDeselectEntity() { }

		EntityPtr selected;
		SelectableVolumeType selectionType;

		vec3 selectedOffsetWorld;
		vec3 selectedHitPosWorld;
		mat4 selectedTransform;
	
		vec3 selectedHitPosLocal;
		size_t selectedIndex;
		CallbackArg2<void, vec3, size_t> onSelectedChangedCallback;

		uint32 gridDimm;
	};
}
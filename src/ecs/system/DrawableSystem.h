#pragma once

#include "ecs/system/BaseSystem.h"
#include "ecs/comp/DrawableComponent.h"
#include "gfx/RenderTexture.h"
#include "gfx/BatchDraw.h"
#include "gfx/DisplayList.h"

namespace cs
{
	enum DrawPass
	{
		DrawPassNone = -1,
		DrawPassDiffuse,
		//...
		DrawPassMAX
	};

	class DrawableSystem : public ECSContextSystemBase<DrawableSystem, ECSDraw>, public BaseSystem
	{

		const static uint32 kInitNumElements = 1000;

	public:
		DrawableSystem(ECSContext* cxt);
		virtual ~DrawableSystem();
	
		virtual void processImpl(SystemUpdateParams* params);
		void flush(DisplayList& display_list);

		void addRenderTarget(RenderTexturePtr ptr); 
		void setCullCamera(const CameraPtr& camera) { this->cullCamera = camera; }

	protected:

		virtual void onComponentSystemAdd(uint32 id, ComponentPtr& component);
		virtual void onComponentSystemRemove(uint32 id, ComponentPtr& component);

	private:
        
        void allocGeometry();
        
        bool allocated;
		CameraPtr cullCamera;
		ComponentMap staticDrawable;
		ComponentMap dynamicDrawable;

		BatchDrawPtr batch[RenderTraversalMAX];

	};
}

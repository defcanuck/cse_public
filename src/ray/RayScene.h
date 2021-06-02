#pragma once

#include "scene/Scene.h"
#include "math/BoundingVolume.h"
#include "gfx/Color.h"

#include <thread>

namespace cs
{

	struct CalcParams
	{
		uint32 startX;
		uint32 startY;
		uint32 boundWidth;
		uint32 boundHeight;
		uint32 width;
		uint32 height;
	};

	struct CalcThread
	{
		std::thread thread;
		CalcParams params;
	};

	CLASS_DEFINITION_DERIVED(RayScene, Scene)
	public:

		RayScene(SceneLock& lock, SceneParams& params) :
			Scene(lock, params)
		{ }

		typedef std::function<void(const Ray&, ColorF*)> ClearColorFunction;

		const static int32 kMaxDepth = 1;
		const static int32 kRaysPerPixel = 1;
		const static uint32 kNumThreads = 4;

		void populate(uchar* bytes, const Dimensions& dimm, TextureChannels channels);
		bool cast(const Ray& ray, HitParams& params, ColorF* color, EntityPtr ignoreActor = nullptr, int32 depth = 0);

		void setClearFunction(ClearColorFunction& func) { this->clearFunc = func; }

	protected:

		ClearColorFunction clearFunc;

		EntityPtr castActors(const Ray& ray, HitParams& params, EntityPtr ignoreActor = nullptr);
		void getClearColor(const Ray& ray, ColorF* color);
		void getDiffuseColor(EntityPtr& actor, HitParams& params, ColorF* color);

		CalcThread threads[RayScene::kNumThreads];
	};
}
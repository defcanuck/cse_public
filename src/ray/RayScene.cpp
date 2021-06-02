#include "PCH.h"

#include "ray/RayScene.h"

#include "global/Timer.h"
#include "global/Utils.h"

#include "ecs/comp/CollisionComponent.h"
#include "ray/RayMaterialComponent.h"


namespace cs
{
	static volatile int numRaysCast = 0;

	void calculate(uint32 threadId, RayScene* scene, uchar* bytes, CalcParams* cp)
	{
		Ray ray;
		HitParams params;

		Timer threadTimer;
		threadTimer.start();

		uint32* ptr = reinterpret_cast<uint32*>(bytes);
		
		for (uint32 j = cp->startY; j < cp->boundHeight; j++)
		{
			for (uint32 i = cp->startX; i < cp->boundWidth; i++)
			{
				scene->getCamera()->getRay(PointI(i, j), ray);
				ColorB* color = reinterpret_cast<ColorB*>(ptr + (i + (cp->width * j)));

				*color = ColorB(0, 0, 0, 0);

				const float32 kRaysPerPixelScale = 1.0f / float32(RayScene::kRaysPerPixel);
				const float32 kRayWiggle = 0.001f;
				for (uint32 r = 0; r < RayScene::kRaysPerPixel; r++)
				{
					ColorF castColor;
					Ray castRay(ray);
					if (r > 0)
					{
						float32 randX = randomRange(-kRayWiggle, kRayWiggle);
						float32 randY = randomRange(-kRayWiggle, kRayWiggle);
						float32 randZ = randomRange(-kRayWiggle, kRayWiggle);

						castRay = Ray(ray.getOrigin(), glm::normalize(ray.getDirection() + vec3(randX, randY, randZ)));
					}

					scene->cast(castRay, params, &castColor);
					
					(*color).r += uchar(std::min<float32>(castColor.r * 255.0f, 255.0f) * kRaysPerPixelScale);
					(*color).g += uchar(std::min<float32>(castColor.g * 255.0f, 255.0f) * kRaysPerPixelScale);
					(*color).b += uchar(std::min<float32>(castColor.b * 255.0f, 255.0f) * kRaysPerPixelScale);
					(*color).a = 255;
					
				}
			}
		}

		float32 threadTime = threadTimer.getTicks() / 1000.0f;
		log::print(LogInfo, "Thread ", threadId, " finished in ", threadTime, " seconds");
	}

	void RayScene::populate(uchar* bytes, const Dimensions& dimm, TextureChannels channels)
	{

		Timer frameTimer;
		frameTimer.start();
		
		numRaysCast = 0;

		const uint32 threadWidth = dimm.w / kNumThreads;
		for (uint32 i = 0; i < kNumThreads; i++)
		{
			threads[i].params.startX = i * threadWidth;
			threads[i].params.startY = 0;
			threads[i].params.boundWidth = (i + 1) * threadWidth;
			threads[i].params.boundHeight = dimm.h;
			threads[i].params.width = dimm.w;
			threads[i].params.height = dimm.h;

			// Start threads
			threads[i].thread = std::thread(calculate, i, this, bytes, &threads[i].params);
		}
		
		// Wait for all threads to join
		for (uint32 i = 0; i < kNumThreads; i++)
			threads[i].thread.join();

		float32 frameTime = frameTimer.getTicks() / 1000.0f;
		log::print(LogInfo, "Updated Frame. Time: ", frameTime, " seconds");
		log::print(LogInfo, "Total Rays Cast: ", numRaysCast);
	}

	EntityPtr RayScene::castActors(const Ray& ray, HitParams& params, EntityPtr ignoreActor)
	{
		EntityPtr hitActor = nullptr;
		params.hitDistance = FLT_MAX;

		Entity::EntityList& actors = this->data->getEntityList();
		for (auto it : actors)
		{
			EntityPtr actor = it;
			if (ignoreActor.get() == actor.get())
				continue;

			CollisionComponentPtr col = actor->getComponent<CollisionComponent>();
			if (!col)
				continue;

			HitParams testHit;
			if (col->intersect(ray, it->getWorldTransform(), testHit) && testHit.hitDistance > 0 && testHit.hitDistance < params.hitDistance)
			{
				params.hitDistance = testHit.hitDistance;
				params.hitNormal = testHit.hitNormal;
				params.hitPos = testHit.hitPos;
				hitActor = actor;
			}
		}

		return hitActor;
	}

	void RayScene::getDiffuseColor(EntityPtr& actor, HitParams& params, ColorF* color)
	{
		
		ColorF matColor(255, 255, 255, 255); 
		RayMaterialComponentPtr mat = actor->getComponent<RayMaterialComponent>();
		if (mat)
		{
			matColor = mat->getColor();
			mat->invokeCallback(params.hitPos, matColor);
		}
		
		SceneData::LightList& lights = this->data->getAllLights();
		for (auto it : lights)
		{
			LightPtr& light = it;
			if (!light->getEnabled())
				continue;

			vec3 lightPos = light->getWorldTransform().getPosition();
			vec3 toLight = glm::normalize(lightPos - params.hitPos);

			float32 ambient = 0.5f;
			float32 diffuse = glm::dot(toLight, params.hitNormal);

			if (diffuse > 0.0f)
			{
				Ray toLightRay(params.hitPos, toLight);
				HitParams lightHitParams;
				EntityPtr obscureActor = this->castActors(toLightRay, lightHitParams, actor);
				if (obscureActor)
					diffuse = 0.0f;
			}

			float32 clampedLamber = clamp(0.0f, 1.0f, diffuse + ambient);
			ColorF lightColor = light->getColor();

			(*color) = (*color) + (matColor * lightColor * clampedLamber);
		}
	}

	void RayScene::getClearColor(const Ray& ray, ColorF* color)
	{
		if (this->clearFunc)
		{
			this->clearFunc(ray, color);
			return;
		}
	
		(*color).r = fabs(ray.getDirection().x); 
		(*color).g = fabs(ray.getDirection().y);
		(*color).b = fabs(ray.getDirection().z);
		(*color).a = 255;
	}

	bool RayScene::cast(const Ray& ray, HitParams& params, ColorF* color, EntityPtr ignoreActor, int32 depth)
	{
		numRaysCast++;

		if (depth > RayScene::kMaxDepth)
		{
			this->getClearColor(ray, color);
			return false;
		}

		EntityPtr hitActor = this->castActors(ray, params, ignoreActor);
		if (!hitActor)
		{
			this->getClearColor(ray, color);
			return false;
		}
		
		ColorF fresnelColor(0.0f, 0.0f, 0.0f, 255.0f);
		RayMaterialComponentPtr mat = hitActor->getComponent<RayMaterialComponent>();

		ColorF refractColor(0.0f, 0.0f, 0.0f, 0.0f);
		ColorF reflectColor(0.0f, 0.0f, 0.0f, 0.0f);

		float32 fresnelEffect = 1.0f;
		
		if (mat && mat->getReflection() > 0)
		{
			Ray reflectRay(params.hitPos, glm::normalize(glm::reflect(ray.getDirection(), params.hitNormal)));
			HitParams reflectParams;
			this->cast(reflectRay, reflectParams, &reflectColor, hitActor, ++depth);
		}
		
		if (mat->getTransparency() > 0)
		{
			float32 facingratio = -glm::dot(ray.getDirection(), params.hitNormal);
			fresnelEffect = lerp((float32) pow(1 - facingratio, 3), 1.0f, 0.1f);

			float32 ior = 1.1f;
			float32 eta = 1 / ior; 
				
			// are we inside or outside the surface? 
			float32 cosi = -facingratio;
			float32 k = 1 - eta * eta * (1 - cosi * cosi);
			vec3 refrdir = (ray.getDirection() * eta) + params.hitNormal * (eta *  cosi - float32(sqrt(k)));
			glm::normalize(refrdir);

			Ray refractRay(params.hitPos, refrdir);
			HitParams refractParams;
				
			this->cast(refractRay, refractParams, &refractColor, hitActor, ++depth);
		}

		fresnelColor = (reflectColor * fresnelEffect) + (refractColor * (1 - fresnelEffect));

		ColorF diffuseColor(0.0f, 0.0f, 0.0f, 0.0f);
		this->getDiffuseColor(hitActor, params, &diffuseColor);
		(*color) = (*color) + fresnelColor + diffuseColor;

		return true;

	}
}
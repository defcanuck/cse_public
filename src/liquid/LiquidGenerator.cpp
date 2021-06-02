#include "PCH.h"

#include "liquid/LiquidGenerator.h"
#include "math/GLM.h"

namespace cs
{
	BEGIN_META_CLASS(LiquidGenerator)

	END_META()

	BEGIN_META_CLASS(LiquidGeneratorRandom)
		ADD_MEMBER_PTR(volume);
	END_META()

	BEGIN_META_CLASS(LiquidGeneratorGrid)
		ADD_MEMBER_PTR(volume);
	END_META()

	BEGIN_META_CLASS(LiquidGeneratorPreset)
		ADD_MEMBER(particles);
			SET_MEMBER_IGNORE_GUI();
	END_META()

	LiquidGeneratorRandom::LiquidGeneratorRandom() :
		LiquidGenerator(),
		volume(CREATE_CLASS(QuadVolume, QuadVolume::kDefaultVolume))
	{

	}

	LiquidGeneratorRandom::LiquidGeneratorRandom(uint32 num) :
		LiquidGenerator(num),
		volume(CREATE_CLASS(QuadVolume, QuadVolume::kDefaultVolume))
	{

	}

	LiquidGeneratorRandom::LiquidGeneratorRandom(uint32 num, VolumePtr& vol) :
		LiquidGenerator(num),
		volume(vol)
	{

	}

	void LiquidGeneratorRandom::populate(std::vector<vec2>& positions, uint32 requested)
	{
		if (!this->volume)
			return;

		for (uint32 i = 0; i < requested; i++)
		{
			positions.push_back(this->volume->getRandomValue());
		}
	}

	void LiquidGeneratorRandom::onPostLoad(const LoadFlagMask& flags)
	{
		BASECLASS::onPostLoad(flags);
	}

	bool LiquidGeneratorRandom::intersects(Ray& ray, vec3& hit_pos)
	{
		if (this->volume)
			return this->volume->intersects(ray, hit_pos);

		log_error("No volume defined to resolve intersection");
		return false;
	}

	bool LiquidGeneratorPreset::intersects(Ray& ray, vec3& hit_pos)
	{
		if (this->volume)
			return this->volume->intersects(ray, hit_pos);
		
		log_error("No volume defined to resolve intersection");
		return false;
	}

	void LiquidGeneratorPreset::addParticle(const vec2 pos) 
	{ 
		this->particles.push_back(pos); 
		this->updateVolume();
	}

	void LiquidGeneratorPreset::populate(std::vector<vec2>& positions, uint32 requested)
	{
		uint32 num = std::min(requested, uint32(this->particles.size()));
		for (uint32 i = 0; i < num; i++)
			positions.push_back(this->particles[i]); 
	}

	void LiquidGeneratorPreset::updateVolume()
	{
		vec2 min_pos, max_pos;
		glm_util::bounds(particles, min_pos, max_pos);
		RectF rect(min_pos.x, min_pos.y, max_pos.x - min_pos.x, max_pos.y - min_pos.y);

		this->volume = CREATE_CLASS(QuadVolume, rect);
	}

	void LiquidGeneratorPreset::onPostLoad(const LoadFlagMask& flags)
	{
		BASECLASS::onPostLoad(flags);
		this->updateVolume();
	}

	LiquidGeneratorGrid::LiquidGeneratorGrid() :
		LiquidGenerator(),
		volume(CREATE_CLASS(QuadVolume, QuadVolume::kDefaultVolume))
	{

	}

	LiquidGeneratorGrid::LiquidGeneratorGrid(uint32 num) :
		LiquidGenerator(num),
		volume(CREATE_CLASS(QuadVolume, QuadVolume::kDefaultVolume))
	{

	}

	LiquidGeneratorGrid::LiquidGeneratorGrid(uint32 num, VolumePtr& vol) :
		LiquidGenerator(num),
		volume(vol)
	{

	}

	void LiquidGeneratorGrid::populate(std::vector<vec2>& positions, uint32 requested)
	{
		if (!this->volume)
			return;

		RectF volume_rect = this->volume->getRect();
		uint32 req = static_cast<uint32>(sqrt(requested) + 0.5f);

		float32 xdiv = volume_rect.size.w / float32(req);
		float32 ydiv = volume_rect.size.h / float32(req);

		for (uint32 w = 0; w < req; w++)
		{
			for (uint32 h = 0; h < req; h++)
			{
				float32 x = volume_rect.pos.x + (xdiv * float32(w));
				float32 y = volume_rect.pos.y + (ydiv * float32(h));

				positions.push_back(vec2(x, y));
			}
		}
	}

	void LiquidGeneratorGrid::onPostLoad(const LoadFlagMask& flags)
	{
		BASECLASS::onPostLoad(flags);
	}

	bool LiquidGeneratorGrid::intersects(Ray& ray, vec3& hit_pos)
	{
		if (this->volume)
			return this->volume->intersects(ray, hit_pos);

		log_error("No volume defined to resolve intersection");
		return false;
	}

}

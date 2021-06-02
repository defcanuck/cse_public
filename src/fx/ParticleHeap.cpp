#include "PCH.h"

#include "fx/ParticleHeap.h"
#include "fx/ParticleEmitter.h"

#include "gfx/RenderInterface.h"
#include "gfx/DrawCall.h"

#include "global/Stats.h"

namespace cs
{

	int32 ParticleHeap::gTotalParticles = 0;
	int32 ParticleHeap::gTotalHeapSize = 0;
    
    const int32 kDropDeadHeapSize = 1000;

	void ParticleHeapUpdate::updateColorDefault(ColorB* dst, ParticleBuffer& buffer, size_t index)
	{
		(*dst) = ColorB::White;
	}

	void ParticleHeapUpdate::updateColorBuffer(ColorB* dst, ParticleBuffer& buffer, size_t index)
	{
		ColorB* col = buffer.get<ColorB>(ParticlePropertyColor, index);
		assert(col);
		(*dst) = *col;
	}

	void ParticleHeap::resetStats()
	{
		gTotalParticles = 0;
		gTotalHeapSize = 0;
	}

	ParticleHeap::ParticleHeap(ParticleEffectPtr& eff)
		: contentScale(1.0f)
		, maxParticles(0)
		, numParticles(0)
		, didResize(0)
		, lastTick(0)
		, gracePeriod(5)
	{
		this->effect = eff;
		GeometryDataPtr data = CREATE_CLASS(cs::GeometryData);

		size_t max_particles = this->effect->getMaxParticles() * 10;
		const ParticlePropertyMask& mask = this->effect->getMask();

		data->decl.addAttrib(AttributeType::AttribPosition, { AttributeType::AttribPosition, TypeFloat, 3, 0 });
		data->decl.addAttrib(AttributeType::AttribTexCoord0, { AttributeType::AttribTexCoord0, TypeFloat, 2, data->decl.getStride() });
		data->decl.addAttrib(AttributeType::AttribColor, { AttributeType::AttribColor, TypeFloat, 4, data->decl.getStride() });

		data->vertexSize = max_particles * 4;
		data->indexSize = max_particles * 6;
		data->storage = BufferStorageDynamic;

		size_t indexCtr = 0;
		data->indexData.resize(data->indexSize);
		for (size_t i = 0; i < max_particles; i++)
		{
			uint16 offset = uint16(i) * 4;
			data->indexData[indexCtr++] = offset + kStaticQuadIndices[0];
			data->indexData[indexCtr++] = offset + kStaticQuadIndices[1];
			data->indexData[indexCtr++] = offset + kStaticQuadIndices[2];

			data->indexData[indexCtr++] = offset + kStaticQuadIndices[3];
			data->indexData[indexCtr++] = offset + kStaticQuadIndices[4];
			data->indexData[indexCtr++] = offset + kStaticQuadIndices[5];
		}

		this->setMaxParticles(int32(max_particles));
		this->numParticles = 0;

		this->geom = CREATE_CLASS(DynamicGeometry, data);

		DynamicGeometry::VertexUpdateFunc vfunc = std::bind(
			&ParticleHeap::updateVertices,
			this,
			std::placeholders::_1,
			std::placeholders::_2,
			std::placeholders::_3);
		this->geom->setVertexUpdateFunc(vfunc);

		/*
		// totally unnecessary
		DynamicGeometry::IndexUpdateFunc ifunc = std::bind(
			&ParticleHeap::updateIndices,
			this,
			std::placeholders::_1,
			std::placeholders::_2);
		this->geom->setIndexUpdateFunc(ifunc);
		*/

		DynamicGeometry::AdjustDrawCallFunc dcfunc = std::bind(
			&ParticleHeap::setDrawParams,
			this,
			std::placeholders::_1,
			std::placeholders::_2);
		this->geom->setDrawCallAdjustFunc(dcfunc);

		DynamicGeometry::GetVertexSizeFunc numVFunc = std::bind(&ParticleHeap::getVertexBufferSize, this);
		this->geom->setVertexBufferSizeFunc(numVFunc);

		DynamicGeometry::GetIndexSizeFunc numIFunc = std::bind(&ParticleHeap::getIndexBufferSize, this);
		this->geom->setIndexBufferSizeFunc(numIFunc);

		this->buffer = new ParticleBuffer(mask, this->maxParticles);
		this->buffer->allocate();

		this->updateColorCallback = &ParticleHeapUpdate::updateColorDefault;
		if (this->buffer->hasProperty(ParticlePropertyColor))
		{
			this->updateColorCallback = &ParticleHeapUpdate::updateColorBuffer;
		}

		EngineStats::incrementStat(StatTypeFXHeap);
	}
    
    void ParticleHeap::setMaxParticles(int32 max_particles)
    {
        this->maxParticles = std::min<int32>(max_particles, kDropDeadHeapSize);
    }

	ParticleHeap::~ParticleHeap()
	{
		delete this->buffer;
		this->buffer = nullptr; 

		EngineStats::decrementStat(StatTypeFXHeap);
	}

	size_t ParticleHeap::getVertexBufferSize()
	{
		size_t stride = this->geom->getGeometryData()->decl.getStride();
		return this->maxParticles * 4 * stride;
	}

	size_t ParticleHeap::getIndexBufferSize()
	{
		return this->maxParticles * 6 * sizeof(uint16);
	}

	size_t ParticleHeap::updateVertices(uchar* data, size_t bufferSize, VertexDeclaration& decl)
	{

		ParticleEffectDataPtr effectData = this->effect->getParticleEffectData();
		assert(effectData.get());

		TextureHandlePtr textureHandle = effectData->getTextureHandle();
		if (!textureHandle.get())
			textureHandle = RenderInterface::kWhiteTexture;

		vec2 uvs[4];

		uint32 vertexCtr = 0;
		char* pos_ptr = decl.getAttributePointerAtIndex<char>(data, AttributeType::AttribPosition, 0);
		char* uv_ptr = decl.getAttributePointerAtIndex<char>(data, AttributeType::AttribTexCoord0, 0);
		char* color_ptr = decl.getAttributePointerAtIndex<char>(data, AttributeType::AttribColor, 0);
		size_t stride = decl.getStride();

		float32 zOffset = 0.0f;
		

		for (size_t i = 0; i < this->numParticles; i++)
		{
			vec3* pos = this->buffer->get<vec3>(ParticlePropertyPosition, i);
			vec2* size = this->buffer->get<vec2>(ParticlePropertySize, i);

			float32* time = this->buffer->get<float32>(ParticlePropertyTime, i);
			float32* lifeTime = this->buffer->get<float32>(ParticlePropertyLifetime, i);

			assert(pos);
			assert(size);
			assert(time);
			assert(lifeTime);

			float32 pct = *time / *lifeTime;
            
			if (this->buffer->hasProperty(ParticlePropertyIndex))
			{
				// add texture pct here
				int32* frame_index = this->buffer->get<int32>(ParticlePropertyIndex, i);
				assert(frame_index);
				textureHandle->getCorners(uvs, *frame_index);
			}
			else
			{
				textureHandle->getCorners(uvs, pct);
			}
			
			float32 halfW = (*size).x * 0.5f * this->contentScale;
			float32 halfH = (*size).y * 0.5f * this->contentScale;

			vec3 positions[] =
			{
				vec3(-halfW, -halfH, 0.0f),
				vec3(-halfW,  halfH, 0.0f),
				vec3( halfW,  halfH, 0.0f),
				vec3( halfW, -halfH, 0.0f)
			};

			bool hasAngle = this->buffer->hasProperty(ParticlePropertyAngle);
			float32* angle = nullptr;
			if (hasAngle)
			{
				angle = this->buffer->get<float32>(ParticlePropertyAngle, i);
			}
			
			for (size_t t = 0; t < 4; t++)
			{
			
				vec3& position = positions[t];
				if (hasAngle)
				{
					assert(angle);
					float32 rad = degreesToRadians(*angle);
					float32 sin_angle = sin(rad);
					float32 cos_angle = cos(rad);
					float32 xp = position.x;
					float32 yp = position.y;

					position.x = (xp * cos_angle) - (yp * sin_angle);
					position.y = (xp * sin_angle) + (yp * cos_angle);
				}
				position += (*pos);

				vec3* pos = reinterpret_cast<vec3*>(PTR_ADD(pos_ptr, vertexCtr * stride));
				assert(size_t(pos) - size_t(data) < bufferSize);
				*pos = positions[t];

				vec2* uv = reinterpret_cast<vec2*>(PTR_ADD(uv_ptr, vertexCtr * stride));
				assert(size_t(uv) - size_t(data) < bufferSize);
				*uv = uvs[t];

				ColorF* color = reinterpret_cast<ColorF*>(PTR_ADD(color_ptr, ((i * 4) + t) * stride));
				ColorB tmpColor;
				this->updateColorCallback(&tmpColor, *this->buffer, i);
				(*color) = toColorF(tmpColor);

				vertexCtr++;
			}
		}


		gTotalParticles += static_cast<int32>(this->numParticles);
		gTotalHeapSize += static_cast<int32>(this->numParticles * decl.getStride());

		return vertexCtr;
	}

	size_t ParticleHeap::updateIndices(uchar* data, size_t bufferSize)
	{
        
        uint16 kFXStaticQuadIndices[] = { 0, 1, 2, 0, 2, 3 };
        
		if (this->didResize > 0)
		{
			uint16* index_data = reinterpret_cast<uint16*>(data);
			size_t indexCtr = 0;
			for (size_t i = 0; i < this->maxParticles; i++)
			{
				uint16 offset = uint16(i) * 4;
                uint16 indices[6];
                indices[0] = offset + kFXStaticQuadIndices[0];
                indices[1] = offset + kFXStaticQuadIndices[1];
                indices[2] = offset + kFXStaticQuadIndices[2];
                indices[3] = offset + kFXStaticQuadIndices[3];
                indices[4] = offset + kFXStaticQuadIndices[4];
                indices[5] = offset + kFXStaticQuadIndices[5];
				memcpy(index_data, indices, 6 * sizeof(uint16));
				index_data += 6;
			}
			this->didResize--;
		}

		gTotalHeapSize += bufferSize;
		return uint32(this->numParticles) * 6;
	}

	void ParticleHeap::setDrawParams(int32 index, std::vector<DrawCallPtr>& dcs)
	{

		ParticleEffectDataPtr effectData = this->effect->getParticleEffectData();
		assert(effectData.get());

		DrawCallPtr dc = CREATE_CLASS(DrawCall);
		dc->tag = this->effect->getName();
		dc->type = DrawTriangles;
		dc->indexType = TypeUnsignedShort;
		dc->count = uint32(this->numParticles) * 6;
		dc->offset = 0;
        dc->shaderHandle = effectData->getShaderHandle();
		dc->textures[TextureStageDiffuse] = effectData->getTextureHandle();
		dc->color = ColorB::White;
        dc->cullFace = CullNone;
        
		effectData->getDrawOptions().populate(dc);

        dc->depthTest = false;
        
		dcs.push_back(dc);
	}

	void ParticleHeap::draw()
	{
		this->geom->draw(nullptr);
	}

	void ParticleHeap::flush(DisplayListTraversal& traversal_list)
	{
		if (this->numParticles == 0)
			return;

		mat4 projection = traversal_list.camera->getCurrentProjection();
		mat4 view = traversal_list.camera->getCurrentView();

		DisplayListNode node;
		mat4 model(1.0f);

		node.geomList.push_back(DisplayListGeom(std::static_pointer_cast<Geometry>(this->geom)));
		node.mvp = projection * view * model;

		traversal_list.nodes.push_back(node);
	}

	void ParticleHeap::link(ParticleEmitter* emitter)
	{
		auto it = std::find(this->links.begin(), this->links.end(), emitter);
		if (it == this->links.end())
		{
			this->links.insert(it, emitter);
		}

		size_t newSize = std::max<size_t>(this->maxParticles, this->links.size() * this->effect->getMaxParticles());
		if (newSize > this->maxParticles)
		{
			this->setMaxParticles(newSize * 2);
			this->buffer->resizeCopy(this->maxParticles);
			this->didResize = 2;
		}
	}

	bool ParticleHeap::isExpired() const
	{
		return Timer::getCurrentTick() > this->lastTick;
	}

	void ParticleHeap::unlink(ParticleEmitter* emitter)
	{
		auto it = std::find(this->links.begin(), this->links.end(), emitter);
		if (it != this->links.end())
		{
			this->links.erase(it);
			if (this->links.size() == 0)
			{
				this->lastTick = Timer::getCurrentTick() + (emitter->getMaxLifetime() * 20);
			}
		}
		else
		{
			log::warning("No emitter link for emitter - this might be very bad?");
		}
	}

	void ParticleHeap::process(float32 dt)
	{
        if (dt <= 0.000f || dt > 1.0f)
            return;

		this->gracePeriod--;
        
		if (this->effect.get() && this->effect->getMaxParticles() > this->maxParticles)
		{
			// Free all previous particles in the heap
			this->setMaxParticles(this->effect->getMaxParticles());
			this->buffer->resize(this->maxParticles);
			this->numParticles = 0;
			this->didResize = 2;
			return;
		}

		size_t particle_index = 0;
		while (particle_index < this->numParticles)
		{
			float32* time = this->buffer->get<float32>(ParticlePropertyTime, particle_index);
			float32* lifeTime = this->buffer->get<float32>(ParticlePropertyLifetime, particle_index);

			assert(time);
			assert(lifeTime);

			(*time) += dt;

			if (*time >= *lifeTime)
			{
				this->buffer->swap(particle_index, this->numParticles - 1);
				this->numParticles--;

				void** owner = this->buffer->get<void*>(ParticlePropertyOwner, this->numParticles);
				assert(owner);
				this->removeParticleFromOwner(*owner);

				continue;
			}

			float32 pct = *time / *lifeTime;

			float32* processTime = this->buffer->get<float32>(ParticlePropertyProcessTime, particle_index);
			bool shouldUpdate = (!processTime || (*processTime) < 0.0f || pct < (*processTime));
			{
				float useDt = (shouldUpdate) ? dt : 0.0f;
				float usePct = (shouldUpdate) ? pct : *processTime;
				this->buffer->update(useDt, usePct, particle_index);
			}
			
			particle_index++;
		}

		if (this->geom)
			this->geom->update();
	}

	size_t ParticleHeap::addParticles(ParticleInitList& particle_list)
	{
		if (this->numParticles >= this->maxParticles)
		{
			log::warning("Exceeded particle count for heap - consider making larger!");
			return 0;
		}

		size_t particles_created = 0;
		for (size_t i = 0; i < particle_list.numParticles; i++)
		{
			ParticleInitProps& particle = particle_list.initList[i];
			if (this->numParticles >= this->maxParticles)
			{
				break;
			}

			void** owner = this->buffer->get<void*>(ParticlePropertyOwner, this->numParticles);
			float32* time = this->buffer->get<float32>(ParticlePropertyTime, this->numParticles);
			float32* lifeTime = this->buffer->get<float32>(ParticlePropertyLifetime, this->numParticles);
			float32* processTime = this->buffer->get<float32>(ParticlePropertyProcessTime, this->numParticles);
			vec3* pos = this->buffer->get<vec3>(ParticlePropertyPosition, this->numParticles);

			(*time) = 0.0f;
			(*lifeTime) = particle.lifeTime;
			(*pos) = particle.position;
           
            // Metal Fix Depth
            (*pos).z = -(*pos).z;
            
			(*processTime) = particle.processTime;
			(*owner) = particle_list.creator;

			for (auto& prop : particle.propertyMap)
			{
				assert(prop.second->getSize() > 0);
				assert(prop.second->getData() != nullptr);

				this->buffer->copy(prop.first, this->numParticles, prop.second->getData(), prop.second->getSize());
			}


			if (this->buffer->hasProperty(ParticlePropertyColor))
			{
				ColorB* color = this->buffer->get<ColorB>(ParticlePropertyColor, this->numParticles);
				(*color) = (*color) * particle_list.tint * particle.tint;
			}

			if (this->buffer->hasProperty(ParticlePropertyColorRange))
			{
				ColorB* color = this->buffer->get<ColorB>(ParticlePropertyColorRange, this->numParticles);
				(*color) = (*color) * particle_list.tint * particle.tint;
			}

			// Rotate velocity if we have a rotation available
			if (this->buffer->hasProperty(ParticlePropertyVelocity))
			{
				vec3* vel = this->buffer->get<vec3>(ParticlePropertyVelocity, this->numParticles);
				(*vel) = particle.rotation * (*vel);
			}

			this->addParticleToOwner(particle_list.creator);
			this->numParticles++;
		}

		return particles_created;
	}

	bool ParticleHeap::hasActiveParticles(void* owner_ptr) const
	{
		ParticleCreatorMap::const_iterator it = this->owners.find(owner_ptr);
		if (it != this->owners.end())
		{
			return it->second > 0;
		}
		return false;
	}

	size_t ParticleHeap::removeParticlesByOwner(void* owner_ptr)
	{
		size_t particle_index = 0;
		size_t particle_killed = 0;
		while (particle_index < this->numParticles)
		{
			void** owner = this->buffer->get<void*>(ParticlePropertyOwner, particle_index);
			assert(owner);

			if (*owner == owner_ptr)
			{
				this->buffer->swap(particle_index, this->numParticles - 1);
				this->numParticles--;
				particle_killed++;
				continue;
			}
			particle_index++;
		}

		ParticleCreatorMap::iterator it = this->owners.find(owner_ptr);
		if (it != this->owners.end())
		{
			this->owners.erase(it);
		}

		return particle_killed;
	}

	void ParticleHeap::addParticleToOwner(void* owner_ptr)
	{
		ParticleCreatorMap::iterator it = this->owners.find(owner_ptr);
		if (it == this->owners.end())
		{
			this->owners[owner_ptr] = 1;
			return;
		}
		it->second++;
	}

	void ParticleHeap::removeParticleFromOwner(void* owner_ptr)
	{
		ParticleCreatorMap::iterator it = this->owners.find(owner_ptr);
		if (it != this->owners.end())
		{
			it->second--;
			if (it->second <= 0)
			{
				this->owners.erase(owner_ptr);
			}
		}
	}

	ParticleHeapPtr ParticleHeapCollection::getHeap(ParticleEffectPtr& effect, RenderTraversal traversal)
	{
		ParticleEffectHeapMask mask;
		mask.first = effect;
		mask.second = effect->getMask().mask;

		ParticleHeapMap::iterator it = this->buffers[traversal].find(mask);
		if (it != this->buffers[traversal].end())
		{
			return it->second;
		}

		this->buffers[traversal][mask] = CREATE_CLASS(ParticleHeap, effect);
		return this->buffers[traversal][mask];
	}

	void ParticleHeapCollection::clearUnusedHeaps()
	{
		for (int32 traversal = 0; traversal < RenderTraversalMAX; ++traversal)
		{
			ParticleHeapMap::iterator it = this->buffers[traversal].begin();
			while (it != this->buffers[traversal].end())
			{
				if (it->second->getLinks() == 0 && it->second->isExpired())
				{
					//log::info("Releasing Particle Buffer from effect ", it->first.first->getName());
					it = this->buffers[traversal].erase(it);
				}
				else
				{
					it++;
				}
			}
		}
	}
}

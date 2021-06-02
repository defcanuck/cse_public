#pragma once

#include "ClassDef.h"
#include "global/Singleton.h"

#include "ui/UIElement.h"
#include "ui/UIAnimationPlayer.h"
#include "os/Input.h"
#include "fx/ParticleEmitter.h"

#include "gfx/BatchDraw.h"

#include <unordered_map>
#include <string>

namespace cs
{
	CLASS_DEFINITION(UIDocument)

	public:
		UIDocument(const std::string& name);
		virtual ~UIDocument();

		virtual void init(const RectI& size);
		virtual void populate() { }

		virtual void draw(const RectI& screen_rect, UIBatchPass pass = UIBatchPassMain);
		virtual void process(float32 dt, float32& depth, RectI& screenSize, UIBatchPass pass = UIBatchPassMain);
		virtual void luaProcess(float32 dt) { }

		UIElementPtr& getRoot() { return this->root; }

		const std::string& getName() const { return this->documentName; }

		size_t updateVertices(uchar* data, size_t bufferSize, VertexDeclaration& decl);
		size_t updateIndices(uchar* data, size_t bufferSize);
		void setDrawParams(int32 index, std::vector<DrawCallPtr>& dcs);

		size_t getNumVertices();
		size_t getNumIndices();

		bool onCursor(ClickInput input, ClickParams& params, UIClickResults& results);
		void onCursorMove(const vec2& pos);

		void setDocumentSize(const RectF& view);
		void setDocumentColor(const ColorB& color);

		virtual void luaPopulate() { }
		virtual void luaOnCursor(ClickInput input, TouchState state, vec2 position) { }
		virtual void luaOnCursorMove(vec2 pos) { }

		UIAnimationPlayer animations;
		ParticleEmitterScope fx;

		void setSortOrder(int32 order) { this->sortOrder = order; }
		int32 getSortOrder() const { return this->sortOrder; }

		void addEmitter(UIElementPtr& element, const std::string& effectName, const vec3& pos, bool preDraw = false);
		void addEmitter(UIElementPtr& element, const std::string& effectName, float32 zOffset, bool preDraw = false);
		void removeEmitter(UIElementPtr& element);
		void removeAllEmitters();

		void setLocked(bool isOn) { this->locked = isOn; }
		bool getLocked() const { return this->locked; }

		void setAnimationMode(bool anim);
		void setPendingRemove(bool pend) { this->pendingRemoval = pend; }
		bool getPendingRemove() const { return this->pendingRemoval; }

		void setConsumeClicks(bool b) { this->consumeClicks = b; }

		void setTint(const ColorB& t) { this->tint = t; }

		void preload();

	private:

		void addEmitterInternal(UIElementPtr& element, const std::string& effectName, ParticleEmitter::GetWorldPositionFunc& func, bool preDraw = false);
		void onStartRendering();
		
		std::string documentName;

		const static uint32 kInitNumElements = 1000;
		friend class UIDocumentCache;

		int32 sortOrder;
		UIElementPtr root;
		BatchDrawPtr batch[UIBatchPassMAX];
		bool locked;
		bool pendingRemoval;
		bool animMode;
		bool consumeClicks;
		ColorB tint;

	};

	class UIDocumentCache : public Singleton <UIDocumentCache>
	{
	public:

		UIDocumentPtr createNewDocument(const std::string& documentName);
		void removeDocument(UIDocumentPtr& doc);
		bool documentExists(const std::string& documentName) { return this->cache.find(documentName) != this->cache.end(); }

	private:

		UIDocumentPtr& getDocument(const std::string& name);
		void addDocument(UIDocumentPtr& doc);
		

		typedef std::unordered_map<std::string, UIDocumentPtr> UIDocumentCacheMap;
		UIDocumentCacheMap cache;

	};
}
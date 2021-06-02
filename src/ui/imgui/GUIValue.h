#pragma once

#include <string>

#include "ui/behavior/UIClickBehavior.h"

#include "ui/imgui/GUIFields.h"
#include "ui/imgui/GUIOpenLink.h"

#include "global/Event.h"
#include "scene/SceneReference.h"
#include "global/PropertySet.h"
#include "gfx/Color.h"
#include "animation/spine/SpineAnimation.h"
#include "animation/spine/SpineRenderable.h"
#include "gfx/Mesh.h"

#include "os/FileManager.h"
#include "global/ResourceFactory.h"

#include "imgui.h"

#define DEFINE_GUI_VALUE_SPEC(type) \
	template <> \
	GUIValue* getGUIValue(GUIMemberInfo& info, type* value)

#define DEFINE_GUI_VALUE_DEF(type, gui_type) \
	template <> \
	GUIValue* getGUIValue(GUIMemberInfo& info, type* value) \
	{ \
		return new gui_type(value); \
	}

#define DEFINE_GUI_VALUE_DEF_LABELED(type, gui_type) \
	template <> \
	GUIValue* getGUIValue(GUIMemberInfo& info, type* value) \
		{ \
		return new gui_type(info, value); \
		}


#define DEFINE_GUI_RANGE_SPEC(value_type) \
	template <> \
	void setGUIValueLimit(GUIValue* value, const value_type& limitValue, Member::MemberValueType type)


#define DEFINE_GUI_RANGE_DEF(value_type, gui_type) \
	template <> \
	void setGUIValueLimit(GUIValue* value, const value_type& limitValue, Member::MemberValueType type) \
	{ \
		gui_type* gui = reinterpret_cast<gui_type*>(value); \
		switch (type) \
		{ \
			case Member::MemberValueMin: gui->min = limitValue; break; \
			case Member::MemberValueMax: gui->max = limitValue; break; \
		} \
	}

// 

#define DEFINE_GUI_RANGE_IMPL(type, slider, ranged_func) \
	template <> \
	bool GUIValueRanged<type>::batch(const RectF& rect) \
	{ \
		this->preBatch(); \
		bool ret = false; \
		if (this->info.member && this->info.member->getUseSlider()) \
			ret = ImGui::slider(this->getName(), this->value, this->min, this->max); \
		else \
			ret = ImGui::ranged_func(this->getName(), this->value); \
		this->postBatch(ret); \
		return ret; \
	}
	
namespace cs
{

    template <class T>
    bool openFilePath(std::string& fileName, std::string& filePath, StringList desc = StringList(), StringList ext = StringList())
    {
        filePath = cs::openFileLoadDialog(desc, ext);
        if (filePath.length() == 0)
            return false;
        
        cs::sdl::consumeEvent();
        return FileManager::getInstance()->addPathToFile(filePath, fileName);
    }
    
	class Transform;

	struct GUIMemberInfo
	{
		GUIMemberInfo() 
			: member(nullptr)
			, ptr(nullptr)
			, indent(0.0f)
		{ }

		GUIMemberInfo(const Member* m, void* p, float32 ident = 0.0f)
			: member(m)
			, ptr(p) 
			, indent(ident) 
		{ }

		GUIMemberInfo(const GUIMemberInfo& m)
			: member(m.member)
			, ptr(m.ptr)
			, callbacks(m.callbacks)
			, indent(m.indent) 
		{ }

		void operator=(const GUIMemberInfo& m)
		{
			this->member = m.member;
			this->ptr = m.ptr;
			this->callbacks = m.callbacks;
			this->indent = m.indent;
		}

		void addCallback(Member::MemberCallbackType type, OnMemberUpdateFunc func)
		{
			if (this->callbacks.find(type) == this->callbacks.end())
				this->callbacks[type] = std::vector<OnMemberUpdateFunc>();
			this->callbacks[type].push_back(func);
		}

		const Member* member;
		void* ptr;
		MemberUpdateCallbackMap callbacks;
		float32 indent;
	};

	struct GUIValue
	{
		GUIValue()
			: info() { }
		GUIValue(GUIMemberInfo& i)
			: info(i) { }

		virtual ~GUIValue() 
		{

		}

		virtual bool batch(const RectF& rect) = 0;

		virtual void onPreUpdate();
		virtual bool onUpdate();

		inline const char* getName()
		{
			static const char* kUnknown = "UNKNOWN";
			if (!this->info.member)
				return kUnknown;

			return this->info.member->getName().c_str();
		}

		void preBatch();
		void postBatch(bool update = false);

		bool callCallbacks(Member::MemberCallbackType callbackType);

		typedef std::vector<std::pair<int32, ImVec4>> GUIStyleColors;


		GUIMemberInfo info;
		GUIStyleColors colors;
	};

	struct GUIValueLabel : public GUIValue
	{
		GUIValueLabel(GUIMemberInfo& i, const char** msg = nullptr)
			: GUIValue(i)
			, text(*msg) { }
		GUIValueLabel(const std::string& str)
			: text(str) { }
		GUIValueLabel(const char* t)
			: text(std::string(t)) { }

		virtual bool batch(const RectF& rect);

		std::string text;
	};

	struct GUIValueText : public GUIValue
	{
		const static int32 kImGuiTextInputFlags = ImGuiInputTextFlags_CallbackCharFilter | ImGuiInputTextFlags_EnterReturnsTrue;

		GUIValueText(GUIMemberInfo& i, std::string* str, const std::string& l = "")
			: GUIValue(i)
			, text(str)
			, text_c(nullptr)
			, label(l)
			, customFlags(kImGuiTextInputFlags) { }
		
		GUIValueText(GUIMemberInfo& i, char** str, const std::string& l = "")
			: GUIValue(i)
			, text()
			, text_c(*str) 
			, label(l)
			, customFlags(kImGuiTextInputFlags){ }


		GUIValueText(std::string* str, const std::string& l = "")
			: text(str) 
			, text_c(nullptr)
			, label(l)
			, customFlags(kImGuiTextInputFlags) { }


		GUIValueText(char** str, const std::string& l = "")
			: text()
			, text_c(*str)
			, label(l)
			, customFlags(kImGuiTextInputFlags) { }
		
		virtual bool batch(const RectF& rect);

		static int onImGuiTextInput(ImGuiTextEditCallbackData* data);

		std::string *text;
		char* text_c;
		std::string label;

		ImGuiInputTextFlags customFlags;
	};

	template <class T>
	struct GUIValueEnum : public GUIValue
	{
		GUIValueEnum(GUIMemberInfo& i, T* ptr)
			: GUIValue(i)
			, value(ptr) 
		{
		
		}

		virtual bool batch(const RectF& rect);

		T* value;
	};

	struct GUIValueButton : public GUIValue
	{
		GUIValueButton(const char* t, CallbackPtr& oc, vec2 sz = vec2(0.0f, 0.0f))
			: text(t)
			, size(sz)
		{
			this->onClick += oc;
		}

		GUIValueButton(const char* t, CallbackList& oc, vec2 sz = vec2(0.0f, 0.0f))
			: text(t)
			, size(sz)
		{
			this->onClick += oc;
		}

		GUIValueButton(const std::string& t, CallbackPtr& oc, vec2 sz = vec2(0.0f, 0.0f))
			: text(t)
			, size(sz)
		{
			this->onClick += oc;
		}

		GUIValueButton(const std::string& t, CallbackList& oc, vec2 sz = vec2(0.0f, 0.0f))
			: text(t)
			, size(sz)
		{
			this->onClick += oc;
		}

		GUIValueButton(GUIMemberInfo& i, CallbackPtr& oc, vec2 sz = vec2(0.0f, 0.0f))
			: text(i.member->getName())
			, size(sz)
		{ 
			this->onClick += oc;
		}

		GUIValueButton(GUIMemberInfo& i, CallbackList& oc, vec2 sz = vec2(0.0f, 0.0f))
			: text(i.member->getName())
			, size(sz)
		{
			this->onClick += oc;
		}

		virtual ~GUIValueButton()
		{
			// log::info("Unloading ", text);
		}
			
		virtual bool batch(const RectF& rect);

		std::string text;
		vec2 size;
		Event onClick;
		
	};

	template <typename T, int MaxValue>
	struct GUIValueBitMask : public GUIValue
	{
		GUIValueBitMask(GUIMemberInfo& i, BitMask<T, MaxValue>* ptr)
			: GUIValue(i)
			, value(ptr) { }

		virtual bool batch(const RectF& rect) 
		{ 
			bool ret = false;
			ImColor color(255, 128, 0);
			this->collapsed = !ImGui::CollapsingHeader(this->getName(), this->getName(), true, true);
			this->preBatch();

			if (!this->collapsed)
			{
				const Member::ComboxBoxValues& values = this->info.member->getComboMetaValues();
				for (auto it : values)
				{
					const Member::MetaDataCombo& combo = it.second;
					auto i = reinterpret_cast<std::uintptr_t>(combo.meta_data);
					T flag = static_cast<T>(i);
					bool bVal = this->value->test(flag);
					if (ImGui::Checkbox(it.first.c_str(), &bVal))
					{
						if (bVal)
						{
							this->value->set(flag);
						}
						else
						{
							this->value->unset(flag);
						}
						ret = true;
					}
				}
			
			}
			this->postBatch(ret);
			return ret;
		}

		BitMask<T, MaxValue>* value;
		bool collapsed;
	};

	template <typename T>
	struct GUIValueRanged : public GUIValue
	{
		GUIValueRanged(GUIMemberInfo& i, T* ptr)
			: GUIValue(i)
			, value(ptr)
			, min(T(0))
			, max(T(100)) 
		{ 
			if (this->min < 0)
			{
				log::info("BREAK");
			}
		}

		virtual bool batch(const RectF& rect);

		T* value;
		T min;
		T max;
	
	};

	template <typename T>
	struct GUIValueRect : public GUIValue
	{
		GUIValueRect(GUIMemberInfo& i, Rect<T>* ptr)
			: GUIValue(i)
			, value(ptr) { }

		virtual bool batch(const RectF& rect);

		Rect<T>* value;
	};

	struct GUIValueBool : public GUIValue
	{
		GUIValueBool(GUIMemberInfo& i, bool* bptr)
			: GUIValue(i)
			, value(bptr) { }

		virtual bool batch(const RectF& rect);

		bool* value;
	};

	struct GUIValueVec2 : public GUIValue
	{
		GUIValueVec2(GUIMemberInfo& i, vec2* vecptr)
			: GUIValue(i)
			, value(vecptr) { }

		GUIValueVec2(GUIMemberInfo& i, SizeF* szptr)
			: GUIValue(i)
			, value(reinterpret_cast<vec2*>(szptr)) { }

		virtual bool batch(const RectF& rect);

		vec2* value;
	};

	struct GUIValueSize : public GUIValue
	{
		GUIValueSize(GUIMemberInfo& i, SizeI* szptr)
			: GUIValue(i)
			, value(szptr) { }

		virtual bool batch(const RectF& rect);

		SizeI* value;
	};

	struct GUIValueVec3 : public GUIValue
	{
		GUIValueVec3(GUIMemberInfo& i, vec3* vecptr)
			: GUIValue(i)
			, value(vecptr) { }

		virtual bool batch(const RectF& rect);

		vec3* value;
	};

	struct GUIValueVec4 : public GUIValue
	{
		GUIValueVec4(GUIMemberInfo& i, vec4* vecptr)
			: GUIValue(i)
			, value(vecptr) { }

		virtual bool batch(const RectF& rect);

		vec4* value;
	};

	struct GUIValueTransform : public GUIValue
	{
		GUIValueTransform(GUIMemberInfo& i, Transform* tptr)
			: GUIValue(i)
			, value(tptr) { }

		virtual bool batch(const RectF& rect);

		Transform* value;
		vec3 euler_angle;

	};

	template <class T>
	struct GUIValueColor : public GUIValue
	{
		GUIValueColor(GUIMemberInfo& i, Color<ColorChannel<T>>* tptr)
			: GUIValue(i)
			, value(tptr) { }

		virtual bool batch(const RectF& rect)
		{
			this->preBatch();
			ColorF color = this->toVecColorF(this->value);
			bool ret = ImGui::ColorEdit4(this->getName(), (float32*) &color, true);
			if (ret)
			{
				*this->value = this->toTemplateColor(color);
			}
			this->postBatch(ret);

			return ret;
		}

		ColorF toVecColorF(Color<ColorChannel<T>>* value);
		Color<ColorChannel<T>> toTemplateColor(ColorF& color);

		Color<ColorChannel<T>>* value;
	};

	template <>
	ColorF GUIValueColor<float32>::toVecColorF(Color<ColorChannel<float32>>* value);

	template <>
	Color<ColorChannel<float32>> GUIValueColor<float32>::toTemplateColor(ColorF& color);

	template <>
	ColorF GUIValueColor<uchar>::toVecColorF(Color<ColorChannel<uchar>>* value);

	template <>
	Color<ColorChannel<uchar>> GUIValueColor<uchar>::toTemplateColor(ColorF& color);

	struct GUIValueSerializableCombo
	{
	public:
		GUIValueSerializableCombo(const GUIMemberInfo& info, CallbackPtr oc = nullptr);
		virtual ~GUIValueSerializableCombo();

		bool batch(const RectF& rect);
		const std::string& getSelectedLabel() const
		{
			assert(this->selected < this->labels.size());
			return this->labels[this->selected];
		}

		const MetaData* getSelectedMeta()
		{
			assert(this->member != nullptr);
			return this->member->getComboMeta(this->getSelectedLabel());
		}

		CallbackPtr onClick;
		StringList labels;
		Member* member;
		char** values;
		size_t numValues;
		size_t selected;
	};

	struct GUIValueSerializable : public GUIValue
	{
		GUIValueSerializable(GUIMemberInfo& i, SerializablePtr* vptr)
			: GUIValue(i)
			, ptr(*vptr)
			, data_ptr((*vptr).get())
			, combo(nullptr)
			, collapsed(false)
		{
			this->init();
		}

		GUIValueSerializable(GUIMemberInfo& i, Serializable* vptr)
			: GUIValue(i)
			, ptr(nullptr)
			, data_ptr(vptr)
			, combo(nullptr)
			, collapsed(false)
		{
			this->init();
		}

		virtual ~GUIValueSerializable();

		bool onPress();

		void init();
		void refresh();
		virtual bool batch(const RectF& rect);

		std::shared_ptr<Serializable> ptr;
		Serializable* data_ptr;

		GUIFields fields;
		GUIValueSerializableCombo* combo;
		bool collapsed;
	};

	struct GUIValueTexture : public GUIValue
	{
		GUIValueTexture(GUIMemberInfo& i, TexturePtr* tptr)
			: GUIValue(i)
			, ptr(*tptr)
		{ 
			TexturePtr& tex = *tptr;
			this->handle = CREATE_CLASS(TextureHandle, tex);
		}


		virtual bool batch(const RectF& rect);

		TexturePtr ptr;
		TextureHandlePtr handle;
	};


	struct GUIValueShaderResource : public GUIValue
	{
		GUIValueShaderResource(GUIMemberInfo& i, ShaderResourcePtr* tptr)
			: GUIValue(i)
			, ptr(*tptr)
			, labels(nullptr)
			, index(-1)
		{
			ShaderResourcePtr& shader = *tptr;
			this->handle = CREATE_CLASS(ShaderHandle, shader);
			this->init();
		}
		virtual ~GUIValueShaderResource();

		void init();
		virtual bool batch(const RectF& rect);

		ShaderResourcePtr ptr;
		ShaderHandlePtr handle;
		
		char** labels;
		int32 numLabels;

		int32 index;
	};

	struct GUIValueSceneReference : public GUIValue
	{
		GUIValueSceneReference(GUIMemberInfo& i, SceneReferencePtr* rptr)
			: GUIValue(i)
			, ptr(*rptr)
		{
			SceneReferencePtr& ref = *rptr;
			this->handle = CREATE_CLASS(SceneReferenceHandle, ref);
		}

		virtual bool batch(const RectF& rect);

		SceneReferencePtr ptr;
		SceneReferenceHandlePtr handle;
	};

	template <class Res, class Handle, const char* FileExtension>
	struct GUIValueResource : public GUIValue
	{
		GUIValueResource(GUIMemberInfo& i, std::shared_ptr<Res>* rptr)
			: GUIValue(i)
			, ptr(*rptr)
		{
			std::shared_ptr<Res>& ref = *rptr;
			this->handle = std::make_shared<Handle>(ref);
		}

		virtual bool batch(const RectF& rect)
		{
			bool ret = false;
			ImVec2 sz(rect.size.w, 0.0f);

			std::string name = "empty";
			if (this->ptr)
				name = this->ptr->getName();

			this->preBatch();

			if (ImGui::Button("Clear Handle"))
			{
				this->handle->clear();
				this->ptr = nullptr;
				this->onUpdate();
				ret = true;
			}
			else if (ImGui::Button(name.c_str()))
			{
				std::string fileName, filePath;
				StringList desc;
				StringList ext;
				
				const std::string& strName = MetaCreator<Res>::get()->getName();
				desc.push_back(strName);
				ext.push_back(FileExtension);

				if (!openFilePath<Res>(fileName, filePath, desc, ext))
				{
					this->postBatch();
					return false;
				}

				this->onPreUpdate();

				std::shared_ptr<Res> resource = std::static_pointer_cast<Res>(
					ResourceFactory::getInstance()->loadResource<Res>(fileName));

				this->handle = std::make_shared<Handle>(this->ptr);
				void* data = PTR_ADD(this->info.ptr, this->info.member->getOffset());
				std::shared_ptr<Res>* resourcePtr = reinterpret_cast<std::shared_ptr<Res>*>(data);

				assert(resource);

				this->ptr = resource;
				*resourcePtr = this->ptr;

				this->onUpdate();

				ret = true;

			}

			if (this->ptr && GUIOpenLink::getInstance()->hasMetaOpenLink(this->info.member->getMetaData()))
			{
				std::stringstream str;
				str << "Edit " << name;
				if (ImGui::Button(str.str().c_str(), ImVec2(0.0f, 0.0f)))
				{
					GUIOpenLink::getInstance()->invokeLinkMap(this->info.member->getMetaData(), name);
				}
			}

			this->postBatch(ret);

			return ret;
		}

		std::shared_ptr<Res> ptr;
		std::shared_ptr<Handle> handle;
	};

	struct GUIValueVector : public GUIValue
	{
		GUIValueVector(GUIMemberInfo& i, SerializableVector* vptr)
			: GUIValue(i)
			, vec(*vptr)
			, vec_ptr(vptr)
			, combo(nullptr)
			, collapsed(false)
		{
			this->init();
		}

		void init();
		void refresh();

		virtual ~GUIValueVector()
		{
			if (this->combo)
				delete this->combo;
			this->clearFields();
		}

		virtual bool batch(const RectF& rect);
		void clearFields();

		SerializableVector vec;
		SerializableVector* vec_ptr;
		GUIValueSerializableCombo* combo;
		std::vector<GUIFields*> fields_vector;
		bool collapsed;
	};

	struct GUIValueSpineRenderable : public GUIValueSerializable
	{
		GUIValueSpineRenderable(GUIMemberInfo& i, SpineRenderablePtr* vptr)
			: GUIValueSerializable(i, reinterpret_cast<SerializablePtr*>(vptr))
			, renderable(*vptr)
			, animationValues(nullptr)
			, numValues(0)
			, selected(0)
			, speed(1.0f)
		{ 
			void initAnimations();
		}

		virtual ~GUIValueSpineRenderable()
		{
			this->clearAnimations();
		}

		void initAnimations();
		void clearAnimations();

		virtual bool batch(const RectF& rect);

		SpineRenderablePtr renderable;

		char** animationValues;
		size_t numValues;
		size_t selected;
		float32 speed;
		
	};

	template <class Key>
	struct GUIValueMap : public GUIValue
	{
		GUIValueMap(GUIMemberInfo& i, std::map<Key, SerializablePtr>* vptr)
			: GUIValue(i)
			, map(*vptr)
			, collapsed(false)
		{
			this->init();
		}

		virtual ~GUIValueMap()
		{
			this->clearFields();
		}

		void init()
		{
			if (this->map.size() == 0)
				return;

			this->refresh();
		}

		void clearFields()
		{
			for (auto& it : this->fields_vector)
			{
				delete it;
			}
			this->fields_vector.clear();
		}

		void refresh()
		{
			this->clearFields();
			for (auto& it : this->map)
			{
				SerializablePtr& ptr = it.second;
				if (!ptr)
				{
					continue;
				}

				this->fields_vector.push_back(new GUIFields());
				GUIFields* fields = this->fields_vector.back();

				const MetaData* metadata = ptr->getMetaData();
				void* data_ptr = ptr.get();
				GUICreatorFactory::populate(metadata, data_ptr, fields, this->info.indent);

			}
		}

		bool batch(const RectF& rect)
		{
			bool ret = false;

			ImColor color(255, 128, 0);
			if (this->info.member->getCollapseable())
			{
				bool startOpen = !this->info.member->getStartCollapsed();
				this->collapsed = !ImGui::CollapsingHeader(this->getName(), this->getName(), true, startOpen);
			}
			else
			{	
				ImGui::TextColored(color, this->getName());
			}

			if (this->fields_vector.size() == 0)
				return false;

			if (!this->collapsed)
			{

				for (size_t i = 0; i < this->fields_vector.size(); ++i)
				{
					GUIFields& fields = *this->fields_vector[i];
					std::map<Key, SerializablePtr>::iterator it = this->map.begin();
					std::advance(it, i);
					SerializablePtr& ptr = it->second;

					if (!ptr)
						continue;

					const MetaData* meta = ptr->getMetaData();

					ImGui::BeginGroup();
					std::stringstream str;
					str << "map_" << i;
					ImGui::PushID(str.str().c_str());

					str = std::stringstream();
					str << "[" << i << " - " << it->first << "] " << meta->getName();
					ImGui::TextColored(color, str.str().c_str());

					ret |= fields.batch(rect);

					ImGui::PopID();
					ImGui::EndGroup();
					ImGui::Separator();
				}
			}

			return ret;
		}

		std::map<Key, SerializablePtr> map;
		std::vector<GUIFields*> fields_vector;
		bool collapsed;
	};

	template <class T>
	GUIValue* getGUIValue(GUIMemberInfo& i, T* value)
	{
		// Unknown value
		return nullptr;
	}

	template <class T>
	void setGUIValueLimit(GUIValue* value, const T& limitValue, Member::MemberValueType type)
	{
		assert(false);
	}

	struct GUINullVal 
	{ 
		bool operator==(const GUINullVal& rhs) { return false; }
	};
	
	template <>
	GUIValue* getGUIValue(GUIMemberInfo& i, GUINullVal* value);

	DEFINE_GUI_VALUE_SPEC(float32);
	DEFINE_GUI_RANGE_SPEC(float32);

	DEFINE_GUI_VALUE_SPEC(int32);
	DEFINE_GUI_RANGE_SPEC(int32);

	DEFINE_GUI_VALUE_SPEC(bool);
	DEFINE_GUI_VALUE_SPEC(vec2);
	DEFINE_GUI_VALUE_SPEC(vec3);
	DEFINE_GUI_VALUE_SPEC(vec4);
	
	DEFINE_GUI_VALUE_SPEC(SizeF);
	DEFINE_GUI_VALUE_SPEC(SizeI);

	DEFINE_GUI_VALUE_SPEC(ColorB);
	DEFINE_GUI_VALUE_SPEC(ColorF);

	DEFINE_GUI_VALUE_SPEC(std::string);
	DEFINE_GUI_VALUE_SPEC(const char*);
	DEFINE_GUI_VALUE_SPEC(Transform);
	DEFINE_GUI_VALUE_SPEC(RectF);
	DEFINE_GUI_VALUE_SPEC(RectI);

	DEFINE_GUI_VALUE_SPEC(RenderTraversalMask);

	DEFINE_GUI_VALUE_SPEC(Serializable);
	DEFINE_GUI_VALUE_SPEC(SerializablePtr);
	DEFINE_GUI_VALUE_SPEC(SerializableVector);
	DEFINE_GUI_VALUE_SPEC(SerializableMap);
	DEFINE_GUI_VALUE_SPEC(SerializableMapString);

	// RESOURCES ADD HERE!
	DEFINE_GUI_VALUE_SPEC(TexturePtr);
	DEFINE_GUI_VALUE_SPEC(SceneReferencePtr);
	DEFINE_GUI_VALUE_SPEC(MeshPtr);
	DEFINE_GUI_VALUE_SPEC(LuaScriptFilePtr);
	DEFINE_GUI_VALUE_SPEC(PropertySetResourcePtr);
	DEFINE_GUI_VALUE_SPEC(ShaderResourcePtr);
	DEFINE_GUI_VALUE_SPEC(ParticleEffectPtr);
	DEFINE_GUI_VALUE_SPEC(SpineSkeletonDataPtr);
	DEFINE_GUI_VALUE_SPEC(SpineTextureAtlasPtr);
	DEFINE_GUI_VALUE_SPEC(SpineRenderablePtr);
	DEFINE_GUI_VALUE_SPEC(SoundEffectPtr);

}

#include "PCH.h"

#include "ui/imgui/GUIValue.h"
#include "ui/imgui/GUIMeta.h"

#include "global/Utils.h"
#include "math/Transform.h"
#include "math/Rect.h"

#include "sdl/SDL_Helpers.h"
#include "os/FileDialog.h"
#include "global/ResourceFactory.h"
#include "global/NotificationCenter.h"
#include "scripting/LuaScriptHandle.h"
#include "fx/ParticleEffect.h"
#include "gfx/Renderable.h"

#include "ui/imgui/GUIContext.h"

#include "imgui.h"

namespace cs
{
	const static int32 kFloatPrecision = 4;

	template <>
	ColorF GUIValueColor<float32>::toVecColorF(Color<ColorChannel<float32>>* value)
	{
		return *value;
	}

	template <>
	Color<ColorChannel<float32>> GUIValueColor<float32>::toTemplateColor(ColorF& color)
	{
		return color;
	}

	template <>
	ColorF GUIValueColor<uchar>::toVecColorF(Color<ColorChannel<uchar>>* value)
	{
		return toColorF(*value);
	}

	template <>
	Color<ColorChannel<uchar>> GUIValueColor<uchar>::toTemplateColor(ColorF& color)
	{
		return toColorB(color);
	}

	void GUIValue::preBatch()
	{
		for (const auto it : this->colors)
		{
			const std::pair<int32, ImVec4>& color = it;
			ImGui::PushStyleColor((ImGuiCol)color.first, color.second);
		}
	}

	void GUIValue::postBatch(bool update)
	{
		ImGui::PopStyleColor(int(this->colors.size()));
		if (update)
		{
			this->onUpdate();
		}
	}

	bool GUIValue::callCallbacks(Member::MemberCallbackType callbackType)
	{
		MemberUpdateCallbackMap::iterator it = this->info.callbacks.find(callbackType);
		if (it == this->info.callbacks.end() || it->second.size() == 0)
		{
			return false;
		}

		for (auto call : it->second)
			call();

		return true;
	}

	void GUIValue::onPreUpdate()
	{
		this->callCallbacks(Member::MemberCallbackPre);
	}

	bool GUIValue::onUpdate()
	{
		NotificationCenter::getInstance()->triggerNotification("onSelectedDirty");
		return this->callCallbacks(Member::MemberCallbackPost);
	}

	bool GUIValueLabel::batch(const RectF& rect)
	{
		this->preBatch();
		ImGui::Text(this->text.c_str());
		this->postBatch();
		return false;
	}

	int GUIValueText::onImGuiTextInput(ImGuiTextEditCallbackData* data)
	{
		GUIValueText* valueText = (GUIValueText*)data->UserData;
		// todo filtering
		return 0;
	}

	bool GUIValueText::batch(const RectF& rect)
	{
		const size_t kMaxTextBufferSize = 1024;
		char buffer[kMaxTextBufferSize];
		char* useBuffer;

		if (this->text_c)
		{
			useBuffer = this->text_c;
		}
		else
		{
			strcpy(buffer, (*this->text).c_str());
			useBuffer = buffer;
		}

		std::string useLabel = this->label;
		if (this->info.member)
			useLabel = this->info.member->getName();

		bool ret = false;

		this->preBatch();

		if (this->info.member && this->info.member->getIsLocked())
		{
			ImGui::TextColored(ImVec4(0.5f, 0.0f, 1.0f, 1.0f), buffer);
		} 
		else
		{
			if (ImGui::InputText(useLabel.c_str(), buffer, kMaxTextBufferSize, this->customFlags, &GUIValueText::onImGuiTextInput, (void*) this))
			{
				if (!this->text_c)
				{
					*this->text = std::string(buffer);
				}
				ret = true;
			}
		}
		this->postBatch(ret);

		return ret;
	}

	bool GUIValueButton::batch(const RectF& rect)
	{
	
		this->preBatch();

		bool ret = false; 
		if (ImGui::Button(this->text.c_str(), ImVec2(size.x, size.y)))
		{
			log::print(LogInfo, "Button Clicked");
			this->onClick.invoke();
			ret = true;
		}

		this->postBatch(ret);
	
		return ret;
	}

	bool GUIValueBool::batch(const RectF& rect)
	{
		bool ret = false;
		this->preBatch();
		ret = ImGui::Checkbox(this->getName(), this->value);
		this->postBatch(ret);

		return ret;
	}

	bool GUIValueVec2::batch(const RectF& rect)
	{
		bool ret = false;
		this->preBatch();
		ret = ImGui::InputFloat2(this->getName(), reinterpret_cast<float*>(this->value), 4, ImGuiInputTextFlags_CharsDecimal);
		this->postBatch(ret);
		return ret;
	}

	bool GUIValueSize::batch(const RectF& rect)
	{
		bool ret = false;
		this->preBatch();
		ret = ImGui::InputInt2(this->getName(), reinterpret_cast<int*>(this->value));
		this->postBatch(ret);
		return ret;
	}

	bool GUIValueVec3::batch(const RectF& rect)
	{
		bool ret = false;
		this->preBatch();
		ret = ImGui::InputFloat3(this->getName(), reinterpret_cast<float*>(this->value), 4, ImGuiInputTextFlags_CharsDecimal);
		this->postBatch(ret);
		return ret;
	}

	bool GUIValueVec4::batch(const RectF& rect)
	{
		bool ret = false;
		this->preBatch();
		ret = ImGui::InputFloat4(this->getName(), reinterpret_cast<float*>(this->value), 4, ImGuiInputTextFlags_CharsDecimal);
		this->postBatch(ret);
		return ret;
	}

	bool GUIValueTransform::batch(const RectF& rect)
	{
		bool ret = false;

		this->preBatch();

		ImGui::Text(this->getName());
		ret |= ImGui::InputFloat3("pos", reinterpret_cast<float*>(&this->value->position), kFloatPrecision, ImGuiInputTextFlags_CharsDecimal);
		ret |= ImGui::InputFloat3("scale", reinterpret_cast<float*>(&this->value->scale), kFloatPrecision, ImGuiInputTextFlags_CharsDecimal);

		
		this->euler_angle = glm::eulerAngles(this->value->rotation);
		if (ImGui::InputFloat3("rotation", reinterpret_cast<float*>(&this->euler_angle), kFloatPrecision, ImGuiInputTextFlags_CharsDecimal))
		{
			this->value->rotation = glm::quat(this->euler_angle);
			ret = true;
		}

		if (ret)
		{
			this->value->refresh();
		}
		this->postBatch(ret);

		return ret;
	}

	bool GUIValueSceneReference::batch(const RectF& rect)
	{
		bool ret = false;
		ImVec2 sz(rect.size.w, 0.0f);
		
		std::string name = "empty";
		if (this->ptr)
			name = this->ptr->getName();

		this->preBatch();
		if (ImGui::Button(name.c_str()))
		{
			std::string fileName, filePath;
			
			StringList desc;
			StringList ext;

			desc.push_back(SerializableHandle<ReferenceNode>::getDescription());
			ext.push_back(SerializableHandle<ReferenceNode>::getExtension());

			if (!openFilePath<SceneReference>(fileName, filePath, desc, ext))
			{
				this->postBatch();
				return false;
			}

			SceneReferencePtr sceneReference = std::static_pointer_cast<SceneReference>(
				ResourceFactory::getInstance()->loadResource<SceneReference>(fileName));

			if (!sceneReference.get())
			{
				ret = false;
			}
			else
			{
				this->handle = CREATE_CLASS(SceneReferenceHandle, this->ptr);
				void* data = PTR_ADD(this->info.ptr, this->info.member->getOffset());
				SceneReferencePtr* sceneReferencePtr = reinterpret_cast<SceneReferencePtr*>(data);

				assert(sceneReference);

				this->ptr = sceneReference;
				*sceneReferencePtr = this->ptr;

				this->onUpdate();

				NotificationCenter::getInstance()->triggerNotification("onSceneDirty");

				ret = true;
			}
			
		}

		this->postBatch(false);

		return ret;
	}

	bool GUIValueTexture::batch(const RectF& rect)
	{
		// Don't allow target type textures to be altered
		if (this->handle.get() && this->handle->hasTargetType())
		{
			return false;
		}

		ImColor color(0, 128, 255);
		ImVec2 sz(64, 64);

		uint32 w = this->handle->getWidth();
		uint32 h = this->handle->getHeight();

		std::stringstream label;
		label << this->handle->getTextureName().c_str() << "[" << w << "x" << h << "]";
		ImGui::TextColored(color, label.str().c_str());
		
		this->preBatch();

		// Scope our texture handles to ensure they're valid throughn to
		// the end of the ImGui::Draw - if these get released before they're
		// used, the renderer will dump itself
		TextureHandlePtr* scope_ptr = GUIContext::pushTexture(this->handle);
		ImTextureID id = scope_ptr;

		bool ret = false;
		if (ImGui::ImageButton(id, sz) && !this->info.member->getIsLocked())
		{
			std::string fileName, filePath;
			if (!openFilePath<Texture>(fileName, filePath))
			{
				this->postBatch();
				return false;
			}
			this->ptr = std::static_pointer_cast<Texture>(
				ResourceFactory::getInstance()->loadResource<Texture>(fileName));

			this->handle = CREATE_CLASS(TextureHandle, this->ptr);
			void* data = PTR_ADD(this->info.ptr, this->info.member->getOffset());
		
			TexturePtr* memberTexture = reinterpret_cast<TexturePtr*>(data);
			assert(memberTexture);

			*memberTexture = this->ptr;
			this->onUpdate();

			ret = true;
		}

		this->postBatch(false);

		return ret;
	}

	GUIValueShaderResource::~GUIValueShaderResource()
	{
		for (int32 i = 0; i < this->numLabels; ++i)
		{
			delete[] this->labels[i];
			this->labels[i] = nullptr;
		}

		delete[] this->labels;
		this->labels = nullptr;
	}

	void GUIValueShaderResource::init()
	{

		struct local
		{
			static bool checkShaderType(std::shared_ptr<ShaderResource>& shader)
			{
				return shader->getBucket() == ShaderBucketGeometry;
			}
		};

		std::vector<std::string> resources;
		ResourceFactory::getInstance()->getResourceNamesByType<ShaderResource>(resources, local::checkShaderType);
		if (resources.size() == 0)
			return;

		this->labels = new char*[resources.size()];
		this->numLabels = int32(resources.size());

		for (size_t i = 0; i < resources.size(); ++i)
		{
			const std::string& str_label = resources[i];
			if (this->ptr.get() && this->ptr->getName() == str_label)
				this->index = int32(i);

			char* label = new char[str_label.length() + 1];
			strcpy(label, str_label.c_str());
			this->labels[i] = label;
		}
	}

	bool GUIValueShaderResource::batch(const RectF& rect)
	{

		int32 tmpSelected = this->index;
		this->preBatch();
		
		if (ImGui::Combo(this->info.member->getName().c_str(), &tmpSelected, (const char**) this->labels, this->numLabels))
		{
			if (tmpSelected != this->index)
			{
				const std::string shaderName = this->labels[tmpSelected];
				if (ResourceFactory::getInstance()->getResource<ShaderResource>(shaderName, this->ptr))
				{
					this->handle = CREATE_CLASS(ShaderHandle, this->ptr);
					void* data = PTR_ADD(this->info.ptr, this->info.member->getOffset());

					ShaderResourcePtr* memberShader = reinterpret_cast<ShaderResourcePtr*>(data);
					assert(memberShader);

					*memberShader = this->ptr;
					this->index = tmpSelected;
					
					this->onUpdate();
				}

				this->postBatch();
				return true;
			}
		}
		this->postBatch();
		return false;
	}

	GUIValueSerializableCombo::GUIValueSerializableCombo(const GUIMemberInfo& info, CallbackPtr oc)
		: member(const_cast<Member*>(info.member))
		, onClick(oc)
		, values(nullptr)
		, numValues(0)
		, selected(-1)
	{
		
		this->numValues = member->getComboMetaLabels(labels);
		this->values = new char*[this->numValues];


		std::string selectedMeta = "";
		if (info.member->getIsPointer())
		{
			SerializablePtr* serial_ptr = reinterpret_cast<SerializablePtr*>(PTR_ADD(info.ptr, member->getOffset()));
			if (!serial_ptr->get())
			{
				const MetaData* meta = info.member->getMetaData();
				selectedMeta = meta->getName();
			}
			else
			{
				// not populated, just fill with whatever
				const MetaData* meta = (*serial_ptr)->getMetaData();
				selectedMeta = meta->getName();
			}
		} 
		else 
		{
			this->selected = 0;
		}
		
		for (size_t i = 0; i < this->numValues; ++i)
		{
			char* label = new char[labels[i].length() + 1];
			strcpy(label, labels[i].c_str());
			this->values[i] = label;

			const MetaData* member_meta = member->getComboMeta(this->values[i]);
			if (member_meta && (selectedMeta == member_meta->getName() || selectedMeta == this->values[i]))
			{
				this->selected = i;
			}
		}
	}

	GUIValueSerializableCombo::~GUIValueSerializableCombo()
	{
		for (size_t i = 0; i < this->numValues; ++i)
		{
			if (this->values[i])
			{
				delete[] this->values[i];
				this->values[i] = nullptr;
			}
		}
		delete[] this->values;
		this->values = nullptr;
	}

	bool GUIValueSerializableCombo::batch(const RectF& rect)
	{
		int32 tmpSelected = int32(this->selected);
		if (ImGui::Combo(member->getName().c_str(), &tmpSelected, (const char**) this->values, int32(this->numValues)))
		{
			if (tmpSelected != this->selected)
			{
				this->selected = tmpSelected;
				if (this->onClick)
					this->onClick->invoke();

				return true;
			}
		}
		return false;
	}

	void GUIValueSerializable::init()
	{
		if (!this->data_ptr)
			return;
		
		this->refresh();
	}

	void GUIValueSerializable::refresh()
	{
		this->fields.clear();
		if (!this->data_ptr)
		{
			log::error("serializinng nullptr");
			return;
		}
		const MetaData* metadata = this->data_ptr->getMetaData();
		GUICreatorFactory::populate(metadata, (void*) this->data_ptr, &this->fields, info.indent);
	}

	bool GUIValueSerializable::onPress()
	{
		log::info("Selected: ", this->combo->selected);
		const MetaData* new_metadata = this->combo->getSelectedMeta();
		if (new_metadata)
		{
			SerializablePtr new_element = std::shared_ptr<Serializable>(reinterpret_cast<Serializable*>(new_metadata->createNew()));
			new_element->onNew();
			new_element->onPostLoad();

			SerializablePtr* serial_ptr = reinterpret_cast<SerializablePtr*>(PTR_ADD(this->info.ptr, this->info.member->getOffset()));
			(*serial_ptr) = new_element;
			
			this->ptr = new_element;
			this->data_ptr = this->ptr.get();

			this->refresh();
		}
		return true;
	}


	bool GUIValueSerializable::batch(const RectF& rect)
	{

		bool ret = false;
		if (this->info.member->getCollapseable())
		{
			bool startOpen = !this->info.member->getStartCollapsed();
			this->collapsed = !ImGui::CollapsingHeader(this->getName(), this->getName(), true, startOpen);
		}
		
		if (!this->collapsed)
		{
			ImGui::BeginGroup();
			ImGui::PushID(this->getName());

			this->preBatch();
			if (this->info.member->getNumComboMeta() > 0)
			{
				if (!this->combo)
				{
					CallbackPtr onClick = createCallbackArg0(&GUIValueSerializable::onPress, this);
					this->combo = new GUIValueSerializableCombo(this->info, onClick);
				}

				if (this->combo->batch(rect))
				{
					this->onUpdate();
					ret |= true;
				}

			} else {

				if (!this->info.member->getCollapseable())
				{
					ImColor color(255, 128, 0);
					ImGui::TextColored(color, this->getName());
				
					if (!this->info.member->getCollapseable())
						ImGui::Separator();
				}
			}

			ret |= this->fields.batch(rect);

			this->postBatch(false);

			ImGui::PopID();
			ImGui::EndGroup();
		}

		return ret;
		
	}

	GUIValueSerializable::~GUIValueSerializable()
	{
		if (this->combo)
			delete this->combo;
	}

	template <typename T>
	bool GUIValueRanged<T>::batch(const RectF& rect)
	{
		assert(false);
	}

	template <>
	bool GUIValueRect<int32>::batch(const RectF& rect)
	{
		std::string rpos = "pos [" + std::string(this->getName()) + "]";
		std::string rsize = "size [" + std::string(this->getName()) + "]";

		bool ret = false;
		this->preBatch();
		if (ImGui::Button("Center", ImVec2(0.0f, 0.0f)))
		{
			this->value->setCenter();
			ret = true;
		}
		ret |= ImGui::InputInt2(rpos.c_str(), reinterpret_cast<int32*>(&this->value->pos), kFloatPrecision);
		ret |= ImGui::InputInt2(rsize.c_str(), reinterpret_cast<int32*>(&this->value->size), kFloatPrecision);
		this->postBatch(ret);

		return ret;
	}

	template <>
	bool GUIValueRect<float32>::batch(const RectF& rect)
	{
		std::string rpos = "pos [" + std::string(this->getName()) + "]";
		std::string rsize = "size [" + std::string(this->getName()) + "]";
		
		bool ret = false;
		this->preBatch();
		if (ImGui::Button("Center", ImVec2(0.0f, 0.0f)))
		{
			this->value->setCenter();
			ret = true;
		}

		ret |= ImGui::InputFloat2(rpos.c_str(), reinterpret_cast<float32*>(&this->value->pos), kFloatPrecision);
		ret |= ImGui::InputFloat2(rsize.c_str(), reinterpret_cast<float32*>(&this->value->size), kFloatPrecision);
		this->postBatch(ret);
	
		return ret;
	}


	void GUIValueSpineRenderable::initAnimations()
	{
		if (!this->renderable.get())
		{
			return;
		}

		SpineAnimationInstance& instance = this->renderable->getInstance();
		if (instance.skeletonData)
		{
			this->numValues = instance.skeletonData->value->animationsCount;
			this->animationValues = new char*[this->numValues];
			for (size_t i = 0; i < this->numValues; i++)
			{
				const spine::Animation& anim = *instance.skeletonData->value->animations[i];
				std::string anim_name = anim.name;
				char* label_name = new char[anim_name.length() + 1];
				strcpy(label_name, anim_name.c_str());
				if (instance.currentAnimName == anim_name)
				{
					this->selected = i;
				}
				this->animationValues[i] = label_name;
			}
		}
	}

	void GUIValueSpineRenderable::clearAnimations()
	{
		for (size_t i = 0; i < this->numValues; i++)
		{
			delete[] this->animationValues[i];
			this->animationValues[i] = nullptr;
		}
		delete[] this->animationValues;
		this->numValues = 0;
		this->selected = 0;
	}

	bool GUIValueSpineRenderable::batch(const RectF& rect)
	{
		bool ret = GUIValueSerializable::batch(rect);

		int32 tmpSelected = int32(this->selected);
		this->preBatch();

		if (ImGui::Combo("Start Animation", &tmpSelected, (const char**) this->animationValues, int32(this->numValues)))
		{
			if (tmpSelected != this->selected)
			{
				this->selected = tmpSelected;
				std::string selected_animation = this->animationValues[this->selected];
				this->renderable->setAnimation(selected_animation);
			}
		}

		SpineAnimationInstance& instance = this->renderable->getInstance();

		if (instance.skeleton.get())
		{
			if (this->numValues != instance.skeletonData->value->animationsCount)
			{
				this->clearAnimations();
				this->initAnimations();
			}

			ImGui::BeginGroup();
			ImGui::PushID("Preview");

			if (ImGui::Button("Play", ImVec2(0.0f, 0.0f)))
			{
				std::string selected_animation = this->animationValues[this->selected];
				instance.playAnimation(selected_animation, 1.0f, AnimationTypeLoop);
			}

			if (ImGui::SliderFloat("Speed", &this->speed, 0.0f, 2.0f))
			{
				instance.animationSpeed = this->speed;
				this->renderable->setSpeed(this->speed);
			}

			if (ImGui::Button("Pause", ImVec2(0.0f, 0.0f)))
			{
				instance.active = false;
			}

			if (ImGui::Button("Stop", ImVec2(0.0f, 0.0f)))
			{
				instance.active = false;
				instance.currentTime = 0.0f;
			}

			ImGui::PopID();
			ImGui::EndGroup();
		}

		this->postBatch(ret);

		return ret;
	}

	void GUIValueVector::init()
	{
		if (this->vec.size() == 0)
			return;

		this->refresh();
	}

	void GUIValueVector::clearFields()
	{
		for (auto it : this->fields_vector)
		{
			delete it;
		}
		this->fields_vector.clear();
	}

	void GUIValueVector::refresh()
	{
		this->clearFields();
		for (auto it : this->vec)
		{
			SerializablePtr& ptr = it;
			if (!ptr)
			{
				continue;
			}
			
			this->fields_vector.push_back(new GUIFields());
			GUIFields* fields = this->fields_vector.back();
				
			const MetaData* metadata = ptr->getMetaData();
			void* data_ptr = ptr.get();
			GUICreatorFactory::populate(metadata, data_ptr, fields, info.indent);
			
		}
	}

	bool GUIValueVector::batch(const RectF& rect)
	{
		if (this->vec_ptr && this->vec_ptr->size() != this->vec.size())
		{
			this->vec = *this->vec_ptr;
			this->refresh();
		}

		bool isLocked = this->info.member->getIsLocked();
		bool ret = false;
		
		this->preBatch();
		if (this->info.member->getCollapseable())
		{
			bool startOpen = !this->info.member->getStartCollapsed();
			this->collapsed = !ImGui::CollapsingHeader(this->getName(), this->getName(), true, startOpen);
		}
		else
		{
			ImColor color(255, 128, 0);
			ImGui::TextColored(color, this->getName());
		}

		if (!this->collapsed)
		{

			ImColor color(128, 255, 0);
			for (size_t i = 0; i < this->fields_vector.size(); ++i)
			{
				GUIFields& fields = *this->fields_vector[i];
				SerializablePtr& ptr = this->vec[i];

				if (!ptr)
					continue;

				const MetaData* meta = ptr->getMetaData();

				ImGui::BeginGroup();
				std::stringstream str;
				str << "vec_" << i;
				ImGui::PushID(str.str().c_str());
				ImGui::Indent();

				str = std::stringstream();
				str << "[" << i << "] " << meta->getName();
				ImGui::Columns(2);
				ImGui::TextColored(color, str.str().c_str());
				ImGui::NextColumn();

				if (!isLocked)
				{
					if (ImGui::Button("Delete", ImVec2(0.0f, 0.0f)))
					{
						log::info("Delete item at index: ", 0);
						SerializableVector::iterator del_iter = (*this->vec_ptr).begin() + i;
						(*this->vec_ptr).erase(del_iter);
						ret = true;

						this->vec = (*this->vec_ptr);
						this->refresh();
					}
				}

				ImGui::Columns(1);

				if (!ret)
					ret |= fields.batch(rect);

				ImGui::Unindent();
				ImGui::PopID();
				ImGui::EndGroup();
				ImGui::Separator();
			}

			if (!isLocked && this->info.member->getNumComboMeta() > 0)
			{
				ImGui::Separator();
				if (!this->combo)
				{
					this->combo = new GUIValueSerializableCombo(this->info);
				}

				this->combo->batch(rect);
				if (ImGui::Button("Add Item", ImVec2(0.0f, 0.0f)))
				{
					log::print(LogInfo, "Add Item");

					// Allocate data for the newly created item
					const MetaData* new_metadata = this->combo->getSelectedMeta();
					if (new_metadata)
					{

						Serializable* serial_ptr = reinterpret_cast<Serializable*>(new_metadata->createNew());
						serial_ptr->onNew();

						SerializablePtr new_element = std::shared_ptr<Serializable>(serial_ptr);
						SerializableVector* vec_ptr = reinterpret_cast<SerializableVector*>(PTR_ADD(this->info.ptr, this->info.member->getOffset()));

						(*vec_ptr).push_back(new_element);
						this->vec.push_back(new_element);

						this->refresh();
						this->onUpdate();

						ret |= true;
					}
					else
					{
						log::info(this->combo->getSelectedLabel(), " - could not find MetaData!");
					}
				}
			}
		}

		this->postBatch();

		return ret;
	}

	template <>
	GUIValue* getGUIValue(GUIMemberInfo& i, GUINullVal* value)
	{
		const static char* kUnknown = "Unknown";
		return new GUIValueLabel(i, &kUnknown);
	}

	DEFINE_GUI_RANGE_IMPL(float32, SliderFloat, InputFloat);
	DEFINE_GUI_RANGE_IMPL(int32, SliderInt, InputInt);
	
	DEFINE_GUI_VALUE_DEF_LABELED(char*, GUIValueText);
	DEFINE_GUI_VALUE_DEF_LABELED(std::string, GUIValueText);

	DEFINE_GUI_VALUE_DEF_LABELED(float32, GUIValueRanged<float32>);
	DEFINE_GUI_RANGE_DEF(float32, GUIValueRanged<float32>);

	DEFINE_GUI_VALUE_DEF_LABELED(int32, GUIValueRanged<int32>);
	DEFINE_GUI_RANGE_DEF(int32, GUIValueRanged<int32>);

	DEFINE_GUI_VALUE_DEF_LABELED(vec2, GUIValueVec2);
	DEFINE_GUI_VALUE_DEF_LABELED(SizeF, GUIValueVec2);

	DEFINE_GUI_VALUE_DEF_LABELED(SizeI, GUIValueSize);

	DEFINE_GUI_VALUE_DEF_LABELED(ColorB, GUIValueColor<uchar>);
	DEFINE_GUI_VALUE_DEF_LABELED(ColorF, GUIValueColor<float32>);

	DEFINE_GUI_VALUE_DEF_LABELED(vec4, GUIValueVec4);
	DEFINE_GUI_VALUE_DEF_LABELED(vec3, GUIValueVec3);
	DEFINE_GUI_VALUE_DEF_LABELED(bool, GUIValueBool);
	DEFINE_GUI_VALUE_DEF_LABELED(Transform, GUIValueTransform);

	DEFINE_GUI_VALUE_DEF_LABELED(RectF, GUIValueRect<float32>);
	DEFINE_GUI_VALUE_DEF_LABELED(RectI, GUIValueRect<int32>);

	typedef GUIValueBitMask<RenderTraversal, RenderTraversalMAX> GUIValueBitMaskRenderTraversal;
	DEFINE_GUI_VALUE_DEF_LABELED(RenderTraversalMask, GUIValueBitMaskRenderTraversal);

	/*
	template <>
	GUIValue* getGUIValue(GUIMemberInfo& info, RenderTraversalMask* value)
	{
		return new GUIValueBitMaskRenderTraversal(info, value);
	}
	*/

	DEFINE_GUI_VALUE_DEF_LABELED(SerializablePtr, GUIValueSerializable);
	DEFINE_GUI_VALUE_DEF_LABELED(Serializable, GUIValueSerializable);
	DEFINE_GUI_VALUE_DEF_LABELED(SerializableVector, GUIValueVector);
	DEFINE_GUI_VALUE_DEF_LABELED(SerializableMap, GUIValueMap<size_t>);
	DEFINE_GUI_VALUE_DEF_LABELED(SerializableMapString, GUIValueMap<std::string>);

	DEFINE_GUI_VALUE_DEF_LABELED(TexturePtr, GUIValueTexture);
	DEFINE_GUI_VALUE_DEF_LABELED(SceneReferencePtr, GUIValueSceneReference);
	DEFINE_GUI_VALUE_DEF_LABELED(ShaderResourcePtr, GUIValueShaderResource);

	DEFINE_GUI_VALUE_DEF_LABELED(SpineRenderablePtr, GUIValueSpineRenderable);

	extern const char kGUIMeshExtension[] = "obj";
	typedef GUIValueResource<Mesh, MeshHandle, kGUIMeshExtension> GUIMesh;
	DEFINE_GUI_VALUE_DEF_LABELED(MeshPtr, GUIMesh);

	extern const char kGUILuaExtension[] = "lua";
	typedef GUIValueResource<LuaScriptFile, LuaScriptHandle, kGUILuaExtension> GUILuaScriptFile;
	DEFINE_GUI_VALUE_DEF_LABELED(LuaScriptFilePtr, GUILuaScriptFile);

	extern const char kGUIParticleExtension[] = "fx";
	typedef GUIValueResource<ParticleEffect, ParticleEffectHandle, kGUIParticleExtension> GUIParticleEffectFile;
	DEFINE_GUI_VALUE_DEF_LABELED(ParticleEffectPtr, GUIParticleEffectFile);

	extern const char kGUIPropertyExtension[] = "prop";
	typedef GUIValueResource<PropertySetResource, PropertySetHandle, kGUIPropertyExtension> GUIPropertyResourceFile;
	DEFINE_GUI_VALUE_DEF_LABELED(PropertySetResourcePtr, GUIPropertyResourceFile);

	extern const char kGUISpineSkeletonDataExtension[] = "json";
	typedef GUIValueResource<SpineSkeletonData, SpineSkeletonHandle, kGUISpineSkeletonDataExtension> GUISpineSkeletonFile;
	DEFINE_GUI_VALUE_DEF_LABELED(SpineSkeletonDataPtr, GUISpineSkeletonFile);

	extern const char kGUISpineTextureExtension[] = "atlas";
	typedef GUIValueResource<SpineTextureAtlas, SpineAtlasHandle, kGUISpineTextureExtension> GUISpineTextureAtlasFile;
	DEFINE_GUI_VALUE_DEF_LABELED(SpineTextureAtlasPtr, GUISpineTextureAtlasFile);

	extern const char kGUISoundEffectExtension[] = "wav";
	typedef GUIValueResource<SoundEffect, SoundEffectHandle, kGUISoundEffectExtension> GUISoundEffectFile;
	DEFINE_GUI_VALUE_DEF_LABELED(SoundEffectPtr, GUISoundEffectFile);



}

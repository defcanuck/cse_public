#include "PCH.h"

#include "scripting/LuaBindGfx.h"
#include "scripting/LuaMacro.h"

#include <string>
#include <vector>

namespace cs
{
	using namespace luabind;

    BEGIN_DEFINE_LUA_CLASS_SHARED_RENAMED(RenderInterface, Renderer)
        .scope
        [
            def("setBlurPercent", &RenderInterface::setBlurPercent),
            def("setChromaticAbberationStrength", &RenderInterface::setChomaticAbberationStrength)
        ]
    END_DEFINE_LUA_CLASS()

	BEGIN_DEFINE_LUA_ENUM(RenderTargetType)
		.enum_("constants")
		[
			value("Back", RenderTargetTypeBackBuffer),
			value("Copy", RenderTargetTypeCopyBuffer),
			value("CopyHalf", RenderTargetTypeCopyBufferHalf),
			value("CopyQuarter", RenderTargetTypeCopyBufferQuarter)
		]
	END_DEFINE_LUA_ENUM()

	BEGIN_DEFINE_LUA_ENUM(RenderTraversal)
		.enum_("constants")
		[
			value("Shadow", RenderTraversalShadow),
			value("Main", RenderTraversalMain)
		]
	END_DEFINE_LUA_ENUM()

	ADD_META_BITSET(RenderTraversalMask, RenderTraversal, RenderTraversalMAX)

	BEGIN_DEFINE_LUA_CLASS(ColorFList)
		.def(constructor<>())
		//.def("size", &ColorFList::size)
		//.def("push_back", static_cast<void (ColorFList::*)(const ColorF&)>(&ColorFList::push_back))
	END_DEFINE_LUA_CLASS()
			
	BEGIN_DEFINE_LUA_CLASS(ColorF)
		.def(constructor<>())
		.def(constructor<float32, float32, float32, float32>())
		.def(constructor<uint32>())
		.def(constructor<const ColorF&>())
		.def_readwrite("r", &ColorF::r)
		.def_readwrite("g", &ColorF::g)
		.def_readwrite("b", &ColorF::b)
		.def_readwrite("a", &ColorF::a)
	END_DEFINE_LUA_CLASS()
		
	struct local_ColorB
	{
		static ColorB deepCopy(const ColorB& rhs)
		{
			return ColorB(rhs);
		}
	};

	BEGIN_DEFINE_LUA_CLASS(ColorB)
		.def(constructor<>())
		.def(constructor<uint32, uint32, uint32, uint32>())
		.def(constructor<uint32>())
		.def(constructor<const ColorB&>())
		.def_readwrite("r", &ColorB::r)
		.def_readwrite("g", &ColorB::g)
		.def_readwrite("b", &ColorB::b)
		.def_readwrite("a", &ColorB::a)
		.scope
		[
			def("copy", &local_ColorB::deepCopy)
		]
	END_DEFINE_LUA_CLASS()

	BEGIN_DEFINE_LUA_ENUM(DepthType)
	.enum_("constants")
	[
		value("Never", DepthNever),
		value("Equal", DepthEqual),
		value("Less", DepthLess),
		value("LessEqual", DepthLessEqual),
		value("Greater", DepthGreater),
		value("GreaterEqual", DepthGreaterEqual),
		value("NotEqual", DepthNotEqual),
		value("Always", DepthAlways)
	]
	END_DEFINE_LUA_ENUM()

	BEGIN_DEFINE_LUA_ENUM(BlendType)
		.enum_("constants")
		[
			value("Zero", BlendZero),
			value("One", BlendOne),
			value("SourceAlpha", BlendSrcAlpha),
			value("OneMinusSourceAlpha", BlendOneMinusSrcAlpha)
		]
	END_DEFINE_LUA_ENUM()

	BEGIN_DEFINE_LUA_CLASS(DrawOptionsBlend)
		.def("setSourceBlend", &DrawOptionsBlend::setSourceBlend)
		.def("setDestBlend", &DrawOptionsBlend::setDestBlend)
	END_DEFINE_LUA_CLASS()

	BEGIN_DEFINE_LUA_CLASS(DrawOptionsDepth)
		.def("setDepthTest", &DrawOptionsDepth::setDepthTest)
		.def("setDepthWrite", &DrawOptionsDepth::setDepthWrite)
		.def("setDepthType", &DrawOptionsDepth::setDepthType)
	END_DEFINE_LUA_CLASS()

	BEGIN_DEFINE_LUA_CLASS(DrawOptions)
		.def_readwrite("depth", &DrawOptions::depth)
		.def_readwrite("blend", &DrawOptions::blend)
	END_DEFINE_LUA_CLASS()

	DEFINE_DERIVED_VALUES_4(Renderable, BatchRenderable, TrailRenderable, MeshRenderable, SplineRenderable)
	BEGIN_DEFINE_LUA_CLASS_SHARED(Renderable)
		.def(constructor<>())
		.def("setLayer", &Renderable::setLayer)
		.def("getLayer", &Renderable::getLayer)
		.def("getVolume", &Renderable::getVolume)
		.def_readwrite("traversalMask", &Renderable::traversalMask)
		SET_DERIVED_VALUES_4(Renderable, BatchRenderable, TrailRenderable, MeshRenderable, SplineRenderable)
	END_DEFINE_LUA_CLASS()

	BEGIN_DEFINE_LUA_CLASS_SHARED(Shape)
	END_DEFINE_LUA_CLASS()

	BEGIN_DEFINE_LUA_CLASS_DERIVED_SHARED(QuadShape, Shape)
		.def(constructor<>())
		.def(constructor<float32, float32>())
		.def(constructor<const RectF&>())
	END_DEFINE_LUA_CLASS()

	BEGIN_DEFINE_LUA_CLASS_DERIVED_SHARED(ParallelogramShape, QuadShape)
		.def(constructor<>())
		.def(constructor<float32, float32, float32>())
		.def(constructor<const RectF&, float32>())
	END_DEFINE_LUA_CLASS()

	DEFINE_INHERITED_VALUES(BatchRenderable, Renderable)
	BEGIN_DEFINE_LUA_CLASS_DERIVED_SHARED(BatchRenderable, Renderable)
		.def(constructor<>())
		.def("setTexture", (void(BatchRenderable::*)(const std::string&)) &BatchRenderable::setTexture)
		.def("setTexture", (void(BatchRenderable::*)(const TextureHandlePtr&)) &BatchRenderable::setTexture)
		.def("setShape", (void(BatchRenderable::*)(std::shared_ptr<QuadShape>&)) &BatchRenderable::setShape<QuadShape>)
		.def("setShape", (void(BatchRenderable::*)(std::shared_ptr<ParallelogramShape>&)) &BatchRenderable::setShape<ParallelogramShape>)
		.def("setShader", (void(BatchRenderable::*)(const std::string&)) &BatchRenderable::setShader)
		.def("setShader", (void(BatchRenderable::*)(const ShaderHandlePtr&)) &BatchRenderable::setShader)
		.def("setLayer", &BatchRenderable::setLayer)
		.def("getLayer", &BatchRenderable::getLayer)
		.def("getTextureHandle", &BatchRenderable::getTextureHandle)
		.def("setTint", &BatchRenderable::setTint)
		.def("getShader", &BatchRenderable::getShader)
		.def("setFlag", &BatchRenderable::setFlag)
		.def_readwrite("blend", &BatchRenderable::blend)
		SET_INHERITED_VALUES(BatchRenderable, Renderable)
	END_DEFINE_LUA_CLASS()

	DEFINE_INHERITED_VALUES(TrailRenderable, Renderable)
	BEGIN_DEFINE_LUA_CLASS_DERIVED_SHARED(TrailRenderable, Renderable)
		.def(constructor<>())
		.def(constructor<int32>())
		.def("push", &TrailRenderable::push)
		.def("top", &TrailRenderable::top)
		.def("update", &TrailRenderable::update)
		.def("clear", &TrailRenderable::clear)
		.def("setWidth", &TrailRenderable::setWidth)
		.def("setSmooth", &TrailRenderable::setSmooth)
		.def("setControl", &TrailRenderable::setControl)
		.def("setDirectionWidth", &TrailRenderable::setDirectionWidth)
		.def("setColor", &TrailRenderable::setColor)
		.def("adjust", &TrailRenderable::adjust)
		.def("setDepthWidth", &TrailRenderable::setDepthWidth)
		.def("setDepthFade", &TrailRenderable::setDepthFade)
		.def("setPointMaxAge", &TrailRenderable::setPointMaxAge)
		.def("setRestrictDirection", &TrailRenderable::setRestrictDirection)
		.def_readwrite("options", &TrailRenderable::options)
		SET_INHERITED_VALUES(TrailRenderable, Renderable)
	END_DEFINE_LUA_CLASS()

	DEFINE_INHERITED_VALUES(SplineRenderable, Renderable)
	BEGIN_DEFINE_LUA_CLASS_DERIVED_SHARED(SplineRenderable, Renderable)
		.def(constructor<>())
		.def(constructor<int32>())
		.def("setControlPoint", &SplineRenderable::setControlPoint)
		.def("setWidth", &SplineRenderable::setWidth)
		.def("setColor", &SplineRenderable::setColor)
		.def("setTexture", (void(SplineRenderable::*)(const std::string&)) &SplineRenderable::setTexture)
		.def("setTexture", (void(SplineRenderable::*)(const TextureHandlePtr&)) &SplineRenderable::setTexture)
		SET_INHERITED_VALUES(SplineRenderable, Renderable)
	END_DEFINE_LUA_CLASS()

	DEFINE_INHERITED_VALUES(MeshRenderable, Renderable)
	BEGIN_DEFINE_LUA_CLASS_DERIVED_SHARED(MeshRenderable, Renderable)
		.def(constructor<>())
		.def("setMesh", &MeshRenderable::setMesh)
		.def("getMeshHandle", &MeshRenderable::getMeshHandle)
	SET_INHERITED_VALUES(MeshRenderable, Renderable)
	END_DEFINE_LUA_CLASS()

	BEGIN_DEFINE_LUA_ENUM(TextureStage)
		.enum_("constants")
		[
			value("Diffuse", TextureStageDiffuse),
			value("Specular", TextureStageSpecular),
			value("Normal", TextureStageNormal),
			value("Highlight", TextureStageSpecularHighlight),
			value("Ambient", TextureStageAmbient),
			value("Displacement", TextureStageDisplacement),
			value("Gobo", TextureStageGobo)
		]
	END_DEFINE_LUA_ENUM()

	BEGIN_DEFINE_LUA_ENUM(RenderableOptions)
		.enum_("constants")
		[
			value("IgnoreGlobalTint", RenderableOptionIgnoreGlobalTint)
		]
	END_DEFINE_LUA_ENUM()

	BEGIN_DEFINE_LUA_CLASS_SHARED(Texture)
		.scope
		[
			def("preload", &Texture::preload)
		]
	END_DEFINE_LUA_CLASS()

	BEGIN_DEFINE_LUA_CLASS_SHARED(TextureHandle)
		.def(constructor<const std::string&>())
		.def(constructor<RenderTargetType, bool>())
		.def(constructor<const TextureHandlePtr&>())
		.def(constructor<const TexturePtr&>())
		.def(constructor<const TexturePtr&, RectF&>())
		.def("setUVRect", &TextureHandle::setUVRect)
		.def("getUVRect", &TextureHandle::getUVRect)
		.def("setFlipHorizontal", &TextureHandle::setFlipHorizontal)
		.def("setFlipVertical", &TextureHandle::setFlipVertical)
		.def("getFlipHorizontal", &TextureHandle::getFlipHorizontal)
		.def("getFlipVertical", &TextureHandle::getFlipVertical)
		.def("setWrapHorizontal", &TextureHandle::setWrapHorizontal)
		.def("setWrapVertical", &TextureHandle::setWrapVertical)
		.def("fitUV", &TextureHandle::fitUV)
        .def("setUVRectSafe", &TextureHandle::setUVRectSafe)
		.def("squishWidth", &TextureHandle::squishWidth)
		.def("squishHeight", &TextureHandle::squishHeight)
		.def("shiftHorizontal", &TextureHandle::shiftHorizontal)
		.def("shiftVertical", &TextureHandle::shiftVertical)
	END_DEFINE_LUA_CLASS()

	BEGIN_DEFINE_LUA_CLASS_SHARED(ShaderHandle)
		.def(constructor<const std::string&>())
		.def("getName", &ShaderHandle::getName)
		.def("setUniformValue", (bool(ShaderHandle::*)(const std::string&, const double&)) &ShaderHandle::setUniformValue<double>)
		.def("setUniformValue", (bool(ShaderHandle::*)(const std::string&, const vec2&)) &ShaderHandle::setUniformValue<vec2>)
		.def("setUniformValue", (bool(ShaderHandle::*)(const std::string&, const vec3&)) &ShaderHandle::setUniformValue<vec3>)
		.def("setUniformValue", (bool(ShaderHandle::*)(const std::string&, const vec4&)) &ShaderHandle::setUniformValue<vec4>)
		.def("setUniformValue", (bool(ShaderHandle::*)(const std::string&, const mat4&)) &ShaderHandle::setUniformValue<mat4>)
	END_DEFINE_LUA_CLASS()

	BEGIN_DEFINE_LUA_ENUM(TextureChannels)
		.enum_("constants")
		[
			value("Alpha", TextureAlpha),
			value("RGB", TextureRGB),
			value("RGBA", TextureRGBA),
			value("Depth16", TextureDepth16),
			value("Depth24", TextureDepth24),
			value("Depth32", TextureDepth32)
		]
	END_DEFINE_LUA_ENUM()

	BEGIN_DEFINE_LUA_ENUM(DepthComponent)
		.enum_("constants")
		[
			value("Depth16", DepthComponent16),
			value("Depth24", DepthComponent24),
			value("Depth32", DepthComponent32)
		]
	END_DEFINE_LUA_ENUM()

	BEGIN_DEFINE_LUA_CLASS_SHARED(MeshHandle)
		.def(constructor<const std::string&>())
		.def("getMesh", &MeshHandle::getMesh)
		.def("getNumShapeInstances", &MeshHandle::getNumShapeInstances)
		.def("getShapeInstanceAtIndex", &MeshHandle::getShapeInstanceAtIndex)
	END_DEFINE_LUA_CLASS()

	BEGIN_DEFINE_LUA_CLASS_SHARED(Mesh)
		.def("getName", &Mesh::getName)
		.def("getShapeByName", &Mesh::getShapeByName)
	END_DEFINE_LUA_CLASS()

	BEGIN_DEFINE_LUA_ENUM(MeshMaterialType)
		.enum_("constants")
		[
			value("Color", MeshMaterialTypeSolidColor),
			value("Texture", MeshMaterialTypeSolidTexture),
			value("Phong", MeshMaterialTypePhong),
			value("PhongTexture", MeshMaterialTypePhongTexture)
		]
	END_DEFINE_LUA_ENUM()

	BEGIN_DEFINE_LUA_CLASS_SHARED(MeshShapeInstance)
		.def("addOverrideShader", &MeshShapeInstance::addOverrideShader)
		.def("addTexture", &MeshShapeInstance::addTexture)
	END_DEFINE_LUA_CLASS()

	BEGIN_DEFINE_LUA_CLASS_SHARED(MeshShape)
		.def("getCenter", &MeshShape::getCenter)
	END_DEFINE_LUA_CLASS()

	BEGIN_DEFINE_LUA_CLASS_SHARED(SharedUniform)
		.def("setUniformValue", (bool(ShaderHandle::*)(const std::string&, const double&)) &ShaderHandle::setUniformValue<double>)
		.def("setUniformValue", (bool(ShaderHandle::*)(const std::string&, const vec2&)) &ShaderHandle::setUniformValue<vec2>)
		.def("setUniformValue", (bool(ShaderHandle::*)(const std::string&, const vec3&)) &ShaderHandle::setUniformValue<vec3>)
		.def("setUniformValue", (bool(ShaderHandle::*)(const std::string&, const vec4&)) &ShaderHandle::setUniformValue<vec4>)
		.def("setUniformValue", (bool(ShaderHandle::*)(const std::string&, const mat4&)) &ShaderHandle::setUniformValue<mat4>)
		.scope
		[
			def("getInstance", &SharedUniform::getInstance)
		]
	END_DEFINE_LUA_CLASS()

	BEGIN_DEFINE_LUA_CLASS_SHARED(RenderTexture)
		.def("getTextureResource", &RenderTexture::getTextureResource)
		.def("getTexture", &RenderTexture::getTexture)
		.def("getDepthTexture", &RenderTexture::getDepthTexture)
		.def("getTextureHandle", &RenderTexture::getTextureHandle)
	END_DEFINE_LUA_CLASS()

	BEGIN_DEFINE_LUA_CLASS_SHARED(RenderTargetManager)
		.def("populateSharedTarget", (RenderTexturePtr(RenderTargetManager::*)(const std::string&, int32, int32, TextureChannels, DepthComponent)) &RenderTargetManager::populateSharedTarget)
		.def("getTextureFit", &RenderTargetManager::getTextureFit)
		.scope
		[
			def("getInstance", &RenderTargetManager::getInstance)
		]
	END_DEFINE_LUA_CLASS()

	BEGIN_DEFINE_LUA_CLASS_SHARED(PostProcess)
		.def("setOrthoOffset", &PostProcess::setOrthoOffset)
		.def("setOrthoScale", &PostProcess::setOrthoScale)
		.def("setSkew", &PostProcess::setSkew)
		.def("setShader", &PostProcess::setShader)
		.def("setTint", &PostProcess::setTint)
	END_DEFINE_LUA_CLASS()
		
}

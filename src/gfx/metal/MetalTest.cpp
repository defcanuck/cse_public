#include "PCH.h"

#include "gfx/metal/MetalTest.h"
#include "gfx/RenderInterface.h"
#include "gfx/Geometry.h"
#include "os/LogManager.h"
#include "scene/Camera.h"
#include "gfx/DrawCall.h"
#include "gfx/RenderTexture.h"
#include "scene/Sprite.h"

#include "ui/UITextElement.h"
#include "font/FontManager.h"

//#define RENDER_TEST_GEOMETRY 1
#define RENDER_TEST_FONTS 1

namespace cs
{
    
    const bool kUseTestAlphaTexture = false;
    const bool kUseTestFontTexture = false;
    const bool kUseRenderTexture = true;
    
    namespace MetalTest
    {
        
        const uint16 kFontSize = 64;
        
        struct Params
        {
            CameraPtr camera;
            GeometryPtr geom;
            GeometryPtr rtGeom;
            DrawCallPtr draw;
            TextureHandlePtr texture;
            RenderTexturePtr rtt;
            void* gpuBytes;
            
        };
        
        struct FontParams
        {
            FontParams()
                : initialized(false)
            { }
            FontPtr font;
            std::string text;
            TextOptions options;
            BatchDrawDataPtr batchDrawData;
            GeometryPtr geom;
            DrawCallPtr draw;
            bool initialized;
           
        };
    }
    
    
    static MetalTest::FontParams gFontParams;
    const std::string kFontName = "FreeSansBold.ttf";
    
    void initInternal();
    void renderInternal();
    
    void initFonts();
    void renderFonts();
    void populateFonts();
    
    void preRender();
    
    void MetalTest::init()
    {
        gFontParams.font = FontManager::getInstance()->getFont(kFontName);
        gFontParams.text = "asdf1234ASDF HELLO WORLD";
        gFontParams.options.size = MetalTest::kFontSize;
        
#if defined(RENDER_TEST_GEOMETRY)
        initInternal();
#elif defined(RENDER_TEST_FONTS)
        RenderInterface::getInstance()->setScreenClearColor(ColorF::Blue);
#endif
    }
    
    void MetalTest::render()
    {
        preRender();
        
#if defined(RENDER_TEST_GEOMETRY)
        renderInternal();
#elif defined(RENDER_TEST_FONTS)
        renderFonts();
#endif
    }

    
    void populateFonts()
    {
        FontManager::getInstance()->preloadWithString(kFontName, MetalTest::kFontSize, gFontParams.text.c_str());
        
        TextureAtlasManager::getInstance()->updateAtlases();
        FontManager::getInstance()->updateFonts();
        
    }
    
    void initFonts()
    {
        const bool kTestFontShader = false;
        
        populateFonts();

        TextLines lines;
        RectF srcRect;
        std::vector<RectF> lineBounds;
        
        gFontParams.font->generateVertices(
           gFontParams.text,
           gFontParams.options,
           lines,
           srcRect,
           lineBounds,
           nullptr,
           1.0f);

        gFontParams.batchDrawData = CREATE_CLASS(BatchDrawData);
        ColorB fontColor(255, 0, 255, 255);
        
        UITextElement::populateDrawData(lines, gFontParams.batchDrawData, fontColor);
        
        GeometryDataPtr data = CREATE_CLASS(GeometryData);
        
        uint32 w, h;
        RenderInterface::getInstance()->getScreenResolution(w, h);
        
        const vec3 kOffset(w / 2.0f, h / 2.0f, 0.0f);
        const vec3 kScale(1.0f, 1.0f, 1.0f);
        
        for (int32 i = 0; i < gFontParams.batchDrawData->positions.size(); ++i)
        {
            vec3& vert = gFontParams.batchDrawData->positions[i];
            vec2& uv0 = gFontParams.batchDrawData->uvs0[i];
            vec2& uv1 = gFontParams.batchDrawData->uvs1[i];
            ColorF col = toColorF(gFontParams.batchDrawData->vcolors[i]);
            
            vec2 flipUV0 = uv0;
            flipUV0.y = 1.0f - flipUV0.y;
            
            data->pushVertexData((vert * kScale) + kOffset);
            data->pushVertexData(flipUV0);
            data->pushVertexData(uv1);
            data->pushVertexData(col);
        }
        
        data->indexData = gFontParams.batchDrawData->indices;
        
        data->decl.addAttrib(AttributeType::AttribPosition, { AttributeType::AttribPosition, TypeFloat, 3, 0 });
        data->decl.addAttrib(AttributeType::AttribTexCoord0, { AttributeType::AttribTexCoord0, TypeFloat, 2, sizeof(vec3) });
        data->decl.addAttrib(AttributeType::AttribTexCoord1, { AttributeType::AttribTexCoord1, TypeFloat, 2, sizeof(vec3) + sizeof(vec2) });
        data->decl.addAttrib(AttributeType::AttribColor, { AttributeType::AttribColor, TypeFloat, 4, sizeof(vec3) + sizeof(vec2) + sizeof(vec2) });
        
        data->vertexSize = gFontParams.batchDrawData->positions.size();
        data->indexSize = gFontParams.batchDrawData->indices.size();
        data->storage = BufferStorageStatic;
        
        gFontParams.geom = CREATE_CLASS(Geometry, data);
        
        gFontParams.draw = CREATE_CLASS(DrawCall);
        gFontParams.draw->tag = "TestDraw";
        gFontParams.draw->type = DrawTriangles;
        gFontParams.draw->indexType = TypeUnsignedShort;
        gFontParams.draw->count = (uint32) data->indexSize;
        gFontParams.draw->offset = 0;
        
        gFontParams.draw->shaderHandle = CREATE_CLASS(ShaderHandle, RenderInterface::kDefaultFontShader);
        
        TextureHandlePtr fontTexture = gFontParams.font->getTexture(gFontParams.options.size);
        gFontParams.draw->textures[0] = fontTexture;
    
    }
    
    void renderFonts()
    {
        
        if (!gFontParams.initialized)
        {
            initFonts();
            gFontParams.initialized = true;
        }
       
        TextLines lines;
        RectF srcRect;
        
        uint32 w, h;
        RenderInterface::getInstance()->getScreenResolution(w, h);
        RectI viewport(0, 0, (int32) w, (int32) h);
        
        RenderInterface::getInstance()->setViewport(viewport);
        
        if (gFontParams.draw->shaderHandle.get())
        {
            ShaderBindParams bindParams;
            bindParams.geom = gFontParams.geom.get();
            bindParams.channels = RenderInterface::getInstance()->getCurrentRenderChannels();
            gFontParams.draw->shaderHandle->bind(bindParams);
        }
        
        RenderInterface::getInstance()->draw(gFontParams.geom.get(), gFontParams.draw);
    }
    
    static MetalTest::Params gTestParams;
    
    void initInternal()
    {
        
        FontManager::getInstance()->preloadWithString(kFontName, MetalTest::kFontSize, "asdf");
        
        GeometryDataPtr data = CREATE_CLASS(GeometryData);
        
        struct TestVertex
        {
            vec3 position;
            vec2 uv;
            vec4 color;
        };
        
        uint32 w = 0, h = 0;
        RenderInterface::getInstance()->getScreenResolution(w, h);
        float32 vw = 128;
        float32 vh = 128;
        
        TestVertex testData[] =
        {
            {
                vec3(-vw, -vh, 0.0f),
                vec2(0.0f, 0.0f),
                vec4(1.0f, 1.0f, 1.0f, 1.0f)
            },
            {
                vec3(-vw, vh, 0.0f),
                vec2(0.0f, 1.0f),
                vec4(1.0f, 1.0f, 1.0f, 1.0f)
            },
            {
                vec3(vw, vh, 0.0f),
                vec2(1.0f, 1.0f),
                vec4(1.0f, 1.0f, 1.0f, 1.0f)
            },
            {
                vec3(vw, -vh, 0.0f),
                vec2(1.0f, 0.0f),
                vec4(1.0f, 1.0f, 1.0f, 1.0f)
            },
        };
        
        for (int32 i = 0; i < ARRAY_LENGTH(testData); ++i)
        {
            TestVertex& vert = testData[i];
            data->pushVertexData(vert.position);
            data->pushVertexData(vert.uv);
            data->pushVertexData(vert.color);
        }
        
        data->indexData.push_back(0);
        data->indexData.push_back(1);
        data->indexData.push_back(2);
        
        data->indexData.push_back(0);
        data->indexData.push_back(2);
        data->indexData.push_back(3);
        
        data->decl.addAttrib(AttributeType::AttribPosition, { AttributeType::AttribPosition, TypeFloat, 3, 0 });
        data->decl.addAttrib(AttributeType::AttribTexCoord0, { AttributeType::AttribTexCoord0, TypeFloat, 2, sizeof(vec3) });
        data->decl.addAttrib(AttributeType::AttribColor, { AttributeType::AttribColor, TypeFloat, 4, sizeof(vec3) + sizeof(vec2) });
        
        data->vertexSize = 4;
        data->indexSize = data->indexData.size();
        data->storage = BufferStorageStatic;
        
        Dimensions testDimm(1024, 1024);
        gTestParams.rtt = CREATE_CLASS(RenderTexture, "MetalTest", testDimm, TextureRGB, DepthComponent32);
        
        // gTestParams.texture = CREATE_CLASS(TextureHandle, "test_texture.png");
        gTestParams.camera = CREATE_CLASS(Camera);
        gTestParams.geom = CREATE_CLASS(Geometry, data);
        
        RectF rect(-512, -512, 1024, 1024);
        // gTestParams.rtGeom = sprite::getSpriteGeometry(rect, gTestParams.rtt->getTextureHandle());
        gTestParams.rtGeom = sprite::getSpriteGeometry(rect, gTestParams.rtt->getTextureHandle());
        
        uchar* gpuBytes = new uchar[64 * 64];
        for (int32 i = 0; i < (64 * 64); ++i)
        {
            gpuBytes[i] = rand() % 255;
        }
        gTestParams.gpuBytes = (void*) gpuBytes;
        
        TextureHandlePtr tmpTexHandle;
        if (kUseTestAlphaTexture)
        {
            Dimensions dimm(64, 64);
            TextureResourcePtr tmpTex = RenderInterface::getInstance()->loadTexture(dimm, TextureAlpha, (uchar*) gpuBytes, TextureUsageShaderRead);
            TexturePtr tex = CREATE_CLASS(Texture, "tmpTex", tmpTex);
            tmpTexHandle = CREATE_CLASS(TextureHandle, tex);
        }

        gTestParams.draw = CREATE_CLASS(DrawCall);
        gTestParams.draw->tag = "TestDraw";
        gTestParams.draw->type = DrawTriangles;
        gTestParams.draw->indexType = TypeUnsignedShort;
        gTestParams.draw->count = (uint32) data->indexSize;
        gTestParams.draw->offset = 0;
        gTestParams.draw->shaderHandle = CREATE_CLASS(ShaderHandle, RenderInterface::kMetalTest);
        
        if (kUseTestAlphaTexture)
        {
            gTestParams.draw->textures[0] = tmpTexHandle;
            gTestParams.draw->textures[1] = tmpTexHandle;
        }
        else
        {
            gTestParams.draw->textures[0] = RenderInterface::kDefaultTexture;
            gTestParams.draw->textures[1] = RenderInterface::kErrorTexture;
        }
        
        //dc->textures[1] = this->edgeTextureHandle;
        gTestParams.draw->color = ColorB::White;
        
        const ColorF kClearColor(0.5f, 0.7f, 0.5f, 1.0f);
        RenderInterface::getInstance()->setClearColor(kClearColor);
    }
    
    
    void pushMVP(float32 w, float32 h, float32 x_offset, float32 y_offset, float32 x_translate, float32 y_translate)
    {
        UniformPtr mvpPtr = SharedUniform::getInstance().getUniform("mvp");
        if (mvpPtr.get())
        {
            mat4 cam = glm::translate(vec3(x_offset, y_offset, 0.0f));
            mat4 model = glm::translate(vec3(x_translate, y_translate, 0.0f));
            mat4 mvp = glm::ortho(0.0f, w, 0.0f, h, -1.0f, 1.0f) * cam * model;
            mvpPtr->setValue(mvp);
        }
    }
    
    void preRender()
    {

    }
    
    void renderInternal()
    {
        
        uint32 w, h;
        float gw, gh;
        RenderInterface::getInstance()->getScreenResolution(w, h);
        
        gw = float32(w);
        gh = float32(h);
        
        if (kUseRenderTexture)
        {
            gw = 1024;
            gh = 1024;
            gTestParams.rtt->bind();
            RectI viewport(0, 0, gw, gh);
            
            RenderInterface::getInstance()->setViewport(viewport);
            RenderInterface::getInstance()->setClearColor(ColorF::Green);
            std::vector<ClearMode> kClearAll = { ClearColor, ClearDepth };
            RenderInterface::getInstance()->clear(kClearAll);
            
            RenderInterface::getInstance()->startRenderPass();
            
        }
        else
        {
             RectI viewport(0, 0, gw, gh);
             RenderInterface::getInstance()->setViewport(viewport);
        }
        
        if (kUseTestFontTexture)
        {
            TextureAtlasManager::getInstance()->updateAtlases();
            FontManager::getInstance()->updateFonts();
        
            gTestParams.draw->textures[0] = gFontParams.font->getCharacterHandle('a', gFontParams.options.size);
            gTestParams.draw->textures[1] = gFontParams.font->getCharacterHandle('a', gFontParams.options.size);
        }
   
        gTestParams.draw->shaderHandle->setUniformValue("color1", toVec4(ColorF::Blue));
        gTestParams.draw->shaderHandle->setUniformValue("color2", toVec4(ColorF::Blue));
        
        TextureChannels channels = RenderInterface::getInstance()->getCurrentRenderChannels();
        TextureChannels depth = RenderInterface::getInstance()->getCurrentDepthChannels();
        
        pushMVP(gw, gh, 0, 0, 256.0, 256.0);
        if (gTestParams.draw->shaderHandle.get())
        {
            gTestParams.draw->instanceIndex = 0;
            ShaderBindParams bindParams;
            bindParams.geom = gTestParams.geom.get();
            bindParams.channels = channels;
            bindParams.depth = depth;
            bindParams.index = gTestParams.draw->instanceIndex;
            
            gTestParams.draw->shaderHandle->bind(bindParams);
        }
        RenderInterface::getInstance()->draw(gTestParams.geom.get(), gTestParams.draw);
        
        gTestParams.draw->shaderHandle->setUniformValue("color1", toVec4(ColorF::Red));
        gTestParams.draw->shaderHandle->setUniformValue("color2", toVec4(ColorF::Red));
        
        pushMVP(gw, gh, 0, 0, 0.0, 0.0f);
        if (gTestParams.draw->shaderHandle.get())
        {
            gTestParams.draw->instanceIndex = 1;
            ShaderBindParams bindParams;
            bindParams.geom = gTestParams.geom.get();
            bindParams.channels = channels;
            bindParams.depth = depth;
            bindParams.index = gTestParams.draw->instanceIndex;
            
            gTestParams.draw->shaderHandle->bind(bindParams);
        }
        RenderInterface::getInstance()->draw(gTestParams.geom.get(), gTestParams.draw);
        
        if (kUseRenderTexture)
        {
            gTestParams.rtt->unbind();
            pushMVP(float32(w), float32(h), w * 0.5f, h * 0.5f, 0.0f, 0.0);
            gTestParams.rtGeom->draw();
        }
    }
}

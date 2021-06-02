#include "PCH.h"


#include "animation/spine/SpineImpl.h"
#include "os/LogManager.h"
#include "os/FileManager.h"

#include "gfx/RenderInterface.h"
#include "gfx/Texture.h"

extern "C" void _spAtlasPage_createTexture(spAtlasPage* page, const char* path)
{
	std::string str_path(path);
	cs::FileManager::flipSeparator(str_path);

	std::string fileName, filePath;
	cs::FileManager::splitPath(str_path, fileName, filePath);

	cs::log::info("Spine: Creating texture ", fileName, " from path ", filePath);

	std::string pngFilePath = filePath + cs::FileManager::getInstance()->separator() + fileName;
    cs::TextureResourcePtr resource = cs::RenderInterface::getInstance()->loadTexture(pngFilePath);
	if (resource.get())
	{
		cs::SpineTexture* spine_tex = new cs::SpineTexture;
		spine_tex->texture = CREATE_CLASS(cs::Texture, fileName, resource);
		page->rendererObject = (void*) spine_tex;
	}
}

extern "C" void _spAtlasPage_disposeTexture(spAtlasPage* page)
{
	cs::log::info("Spine: Destroying texture");
	if (page->rendererObject)
	{
		cs::SpineTexture* texture = reinterpret_cast<cs::SpineTexture*>(page->rendererObject);
		delete texture;
		page->rendererObject = nullptr;
	}
}

extern "C" char* _spUtil_readFile(const char* path, int* length)
{
	std::string str_path(path);
	cs::FileManager::flipSeparator(str_path);

	cs::AutoRelease<cs::FileStream> stream;
	if (!cs::FileManager::getInstance()->openStreamPath(str_path, (cs::DataStream**) &stream.value))
	{
		cs::log::error("Spine: Cannot find path:", str_path);
		return "";
	}

	std::istream& ifs = stream->getStream();

	ifs.seekg(0, std::ios::end);
	*length = (int32) ifs.tellg();
	ifs.seekg(0, std::ios::beg);

	std::string strBuffer;
	strBuffer.assign((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());

	char* buffer = (char*)malloc(*length * sizeof(char));
	memcpy(buffer, strBuffer.c_str(), *length);

	return buffer;
}

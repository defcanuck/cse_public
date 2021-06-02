#include "PCH.h"

#include "gfx/Types.h"

namespace cs
{
	const char* kTextureStageName[] =
	{
		"diffuse",					// TextureStageDiffuse
		"specular",					// TextureStageSpecular
		"normal",					// TextureStageNormal
		"specular_highlight",		// TextureStageSpecularHighlight
		"ambient",					// TextureStageAmbient
		"displacement",				// TextureStageDisplacement

		//...
		"post_depth",				// TexturePostDepth
		"post_color"				// TexturePostColor
	};

	TextureStage getStageForName(const std::string& name)
	{
		for (size_t i = 0; i < TextureStageMAX; i++)
		{
			if (kTextureStageName[i] == name)
				return (TextureStage)i;
		}
		return TextureStageNone;
	}

	TextureStageNum getPhysicalStage(TextureStage stage)
	{
		switch (stage)
		{
			case TextureStageDiffuse:
			case TexturePostColor:
				return TextureStageNum0;

			case TextureStageSpecular:
			case TexturePostDepth:
				return TextureStageNum1;
			
			case TextureStageNormal:
				return TextureStageNum2;
			
			case TextureStageSpecularHighlight:
				return TextureStageNum3;
			
			case TextureStageAmbient:
				return TextureStageNum4;

			case TextureStageDisplacement:
				return TextureStageNum5;

			case TextureStageGobo:
				return TextureStageNum6;

			default:
				return TextureStageNumNone;
		}
	}

	uint16 kStaticQuadIndices[] = { 0, 2, 1, 0, 3, 2 };


}
#include <Arclight/Graphics/Image.h>

#include <Arclight/Resource.h>
#include <Arclight/ResourceManager.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

namespace Arclight {

Image::Image(){

}

int Image::LoadResource(const std::string& name){
	Resource* res;
	if(int e = ResourceManager::LoadResource(name, res); e){
		return e;
	}

	const uint8_t* imageData = res->m_data.data();
	int channels; // We want four channels (RGBA)
	uint8_t* pixelData = stbi_load_from_memory(imageData, res->m_data.size(), &m_size.x, &m_size.y, &channels, 4);

	if(channels != 4){
		throw std::runtime_error("Image::LoadResource: Failed to load image from resource, expected 4 channels.");
		return 1;
	}

	m_pixelData = std::unique_ptr<uint8_t>(pixelData);

	return 0;
}

} // namespace Arclightload_from_memory
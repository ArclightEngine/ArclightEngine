#include <Arclight/Graphics/Image.h>

#include <Arclight/Core/Resource.h>
#include <Arclight/Core/ResourceManager.h>
#include <Arclight/Core/File.h>

#include <stdexcept>

#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

namespace Arclight {

static int stbi_read(void* user, char* data, int size) {
	auto f = reinterpret_cast<File*>(user);
	return f->Read(data, size);
}

static void stbi_skip(void* user, int n) {
	auto f = reinterpret_cast<File*>(user);
	f->Seek(f->Tell() + n);
}

static int stbi_eof(void* user) {
	auto f = reinterpret_cast<File*>(user);
	return f->IsEOF();
}

static stbi_io_callbacks stbi_callbacks = {
	stbi_read,
	stbi_skip,
	stbi_eof
};

Image::Image(std::string name)
	: Resource(std::move(name)) {

}

// Load if not loaded
int Image::Load() {
	return LoadImpl();
}

int Image::LoadImpl(){
	int channels = 0; // We want four channels (RGBA)

	File* file = File::Open(m_filesystemPath, File::OpenReadOnly);
	uint8_t* pixelData = stbi_load_from_callbacks(&stbi_callbacks, file, &m_size.x, &m_size.y, &channels, 4);
	delete file;

	if(channels != 4){
		throw std::runtime_error("Image::LoadResource: Failed to load image from resource, expected 4 channels.");
		return 1;
	}

	m_pixelData = std::unique_ptr<uint8_t>(pixelData);

	return 0;
}

} // namespace Arclight

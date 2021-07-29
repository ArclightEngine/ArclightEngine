#pragma once

#include <string>
#include <vector>

namespace Arclight {

struct Resource {
	std::string m_id;
	std::vector<uint8_t> m_data;
};

} // namespace Arclight
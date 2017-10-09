#include "utils.h"

#include <fstream>

std::vector<char> utils::readFile(const std::string & filen)
{
	std::ifstream file (filen, std::ios::ate | std::ios::binary);

	if (!file.is_open()) {
		throw std::runtime_error("Failed to open file '" + filen + "'!");
	}

	size_t fileSize = (size_t)file.tellg();
	std::vector<char> buffer(fileSize);

	file.seekg(0);
	file.read(buffer.data(), fileSize);

	file.close();

	return buffer;
}

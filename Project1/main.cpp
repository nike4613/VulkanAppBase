#include <iostream>

#include "libs.h"
#include "VkApplication.h"

int main() {
	VkApplication app(1280, 720, ENGINE_FULL_NAME_STR + " Test", Version(1,0,0));

	int exit = EXIT_SUCCESS;

	try {
		app.run();
	}
	catch (const ERROR_TYPE& e) {
		std::cerr << e.what() << std::endl;
		exit = EXIT_FAILURE;
	}

	return exit;
}
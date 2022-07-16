#include "texture_mapping.h"

int main(int argc, char* argv[]) {
	try {
		TextureMapping app;
		app.ParseArguments(argc, argv);
		app.run();
	}
	catch (std::exception& e) {
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}
	catch (...) {
		std::cerr << "Unknown Error" << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
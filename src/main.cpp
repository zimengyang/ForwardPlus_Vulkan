
#include "VulkanBaseApplication.h"

extern const int WIDTH = 1024;
extern const int HEIGHT = 512;

VulkanBaseApplication app;

int main() {

	try {
		app.run();
	}
	catch (const std::runtime_error& e) {
		std::cerr << e.what() << std::endl;
		
		system("pause");
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
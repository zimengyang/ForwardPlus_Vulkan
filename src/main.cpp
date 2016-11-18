
#include "VulkanBaseApplication.h"

extern const int WIDTH = 800;
extern const int HEIGHT = 600;


int main() {
	VulkanBaseApplication app;

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
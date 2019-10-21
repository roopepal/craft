#include "pch.h"
#include "Display.hpp"

int main(int argc, char* argv[])
{
	Display d = Display("Craft", 1920, 1080);
	if (d.setup(argc, argv))
	{
		d.start();
	}

	return EXIT_SUCCESS;
}

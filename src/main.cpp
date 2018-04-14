#include "imgui.h"
#include "imgui-SFML.h"

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/System/Clock.hpp>
#include <SFML/Window/Event.hpp>

#include "windows/map_display_window.h"
#include "windows/parameter_window.h"
#include "parameters/parameter_loader.h"

#include <iostream>


int main(int argc, char *argv[])
{
	if (argc < 2)
	{
		std::cerr << "Invalid number of arguments" << std::endl;
		return 1;
	}

	const std::string param_file(argv[1]);
	ParameterLoader loader(param_file);

	sf::RenderWindow window(sf::VideoMode(1080, 720), "Map Generator");
	window.setFramerateLimit(60);
	ImGui::SFML::Init(window);

	ParameterWindow parameter_window(loader.getParams());
	MapDisplayWindow map_display_window(512, 512);

	sf::Clock deltaClock;

	while (window.isOpen()) 
	{
		// poll events
		sf::Event event;
		while (window.pollEvent(event))
		{
			// pass events to imgui
			ImGui::SFML::ProcessEvent(event);

			if (event.type == sf::Event::Closed)
			{
				window.close();
			}
		}

		// update imgui
		ImGui::SFML::Update(window, deltaClock.restart());
		
		// draw gui
		bool updated = parameter_window.update();
		map_display_window.update();

		if (updated)
		{
			std::cout << "Parameters Updated\n";
		}

		// re-draw the screen
		window.clear(sf::Color(255, 255, 255, 255));
		ImGui::SFML::Render(window);
		window.display();
	}

	ImGui::SFML::Shutdown();

	return 0;
}

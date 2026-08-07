#include "P2Game.hpp"

int main()
{

	P2::Game game;

	game.initialize();
	game.loop();
	game.deinitialize();
	
	return 0;
}
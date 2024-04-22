#include "ECS.hpp"



int main()
{
	ecs::World world;
	ecs::Commands commands(world);

	commands.CreateEntity(10, "Test", 5.5);
	commands.CreateEntity("555", -10);
	return 0;

}
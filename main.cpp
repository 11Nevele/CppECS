#include "ECS.hpp"
#include <string>
#include<iostream>
#include <vector>
#include "SparseSet.hpp"

class MyClass
{
public:
	std::string name;
	MyClass(const std::string& name = "default") : name(name) {}
};
class Timer
{
public :
	int time;
};

void Test(ecs::Queryer& queryer)
{
	for (auto i : queryer.GetEntities<MyClass>())
	{
		std::cout << i << " ";
	}

	std::cout << "\n--------------------------\n";

	for (auto i : queryer.GetEntities<int>())
	{
		std::cout << i << " ";
	}

	std::cout << "\n--------------------------\n";

	for (auto i : queryer.GetEntities<std::string>())
	{
		std::cout << i << " ";
	}

	std::cout << "\n--------------------------\n";

	for (auto i : queryer.GetEntities<std::string>())
	{
		std::cout << i << " ";
	}

	std::cout << "\n--------------------------\n";

	for (auto i : queryer.GetEntities<std::string, int>())
	{
		std::cout << i << " ";
	}

	std::cout << "\n--------------------------\n";

	for (auto i : queryer.GetEntities<std::string, int, MyClass>())
	{
		std::cout << i << " ";
	}

	std::cout << "\n--------------------------\n";

}
int main()
{

	ecs::World world;
	ecs::Commands commands(world);

	commands.CreateEntity(MyClass())
		.CreateEntity(MyClass("test"))
		.CreateEntity(13)
		.CreateEntity(16)
		.CreateEntity(MyClass("Nick"))
		.CreateEntity(11)
		.CreateResource(Timer{ 321 })
		.CreateEntity(std::string("123"))
		.CreateEntity(std::string("12"), 10)
		.CreateEntity(11, std::string("12"))
		.CreateEntity(12, std::string("312"), MyClass());
		;

	ecs::Resource resource(world);
	Timer t = resource.GetResource<Timer>();
	commands.DestroyResource<Timer>();

	ecs::Queryer queryer(world);
	Test(queryer);

	commands.RemoveEntity(1).
		RemoveEntity(9).
		RemoveEntity(7);
	for (auto& i : queryer.GetEntities<int>())
	{
		commands.RemoveEntity(i);
	}

	Test(queryer);

	
	return 0;
	
}
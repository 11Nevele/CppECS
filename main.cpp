#include "ECS.hpp"
#include <string>
#include<iostream>
#include <vector>
#include "SparseSet.hpp"
#include <memory>
class Timer
{
public :
	int time;
	Timer() : time(0) {}
	Timer(int time) : time(time) {}
	~Timer() {}
};

class MyClass
{
public:
	std::string name;
	MyClass() : name("default") {}
	MyClass(std::string name) : name(name) {}
};
using curCSetting = ecs::Settings<int, std::string, MyClass>;
using curRSetting = ecs::Settings<Timer>;
using curWorld = ecs::World<curCSetting, curRSetting>;

void Test(ecs::Queryer<curCSetting, curRSetting>& queryer)
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

	for (auto& i : queryer.GetEntities<std::string, int>())
	{
		std::cout << i << " ";
	}

	std::cout << "\n--------------------------\n";

	for (auto& i : queryer.GetEntities<std::string, int, MyClass>())
	{
		std::cout << i << " ";
	}

	std::cout << "\n--------------------------\n";

}
int main()
{
	

	curWorld world;
	ecs::Commands commands(world);
	ecs::Queryer queryer(world);

	commands.CreateResource<Timer>(Timer(321));
	commands.CreateEntity(MyClass())
		.CreateEntity(MyClass("test"))
		.CreateEntity(13)
		.CreateEntity(16)
		.CreateEntity(MyClass("Nick"))
		.CreateEntity(11)
		.CreateEntity(std::string("123"))
		.CreateEntity(std::string("12"), 10)
		.CreateEntity(11, std::string("12"))
		.CreateEntity(12, std::string("312"), MyClass());
		;

	Test(queryer);

	commands.RemoveEntity(0)
		.RemoveEntity(7)	
		.RemoveEntity(9);

	Test(queryer);

	Timer& tResource = commands.GetResource<Timer>();
	
	

	
	return 0;
	
}
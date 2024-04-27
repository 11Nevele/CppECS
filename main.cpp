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

using Commands = ecs::Commands<curCSetting, curRSetting>;
using Queryer = ecs::Queryer<curCSetting, curRSetting>;
using StartupSystem = curWorld::StartupSystem;
using UpdateSystem = curWorld::UpdateSystem;

void InitObjects(Commands& commands)
{
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
	
}

void PrintObjects(Commands& commands, Queryer& queryer)
{
	for (auto& i : queryer.GetEntities<int>())
	{
		std::cout << i << ": " << commands.Get<int>(i) << std::endl;
	}
	std::cout << "\n-----------------------------\n";
	for (auto& i : queryer.GetEntities<std::string>())
	{
		std::cout << i << ": " << commands.Get<std::string>(i) << std::endl;
	}
	std::cout << "\n-----------------------------\n";
	for (auto& i : queryer.GetEntities<MyClass>())
	{
		std::cout << i << ": " << commands.Get<MyClass>(i).name << std::endl;
	}
	std::cout << "\n-----------------------------\n";
}



int main()
{
	curWorld world;
	world.AddStartupSystem(InitObjects);
	world.AddUpdateSystem(PrintObjects);
	world.StartupUpdate();
	world.Update();

	
	return 0;
	
}
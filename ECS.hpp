#ifndef ECS
#define ECS
#include <vector>
#include <algorithm>
#include <cassert>
#include <unordered_map>
#include <any>
#include "SparseSet.hpp"
namespace ecs
{
	using Entity = uint32_t;
	using ComponentID = uint32_t; //each type of component has a unique ID

	class ComponentIndexGetter final //this class is used to get the index of a component
	{
	private:
		inline static ComponentID index;
	public:
		template <class TYPE>
		inline static ComponentID GetIndex()
		{
			static ComponentID id = index++;
			return id;
		}
	};

	//this class is used to get the index of a entity

	class EntityIndexGetter final 
	{
	private:
		inline static Entity index;
	public:
		static Entity GetIndex()
		{
			Entity id = index++;
			return id;
		}	
	};

	class Commands;

	class World final
	{
	public:

	private:
		friend Commands;

		// stores instances of a type of component
		template <class TYPE>
		class Pool final 
		{
		

		public:
			using Create = TYPE * (*)(void);
			using Destroy = void(*)(TYPE*);
			Pool(Create newCreate, Destroy newDestory): create(newCreate), destroy(newDestory) {}

			TYPE* CreateObject()
			{
				TYPE* object = nullptr;
				if (cache.size() > 0)
				{
					object = cache.back();
					cache.pop_back();
					objects.push_back(object);
				}
				else
				{
					object = create();
					objects.push_back(object);
				}
				return object;
			}
			TYPE* DestoryObject(TYPE* target)
			{
				auto iter = std::find(objects.begin(), objects.end(), target);
				if (iter != objects.end())
				{
					std::swap(iter, objects.back());
					cache.push_back(objects.back());
					objects.pop_back();
				}
				else
				{
					assert(false);
					
				}
			}
		private:


			std::vector<TYPE*> objects;
			std::vector<TYPE*> cache;

			Create create;
			Destroy destroy;
		};

		template <class TYPE>
		struct ComponentInfo
		{
			Pool<TYPE> pool;//instance of the component
			SparseSet<Entity> entities;//entities that have the component
		//public:
			ComponentInfo() : pool(nullptr, nullptr) {}
			ComponentInfo(Pool<TYPE>::Create create, Pool<TYPE>::Destroy destroy) : pool(create, destroy) {}
			void AddEntity(Entity entity)
			{
				entities.Add(entity);
			}
			void RemoveEntity(Entity entity)
			{
				entities.Remove(entity);
			}
			
		};


		//used to link a type of component to its info
		//any should be a ComponentInfo
		using ComponentMap = std::unordered_map<ComponentID, std::any>; 
		//stores the componentsID and a instance of the component
		using ComponentStorage = std::unordered_map<ComponentID, std::any>;

		ComponentMap componentMap;
		
		//link the entity to componentsStorage
		std::unordered_map<Entity, ComponentStorage> entitysMap;
	};

	//Do operations in the world
	class Commands final
	{
	private:
		World& curWorld;

		template<class TYPE, class... ARGS>
		void AddComponent(Entity entity, const TYPE& component, const ARGS&... remain)
		{
			ComponentID index = ComponentIndexGetter::GetIndex<TYPE>();
			auto it = curWorld.componentMap.find(index);
			if (it == curWorld.componentMap.end())
			{
				curWorld.componentMap.insert({index, World::ComponentInfo<TYPE>(
					[]()->TYPE* {return new TYPE(); },
					[](TYPE* target) {delete target; }
				)});
				
			}
			World::ComponentInfo<TYPE>& curComponentInfo = std::any_cast<World::ComponentInfo<TYPE>&>(curWorld.componentMap[index]);
			TYPE* element = (TYPE*)curComponentInfo.pool.CreateObject();
			*element = component;
			curComponentInfo.AddEntity(entity);
			curWorld.entitysMap[entity].insert({index, element});
			if constexpr(sizeof...(remain) != 0)
			{
				AddComponent(entity, remain...);
			}
		}
		
	public:
		Commands(World& newWorld): curWorld(newWorld){}

		template <class... ARGS>
		void CreateEntity(const ARGS... args)
		{
			Entity entity = EntityIndexGetter::GetIndex();
			AddComponent(entity, args...);
		}


	};
}


#endif // !ECS

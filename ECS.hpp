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
	class COMPONENT{};
	class RESOURCE{};
	using Entity = uint32_t;
	using ResourceID = uint32_t;
	using ComponentID = uint32_t; //each type of component has a unique ID

	template <class TYPE>
	class IndexGetter final //this class is used to get the index of a component
	{
	private:
		inline static uint32_t index;
	public:
		template <class T>
		inline static uint32_t GetIndex()
		{
			static uint32_t id = index++;
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
	class Resource;
	class Queryer;

	class World final
	{
	public:

	private:
		friend Commands;
		friend Resource;
		friend Queryer;
		// stores instances of a type of component
		
		class Pool final 
		{
		public:
			using Create = std::any(*)(void);
			using Destroy = void(*)(std::any);
			Pool(Create newCreate, Destroy newDestory): create(newCreate), destroy(newDestory) {}

			std::any CreateObject()
			{
				std::any object = nullptr;
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
			void DestoryObject(std::any target)
			{
				auto com = [&target](const std::any& a)
				{ return std::any_cast<void*> (target) == std::any_cast<void*>(a); };
				auto iter = std::find_if(objects.begin(), objects.end(), com);
				if (iter != objects.end())
				{
					std::iter_swap(iter, objects.end() - 1);
					cache.push_back(objects.back());
					objects.pop_back();
				}
				else
				{
					assert(false);
					
				}
			}
		private:


			std::vector<std::any> objects;
			std::vector<std::any> cache;

			Create create;
			Destroy destroy;
		};

		class ComponentInfo
		{
		public:
			Pool pool;//instance of the component
			SparseSet<Entity> entities;//entities that have the component
			ComponentInfo() : pool(nullptr, nullptr) {}
			ComponentInfo(Pool::Create create, Pool::Destroy destroy) : pool(create, destroy) {}
		};
		class ResourceInfo final
		{
		public:
			std::any resource;

			using Create = std::any(*)(void);
			using Destroy = void(*)(std::any);

			Create create;
			Destroy destroy;

			ResourceInfo(Create newCreate, Destroy newDestroy) : create(newCreate), destroy(newDestroy)
			{
				assert(newCreate);
				assert(newCreate);
			}
			ResourceInfo() : create(nullptr), destroy(nullptr) {}
		};

		using ComponentMap = std::unordered_map<ComponentID, ComponentInfo>; 
		using ResourceMap = std::unordered_map<ResourceID, ResourceInfo>;

		using ComponentStorage = std::unordered_map<ComponentID, std::any>;

		//used to link a type of component to its info
		ComponentMap componentMap;
		ResourceMap resourceMap;
		
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
			ComponentID index = IndexGetter<COMPONENT>::GetIndex<TYPE>();
			auto it = curWorld.componentMap.find(index);
			if (it == curWorld.componentMap.end())
			{
				curWorld.componentMap.emplace(index, World::ComponentInfo(
					[]() -> std::any {return (new TYPE); },
					[](std::any target) {delete std::any_cast<TYPE*>(target); }
					)
				);
			}
			World::ComponentInfo& curComponentInfo = curWorld.componentMap[index];
			TYPE* element = std::any_cast<TYPE*>(curComponentInfo.pool.CreateObject());
			*element = component;
			curComponentInfo.entities.Add(entity);
			curWorld.entitysMap[entity][index] = element;
			if constexpr(sizeof...(remain) != 0)
			{
				AddComponent(entity, remain...);
			}
		}
		
	public:
		Commands(World& newWorld): curWorld(newWorld){}

		template <class... ARGS>
		Commands& CreateEntity(const ARGS&... args)
		{
			Entity entity = EntityIndexGetter::GetIndex();
			AddComponent(entity, args...);
			return *this;
		}

		Commands& RemoveEntity(Entity entity)
		{
			auto it = curWorld.entitysMap.find(entity);
			assert(it != curWorld.entitysMap.end());
			for (auto& t : it->second)
			{
				const ComponentID& componentID = t.first;
				auto& component = t.second;
				World::ComponentInfo& info = (curWorld.componentMap[componentID]);
				info.entities.Remove(entity);
				info.pool.DestoryObject(component);
			}
			curWorld.entitysMap.erase(it);
			return *this;
		}

		template <class TYPE>
		Commands& CreateResource(const TYPE& object)
		{
			ResourceID index = IndexGetter<RESOURCE>::GetIndex<TYPE>();
			auto it = curWorld.resourceMap.find(index);
			assert(it == curWorld.resourceMap.end());
			curWorld.resourceMap.emplace(index,
				World::ResourceInfo(
					[]() -> std::any {return (new TYPE); },
					[](std::any target) {delete std::any_cast<TYPE*>(target); }
				)
			);
			auto& info = curWorld.resourceMap.find(index)->second;
			info.resource = info.create();
			*std::any_cast<TYPE*>(info.resource) = object;
			return *this;
		}

		template <class TYPE>
		Commands& DestroyResource()
		{
			ResourceID index = IndexGetter<RESOURCE>::GetIndex<TYPE>();
			auto it = curWorld.resourceMap.find(index);
			assert(it != curWorld.resourceMap.end());

			auto& info = it->second;
			info.destroy(info.resource);
			curWorld.resourceMap.erase(it);
			return *this;
		}


	};

	class Resource final
	{
	private:
		World & curWorld;
	public:
		Resource(World& newWorld) : curWorld(newWorld) {}
		template <class TYPE>
		TYPE& GetResource()
		{
			ResourceID index = IndexGetter<RESOURCE>::GetIndex<TYPE>();
			auto it = curWorld.resourceMap.find(index);
			assert(it != curWorld.resourceMap.end());
			return *std::any_cast<TYPE*>(it->second.resource);
		}
		template <class TYPE>
		bool HasResource()
		{
			ResourceID index = IndexGetter<RESOURCE>::GetIndex<TYPE>();
			auto it = curWorld.resourceMap.find(index);
			return it != curWorld.resourceMap.end();
		}
	};

	class Queryer final
	{
	private:
		World & curWorld;
		
		//Check if the entity has the all component
		template <class TYPE, class... ARGS>
		bool CheckEntity(Entity entity)
		{
			ComponentID index = IndexGetter<COMPONENT>::GetIndex<TYPE>();
			auto it = curWorld.componentMap.find(index);
			if (it == curWorld.componentMap.end() || !it->second.entities.Contains(entity))
			{
				return false;
			}
			if constexpr (sizeof...(ARGS) > 0)
			{
				return CheckEntity<ARGS...>(entity);
			}
			else
			{
				return true;
			}
			
		}
	public:

		Queryer(World& newWorld) : curWorld(newWorld) {}
		template <class TYPE, class... ARGS>
		std::vector<Entity> GetEntities()
		{
			std::vector<Entity> result;
			ComponentID index = IndexGetter<COMPONENT>::GetIndex<TYPE>();
			auto it = curWorld.componentMap.find(index);
			if (it == curWorld.componentMap.end())
			{
				return result;
			}
			auto& enetities = it->second.entities.GetDense();

			for (auto& entity : enetities)
			{
				if constexpr (sizeof...(ARGS) > 0)
				{
					if (CheckEntity<ARGS...>(entity))
					{
						result.push_back(entity);
					}
				}
				else
				{
					result.push_back(entity);
				}
			}
			return result;
		}
	};
}


#endif // !ECS

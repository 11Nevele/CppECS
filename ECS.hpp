#ifndef ECS
#define ECS
#include <vector>
#include <algorithm>
#include <cassert>
#include <unordered_map>
#include "SparseSet.hpp"
#include <tuple>
#define MAX_COMPONENTS UINT32_MAX

namespace ecs
{


	class COMPONENT {};
	class RESOURCE {};
	using Entity = uint32_t;
	using ResourceID = uint32_t;
	using ComponentID = uint32_t; //each type of component has a unique ID
	using ComponentIndex = uint32_t; //a index within each type of component


	template <typename Type>
	struct Pool final
	{
	public:
		std::vector<Type> objects;
		std::vector<ComponentIndex> cache;

		ComponentIndex CreateObject()
		{
			ComponentIndex index;
			if (cache.empty())
			{
				index = objects.size();
				objects.emplace_back();
			}
			else
			{
				index = cache.back();
				cache.pop_back();
			}
			return index;
		}
		void DestroyObject(ComponentID index)
		{
			cache.push_back(index);
		}
	};


	template <typename Type, typename... Other>
	struct ComponentInfo final
	{
		Pool<Type> pool;
		SparseSet<Entity> entities;
	};

	template<typename... TYPE_LIST>
	struct Settings
	{

	public:
		//using test = Pool<TYPE_LIST...>;myStruct

		using PoolsList = std::tuple<ComponentInfo<TYPE_LIST>...>;
		using TupleList = std::tuple<TYPE_LIST...>;

		template <class Target>
		static constexpr bool HasType()
		{
			return DoHasType<Target, TYPE_LIST...>();
		}

		static constexpr uint32_t Size()
		{
			return sizeof...(TYPE_LIST);
		}

		template <class Target>
		static constexpr uint32_t GetID()
		{
			static_assert(HasType<Target>(), "Type not found in the list");
			return DoIndex<Target, TYPE_LIST...>();
		}

	private:
		template <class Target, class Cur, class... Remains>
		static constexpr bool DoHasType()
		{
			if constexpr (std::is_same<Target, Cur>{})
				return true;
			if constexpr (sizeof...(Remains) == 0)
				return false;
			else
				return DoHasType<Target, Remains...>();
		}

		template <class Target, class Cur, class... Remains>
		static constexpr uint32_t DoIndex()
		{
			if constexpr (std::is_same<Target, Cur>{})
				return 0;
			else
				return 1 + DoIndex<Target, Remains...>();
		}
	};


	template <class ComponentSetting, class ResourceSetting>
	class Commands;
	template <class ComponentSetting, class ResourceSetting>
	class Queryer;

	template <class ComponentSetting, class ResourceSetting>
	class World final
	{
	public:

	private:
		friend Commands;
		friend Queryer;
		friend Settings;



		using ComponentPools = ComponentSetting::PoolsList;
		using ResourceTuple = ResourceSetting::TupleList;
		using EntityInfo = std::vector<ComponentIndex>;
		using EntityPool = Pool<EntityInfo>;

		ComponentPools componentPools;
		ResourceTuple resourceTuple;
		EntityPool entityPool;

	};
	
	//Do operations in the world
	template <class ComponentSetting, class ResourceSetting>
	class Commands final
	{
	private:
		using World = World<ComponentSetting, ResourceSetting>;
		World& curWorld;

		template<class Type, class... Remains>
		void AddComponent(Entity entity, World::EntityInfo& entityInfo, const Type& component, const Remains&... remain)
		{
			constexpr ComponentID id = ComponentSetting::template GetID<Type>();
			ComponentInfo<Type>& cInfo = std::get<id>(curWorld.componentPools);
			ComponentIndex index = cInfo.pool.CreateObject();
			cInfo.pool.objects[index] = component;
			cInfo.entities.Add(entity);
			entityInfo[id] = index;

			if constexpr(sizeof...(Remains) > 0)
				AddComponent(entity, entityInfo, remain...);
		}

		template<class Type, class... Remains>
		void RemoveComponent(Entity entity, World::EntityInfo& entityInfo)
		{
			constexpr ComponentID id = ComponentSetting::template GetID<Type>();
			assert(entityInfo[id] != MAX_COMPONENTS);

			ComponentInfo<Type>& cInfo = std::get<id>(curWorld.componentPools);
			cInfo.pool.DestroyObject(entityInfo[id]);
			cInfo.entities.Remove(entity);
			entityInfo[id] = MAX_COMPONENTS;

			if constexpr(sizeof...(Remains) > 0)
				RemoveComponent<Remains...>(entity, entityInfo);
		}

		template<std::size_t curID>
		void RemoveAllComponent(Entity entity, World::EntityInfo& entityInfo)
		{
			//auto it = entityInfo.find(curID);
			if (entityInfo[curID] != MAX_COMPONENTS)
			{
				auto& cInfo = std::get<curID>(curWorld.componentPools);
				cInfo.pool.DestroyObject(entityInfo[curID]);
				cInfo.entities.Remove(entity);
			}
			if constexpr (curID < ComponentSetting::Size() - 1)
			{
				RemoveAllComponent <curID + 1>(entity, entityInfo);
			}
		}
	public:
		Commands(World& newWorld) : curWorld(newWorld) {}

		template <class... ARGS>
		Commands& CreateEntity(const ARGS&... args)
		{
			Entity entity = curWorld.entityPool.CreateObject();

			if(curWorld.entityPool.objects[entity].size() == 0)
				curWorld.entityPool.objects[entity].resize(ComponentSetting::Size(), MAX_COMPONENTS);
			AddComponent(entity, curWorld.entityPool.objects[entity], args...);
			return *this;
		}

		template <class... ARGS>
		Commands& AddComponent(Entity entity, const ARGS&... args)
		{
			AddComponent(entity, curWorld.entityPool.objects[entity], args...);
			return *this;
		}

		Commands& RemoveEntity(Entity entity)
		{
			auto& entityInfo = curWorld.entityPool.objects[entity];
			RemoveAllComponent<0>(entity, entityInfo);
			curWorld.entityPool.DestroyObject(entity);
			return *this;
		}

		template <class... ARGS>
		Commands& RemoveComponent(Entity entity)
		{
			RemoveComponent<ARGS...>(entity, curWorld.entityPool.objects[entity]);
			return *this;
		}

		template <class Type>
		Commands& CreateResource(const Type& type)
		{
			constexpr ResourceID id = ResourceSetting::template GetID<Type>();
			auto& resource = std::get<id>(curWorld.resourceTuple);
			resource = type;
			return *this;
		}

		template <class Type>
		Type& GetResource()
		{
			return std::get<ResourceSetting::template GetID<Type>()>(curWorld.resourceTuple);
		}

		template <class Type>
		Type& GetComponent(Entity entity)
		{
			constexpr ComponentID id = ComponentSetting::template GetID<Type>();
			auto& cInfo = std::get<id>(curWorld.componentPools);
			return cInfo.pool.objects[curWorld.entityPool.objects[entity][id]];
		}

		template <class Type>
		Type& operator [](Entity entity)
		{
			return GetComponent<Type>(entity);
		}
	};

	template <class ComponentSetting, class ResourceSetting>
	class Queryer final
	{
	private:
		using World = World<ComponentSetting, ResourceSetting>;
		World& curWorld;
		 
		template <class Cur, class... ARGS>
		bool checkEntity(Entity entity, World::EntityInfo& entityInfo)
		{
			constexpr ComponentID id = ComponentSetting::template GetID<Cur>();
			if (entityInfo[id] == MAX_COMPONENTS)
				return false;
			if constexpr (sizeof...(ARGS) == 0)
				return true;
			else
				return checkEntity<ARGS...>(entity, entityInfo);
		}


		template <class Type, class... ARGS>
		struct front
		{
			using type = Type;
		};
	public:

		Queryer(World& newWorld) : curWorld(newWorld) {}
		template <class... ARGS>
		std::vector<Entity> GetEntities()
		{
			std::vector<Entity> result;
			constexpr ComponentID id = ComponentSetting::template GetID<front<ARGS...>::type>();
			auto& cInfo = std::get <id> (curWorld.componentPools);
			const std::vector<Entity>& dense = cInfo.entities.GetDense();
			for (const Entity& entity : dense)
			{
				auto& entityInfo = curWorld.entityPool.objects[entity];
				if (checkEntity<ARGS...>(entity, entityInfo))
				{
					result.push_back(entity);
				}
			}
			return result;
		}
	};
}


#endif 
// !ECS
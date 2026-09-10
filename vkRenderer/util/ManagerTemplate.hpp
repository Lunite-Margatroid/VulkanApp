#pragma once

namespace LT {

	template<typename TypeID, typename T, typename TypeGetPtr, typename TypeRefIncrease, typename TypeRefDecrease>
	class ResourceRef {
		TypeID m_nID;
	public:
		ResourceRef() : m_nID(INVALID_ITEM_ID) {}

		explicit ResourceRef(TypeID id) : m_nID(id) {
			TypeRefIncrease{}(id);
		}

		~ResourceRef() {
			TypeRefDecrease{}(m_nID);
		}

		ResourceRef(const ResourceRef& other) {
			TypeRefDecrease{}(m_nID);
			m_nID = other.m_nID;
			TypeRefIncrease{}(m_nID);
		}

		ResourceRef(ResourceRef&& other) {
			TypeRefDecrease{}(m_nID);
			m_nID = other.m_nID;
			other.m_nID = INVALID_ITEM_ID;
		}

		ResourceRef& operator = (ResourceRef&& other) {
			if (this != &other) {
				TypeRefDecrease{}(m_nID);
				m_nID = other.m_nID;
				other.m_nID = INVALID_ITEM_ID;
			}
			return *this;
		}

		ResourceRef& operator = (const ResourceRef& other) {
			if (this != &other) {
				TypeRefDecrease{}(m_nID);
				m_nID = other.m_nID;
				TypeRefIncrease{}(m_nID);
			}
			return *this;
		}

		TypeID GetID()const { return m_nID; }

		T* GetPtr() {
			return TypeGetPtr{}(m_nID);
		}

		T* operator->() {
			return GetPtr();
		}

		void Release() {
			TypeRefDecrease{}(m_nID);
			m_nID = INVALID_ITEM_ID;
		}
	};

	template<typename T>
	struct PtrWithRefCount {
		T* m_ptr;
		unsigned int m_nRefCount;

		T* GetPtr() { return m_ptr; }
		PtrWithRefCount(T* p) : m_ptr(p) { m_nRefCount = 1; }
		PtrWithRefCount() : m_ptr(nullptr), m_nRefCount(0) {}
		~PtrWithRefCount() { }

		PtrWithRefCount(PtrWithRefCount&&) = default;
		PtrWithRefCount(const PtrWithRefCount&) = default;
		PtrWithRefCount& operator = (const PtrWithRefCount&) = default;
		PtrWithRefCount& operator = (PtrWithRefCount&&) = default;

		unsigned int DecreaseRefCount() {
			if (m_nRefCount > 0)
			{
				if (--m_nRefCount == 0)
				{
					Release();
				}
			}
			return m_nRefCount;
		}
		unsigned int IncreaseRefCount() {
			if (m_ptr)
			{
				++m_nRefCount;
			}
			return m_nRefCount;
		}

		unsigned int GetRefCount() const {
			return m_nRefCount;
		}

		void Release() noexcept {
			if (m_ptr)
			{
				delete m_ptr;
				m_ptr = nullptr;
			}
		}

		T* operator->() {
			return m_ptr;
		}
	};

	template<typename TypeID, typename T>
	class ManagerTemplate {
	protected:
		std::map<TypeID, PtrWithRefCount<T>> m_mapResources;
		long long m_nIDCounter;

		ManagerTemplate() : m_nIDCounter(0) {};
		virtual ~ManagerTemplate() {
			for (auto& resource : m_mapResources) {
				resource.second.Release();
			}
			m_mapResources.clear();
		}
		long long GenID() { return m_nIDCounter++; }

		T* GetResourcePtr(TypeID nID) {
			auto iter = m_mapResources.find(nID);
			if (iter == m_mapResources.end())
			{
				return nullptr;
			}
			return iter->second.GetPtr();
		}
	public:
		unsigned int RefIncrease(TypeID nID) {
			auto iter = m_mapResources.find(nID);
			if (iter == m_mapResources.end())
			{
				return 0;
			}
			LOG_TRACING("Manager: %p. Object RefIncrease. ID: %lld, refCount: %u", this, nID, iter->second.GetRefCount() + 1);
			return iter->second.IncreaseRefCount();
		}
		unsigned int RefDecrease(TypeID nID) {
			auto iter = m_mapResources.find(nID);
			if (iter == m_mapResources.end())
			{
				return 0;
			}
			unsigned int nRefCount = iter->second.DecreaseRefCount();
			if (nRefCount == 0)
			{
				m_mapResources.erase(nID);
			}
			LOG_TRACING("Manager: %p. Object RefDecrease. ID: %lld, refCount: %u", this, nID, nRefCount);
			return nRefCount;
		}
	};


#define DECLEAR_SINGLETON_MANAGER_BEGIN(ManagerType, TargetType, IDType, TargetName) \
	class ManagerType : public ManagerTemplate<IDType, TargetType>{\
	private:\
\
		ManagerType() = default;\
		~ManagerType();\
	private:\
		static ManagerType* s_pInstance;\
		\
	public:\
		static void Init();\
		static void Release();\
		static ManagerType& GetInstance();\
\
		static TargetType* Get##TargetName(IDType nID);\
\
struct _GetPtr{ \
	TargetType* operator ()(IDType nID)const {\
		return GetInstance().GetResourcePtr(nID);\
	}\
};\
\
struct _RefIncrease {\
unsigned int operator ()(IDType nID) const {\
	return GetInstance().RefIncrease(nID);\
	}\
};\
\
struct _RefDecrease {\
	unsigned int operator ()(IDType nID) const {\
		return GetInstance().RefDecrease(nID);\
		}\
	};\
	using _##TargetName##Ref = \
	ResourceRef<\
		IDType,\
		TargetType,\
		_GetPtr,\
		_RefIncrease,\
		_RefDecrease>; \
	\
	\
private:\
	_##TargetName##Ref Insert(IDType id, TargetType* pItem); \
private:

#define DECLEAR_SINGLETON_MANAGER_END(ManagerType, TargetType, IDType, TargetName) \
};\
using TargetName##Ref = ManagerType::_##TargetName##Ref;\
using TargetName##Ptr = PtrWithRefCount<TargetType>;\


#define IMPLEMENT_SINGLETON_MANAGER(ManagerType, TargetType, IDType, TargetName)\
\
ManagerType* ManagerType::s_pInstance = nullptr;\
ManagerType::~ManagerType(){\
	if(m_mapResources.size() > 0){\
		LOG_WARNING(#ManagerType": There are "#TargetName" unreleased.");\
	for(auto & [id, ptr] : m_mapResources)\
	{\
		LOG_WARNING("unreleased object id: %lld, refcount: %u", id, ptr.m_nRefCount);\
	}\
	}\
}\
void ManagerType::Init(){\
	if(!s_pInstance){s_pInstance = new ManagerType();}\
}\
void ManagerType::Release(){\
	if(s_pInstance){delete s_pInstance;s_pInstance = nullptr;}\
}\
\
ManagerType& ManagerType::GetInstance(){\
	if(!s_pInstance) {Init();} return *s_pInstance;\
	}\
TargetType* ManagerType::Get##TargetName(IDType nID)\
{\
	ManagerType& mgr = GetInstance();\
	return mgr.GetResourcePtr(nID); \
}\
TargetName##Ref ManagerType::Insert(IDType nID, TargetType* pItem){\
	if (m_mapResources.find(nID) != m_mapResources.end())\
	{\
		RENDERER_ASSERT(false, "Manager Repeating Insert.");\
		return TargetName##Ref(INVALID_ITEM_ID);\
	}\
	TargetName##Ptr ptrWarpper(pItem);\
	ptrWarpper.m_nRefCount = 0; \
	m_mapResources[nID] = ptrWarpper;\
	return TargetName##Ref(nID);\
}\

} // namespace
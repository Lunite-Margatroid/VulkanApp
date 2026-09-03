// 辅助函数
#pragma once

namespace LT
{
	namespace util
	{
		vk::Instance CreateVulkanInstance(const char* const* extensions, uint32_t nCount);

		template<typename T, typename MaskType>
		inline void SetBit(T& nFlag, MaskType mask) {
			nFlag |= (mask);
		}

		template<typename T, typename MaskType>
		inline void ResetBit(T& nFlag, MaskType mask) {
			nFlag &= (~(mask));
		}

		template<typename T, typename MaskType>
		inline void SetBit(T& nFlag, MaskType nMask, T nValue) {
			nFlag = (nFlag & ~nMask) | (nValue & nMask);
		}


		template<typename TypeID, typename T, typename TypeGetPtr, typename TypeRefIncrease, typename TypeRefDecrease>
		class ResourceRef {
			TypeID m_nID;
		public:
			ResourceRef(TypeID id) : m_nID(id) {
				TypeRefIncrease{}(id);
			}

			~ResourceRef() {
				TypeRefDecrease{}(m_nID);
			}

			ResourceRef(const ResourceRef& other) {
				m_nID = other.m_nID;
				TypeRefIncrease{}(m_nID);
			}

			ResourceRef(ResourceRef&& other) {
				m_nID = other.m_nID;
				other.m_nID = INVALID_ITEM_ID;
			}

			ResourceRef& operator = (ResourceRef&& other) {
				if (this != &other) {
					m_nID = other.m_nID;
					other.m_nID = INVALID_ITEM_ID;
				}
				return *this;
			}

			ResourceRef& operator = (const ResourceRef& other) {
				if (this != &other) {
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
		};

		template<typename T>
		struct PtrWithRefCount {
			T* m_ptr;
			unsigned int m_nRefCount;

			T* GetPtr() { return m_ptr; }
			PtrWithRefCount(T* p) : m_ptr(p) { m_nRefCount = 1; }
			~PtrWithRefCount() { Release(); }

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
				return ++ m_nRefCount; 
			}

			unsigned int GetRefCount() const {
				return m_nRefCount;
			}

			void Release() {
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
			
			ManagerTemplate() : m_nIDCounter (0){};
			virtual ~ManagerTemplate() {
				for (auto& resource) {
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
				return iter->second.IncreaseRefCount();
			}
			unsigned int RefDecrease(TypeID nID) {
				auto iter = m_mapResources.find(nID);
				if (iter == m_mapResources.end())
				{
					return 0;
				}
				return iter->second.DecreaseRefCount();
			}
		};
	} // namespace util


#define DECLEAR_SINGLETON_MANAGER_BEGIN(ManagerType, TargetType, IDType, TargetName) \
	class ManagerType : public util::ManagerTemplate<IDType, TargetType>{\
	private:\
		using TargetName##Ptr = util::PtrWithRefCount<TargetType>;\
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
private:

#define DECLEAR_SINGLETON_MANAGER_END(ManagerType, TargetType, IDType, TargetName) \
};\
	struct ManagerType##GetPtr {\
	IMesh* operator ()(IDType nID)const {\
		return ManagerType::Get##TargetName(nID);\
	}\
	};\
\
	struct ManagerType##RefIncrease {\
		unsigned int operator ()(IDType nID) const {\
			return ManagerType::GetInstance().RefIncrease(nID);\
		}\
	};\
\
	struct ManagerType##RefDecrease {\
		unsigned int operator ()(IDType nID) const {\
			return ManagerType::GetInstance().RefDecrease(nID);\
		}\
	};\
\
	using TargetName##Ref = util::ResourceRef<MeshID, IMesh, ManagerType##GetPtr, ManagerType##RefIncrease, ManagerType##RefDecrease>;\


#define IMPLEMENT_SINGLETON_MANAGER(ManagerType, TargetType, IDType, TargetName)\
\
ManagerType* ManagerType::s_pInstance = nullptr;\
ManagerType::~ManagerType(){\
	if(m_mapResources.size() > 0){\
		LOG_WARNING(#ManagerType": There are "#TargetName" unreleased.");\
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

}// namespace LT
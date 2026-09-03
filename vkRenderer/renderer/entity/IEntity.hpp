// 实体基类
#pragma once

namespace LT {
	using EntityID = int64_t;
	constexpr EntityID INVALID_ENTITY_ID = -1;

	class IEntity {
	protected:
		EntityID m_nID;
	public:
		IEntity(EntityID nID) 
			:m_nID(nID)
		{}
		virtual ~IEntity() = default;

		EntityID GetID const{ return m_nID; }

	};


} // namespace LT
// 实体基类
#pragma once

namespace LT {
	class IEntity {
	protected:
		const EntityID m_nID;
	public:
		IEntity(EntityID nID) 
			:m_nID(nID)
		{}
		virtual ~IEntity() = default;

		EntityID GetID() const{ return m_nID; }

	};


} // namespace LT
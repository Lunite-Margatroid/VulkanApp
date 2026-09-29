#pragma once

namespace LT {

#define MtlPropDataTypeList(X)\
	X(Bool, bool)\
	X(Int, int)\
X(Int2, int2)\
X(Int3, int3)\
X(Int4, int4)\
X(Float, float)\
X(Float2, float2)\
X(Float3, float3)\
X(Float4, float4)\
X(Image, Image)\


	enum class MtlPropDataType
	{
		eUnknown = -1,
#define X(type, strType) e##type,

		MtlPropDataTypeList(X)

#undef X
		MtlPropDataTypeCount

	};

	inline constexpr std::array<std::string_view, static_cast<size_t>(MtlPropDataType::MtlPropDataTypeCount)> g_arrMtlPropDataTypeStr = {
#define X(type, strType) #strType,

		MtlPropDataTypeList(X)

#undef X
	};

	constexpr std::string_view ToString(MtlPropDataType eType) {
		size_t index = static_cast<size_t>(eType);
		return index < g_arrMtlPropDataTypeStr.size() ? g_arrMtlPropDataTypeStr[index] : "Unknown";
	}

	// 在此处添加属性
#define MtlPropList(X)\
			X(ColorDiffuse, Float3)	\
			X(TexDiffuse, Image)\
			X(IOR, Float)\
			X(Alpha, Float)

	enum class MtlProp {
#define X(eProp, eType)	e##eProp,

		MtlPropList(X)
#undef X
		MtlPropCount,
		Padding, // 占位 4字节
	};


	struct MtlPropDescriptor {
		MtlProp eProp;
		MtlPropDataType eDataType;
		std::string_view strPropName;
		MtlPropDescriptor(MtlProp prop, MtlPropDataType dataType, std::string_view propName)
			: eProp(prop)
			, eDataType(dataType)
			, strPropName(propName)
		{
		}
	};

	inline constexpr std::array<std::string_view, static_cast<size_t>(MtlProp::MtlPropCount)> g_arrMtlPropStringList = {
#define X(eProp, eType) #eProp,

		MtlPropList(X)

#undef X
	};

	inline constexpr std::array<MtlPropDataType, static_cast<size_t>(MtlProp::MtlPropCount)> g_arrMtlPropDataTypeList = {
#define X(eProp, eType) MtlPropDataType::e##eType,

		MtlPropList(X)

#undef X

	};

	constexpr std::string_view ToString(MtlProp eProp) {
		size_t index = static_cast<size_t>(eProp);
		return index < g_arrMtlPropStringList.size() ? g_arrMtlPropStringList[index] : "Unknown";
	}

	constexpr MtlPropDataType GetMtlPropDataType(MtlProp eProp)
	{
		size_t index = static_cast<size_t>(eProp);
		return index < g_arrMtlPropDataTypeList.size() ? g_arrMtlPropDataTypeList[index] : MtlPropDataType::eUnknown;
	}

} // namespace LT
#pragma once

namespace LT {


	// 保证图片类型的属性在最后面
#define MtlPropDataTypeList(X)\
	X(Bool,		bool,	4)\
	X(Int,		int,	4)\
	X(Int2,		int2,	8)\
	X(Int3,		int3,	12)\
	X(Int4,		int4,	16)\
	X(Float,	float,	4)\
	X(Float2,	float2, 8)\
	X(Float3,	float3, 12)\
	X(Float4,	float4, 16)\
	X(Image,	Image,	0)\


	enum class MtlPropDataType
	{
		eUnknown = -1,
#define X(type, strType, nSize) e##type,
		MtlPropDataTypeList(X)
#undef X
		MtlPropDataTypeCount

	};

	inline constexpr std::array<std::string_view, static_cast<size_t>(MtlPropDataType::MtlPropDataTypeCount)> g_arrMtlPropDataTypeStr = {
#define X(type, strType, nSize) #strType,
		MtlPropDataTypeList(X)
#undef X
	};

	inline constexpr std::array<size_t, static_cast<size_t>(MtlPropDataType::MtlPropDataTypeCount)> g_arrSizeOfMtlPropDataType = {
#define X(type, strType, nSize) nSize,
		MtlPropDataTypeList(X)
#undef X
	};

	constexpr std::string_view ToString(MtlPropDataType eType) {
		size_t index = static_cast<size_t>(eType);
		return index < g_arrMtlPropDataTypeStr.size() ? g_arrMtlPropDataTypeStr[index] : "Unknown";
	}

	constexpr size_t SizeOf(MtlPropDataType ePropDataType) {
		size_t index = static_cast<size_t>(ePropDataType);
		return index < g_arrSizeOfMtlPropDataType.size() ? g_arrSizeOfMtlPropDataType[index] : 0;
	}

	// 在此处添加属性
#define MtlPropList(X)\
			X(ColorDiffuse, Float3)\
			X(TexDiffuse, Image)\
			X(ColorSpecular, Float3)\
			X(TexSpecular, Image)\
			X(IOR, Float)\
			X(TexIOR, Image)\
			X(Metalness, Float)\
			X(TexMeltalness, Image)\
			X(Roughness, Float)\
			X(TexRoughness, Image)\
			X(Alpha, Float)\
			X(TexAlpha, Image)\


	constexpr const char* MTL_PROP_UNIFORM_NAME = "u_MtlProp";


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

	constexpr size_t SizeOf(MtlProp eProp) {
		return SizeOf(GetMtlPropDataType(eProp));
	}

} // namespace LT
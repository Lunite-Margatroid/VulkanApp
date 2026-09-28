# ShaderProp

## Type

`bool`, `int`, `int2`, `int3`,`int4`, `float`,`float2`,`float3`, `float4`, `texture`

### `texture`

`Image`+`Sampler`

## 材质属性接口

### 属性标识

允许不同类型的材质拥有同名的属性

`MaterialType`+`MaterialProp`

### 描述符定义

```c++
// 材质属性的数据类型
enum class MtlPropDataType{
    eBool,
    eInt,
    eInt2,
    ...
    eImageID,
    ...
};
// 标识材质属性的枚举
enum class MtlProp{
    eDiffuseColor,
    eIOR,
    eRoughness,
    ...
};
// 材质属性描述
struct MtlPropDescriptor{
    MaterialPropDataType eType;
    MtlProp eProp;
    std::string strPropName;
};
// 所有材质属性的描述
std::vector<MtlProp, MtlPropTypeDescriptor> mapMtlProp;


```



### 泛型定义

```c++
using MaterialParam = std::variant<bool, int, int2, ... >;
```



### shell接口

```c++
EngineResult Engine::SetMtlProp(MaterialID nID, MaterialProp eProp, const MaterialParam& vParam);
```

### Buffer生成

#### 类型映射

从材质属性类型映射到Slang类型

| enum MtlPropDataType | Slang类型 |
| -------------------- | --------- |
| eBool                | int       |
| eInt                 | int       |
| eInt2                | int2      |
| ...                  |           |

#### 生成Slang缓冲定义

每一个Material对象包含一个`enum MtlProp`集合。生成材质属性const buffer，Image的Slang代码
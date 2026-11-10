## 菲涅尔项 Schlick近似

### 方程

$$
F_{Schlick}(F_0,\mathbf l, \mathbf h ) = F_0 + (1 - F_0)(1 - (\mathbf l \cdot \mathbf h))^5 \tag 1
$$

### slang代码

```c
float3 FresnelSchlick(float fCosTheta, float3 cF0){
	return cF0 + (float3(1.0, 1.0, 1.0) - cF0) * pow(max(0.0, 1.0 - fCosTheta), 5.0);
}
```

## 计算基础反射率$F_0$

### 方程

$$
F_0 = (\frac{ior - 1}{ior + 1})^2 \tag 2
$$

代入金属度和反射颜色
$$
F_0 = lerp((\frac{ior - 1}{ior + 1})^2, c_{spec}, metalness) \tag 3
$$

### slang代码

```c
float3 ComputeF0(float3 cSpecularColor, float fMetalness, float fIOR){
	float fF0Dielectric = (fIOR - 1.0) / (fIOR + 1.0);
	fF0Dielectric = fF0Dielectric * fF0Dielectric;
	return lerp(float3(fF0Dielectric, fF0Dielectric, fF0Dielectric), cSpecularColor, fMetalness);
}
```

## Phong NDF

### 方程

$$
D_p(\mathbf m) = \frac{\alpha_p + 2}{2\pi}(\mathbf n \cdot \mathbf m)^{\alpha_p} \tag{4}
$$

其中$\alpha_p \in [0, +\infin)$，$\alpha_p$越大表面越光滑。

### slang代码

```c
float PhongDistribution(float fRoughness, float fNDotH){
	float fAlpha = max(fRoughness, 0.001); // 防止除零
	float fShininess = 2.0 / (fAlpha * fAlpha) - 2.0;
	return (fShininess + 2.0) / (2.0 * BRDF_PI) * pow(max(fNDotH, 0.0), fShininess);
}
```

把"粗糙度"映射到$\alpha_p$:
$$
\alpha_p = \frac{2}{roughness^2} - 2 \tag5
$$
$roughness \in [0, 1]$, $\alpha\in[0, +\infin)$


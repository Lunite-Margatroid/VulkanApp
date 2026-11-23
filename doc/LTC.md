## Closed-Form Expression

设$D_o(\mathbf \omega_o)$是一个概率密度函数，而$\mathbf \omega_o$是来自$D_o$的随机变量。设随机变量$\mathbf \omega =  \frac {M\mathbf\omega_o}{||M \mathbf \omega_o||}$，那么随机变量$\mathbf \omega$的概率密度函数为：
$$
D(\mathbf \omega) = D_o(\mathbf \omega_o)\frac{\partial \omega_o}{\partial \mathbf \omega} = D_o(\frac{M^{-1}\mathbf \omega}{||M^{-1}\mathbf \omega||})\frac{|M^{-1}|}{||M^{-1}\mathbf \omega||^3}\tag 1
$$

## 线性变换的余弦分布

设$D_o(\mathbf \omega_o)$是一个球面余弦分布，$\mathbf \omega =  \frac {M\mathbf\omega_o}{||M \mathbf \omega_o||}$，则$\mathbf \omega$的概率密度函数$D(\mathbf \omega)$是线性变换的余弦分布。

BRDF函数$f_{brdf}(\omega_v, \omega_l, \alpha)$. 把$\mathbf \omega_v$和$\alpha$参数化，就可以得到$f_{brdf}(\mathbf \omega_l)$. 如果不考虑各向异性，则入射方向$\mathbf \omega_v$可以简化为$\theta_v$.

$f_{brdf}$是一个类余弦分布。使用线性变化的余弦分布拟合在不同$\mathbf \omega_v$和$\alpha$参数组合下的分布函数。就可以得到一系列的线性变化$M$.

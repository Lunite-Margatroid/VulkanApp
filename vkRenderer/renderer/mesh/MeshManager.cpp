// Mesh管理器
#include "vkRendererCommon.h"
#include "MeshManager.hpp"
#include "MeshStatic.hpp"
#include <cmath>

namespace LT {
	//IMPLEMENT_SINGLETON_MANAGER(MeshManager, IMesh, MeshID, Mesh)
	IMPLEMENT_SINGLETON_MANAGER(MeshManager, IMesh, MeshID, Mesh)


	MeshRef MeshManager::CreateStaticMesh() {
		MeshManager& mgr = GetInstance();
		MeshID id = mgr.GenID();
		MeshStatic* pMesh = new MeshStatic(id);
		return mgr.Insert(id, pMesh);
	}

	MeshRef MeshManager::CreateCube(float fSideLength) {
		MeshRef refMesh = CreateStaticMesh();
		// 初始化模型数据
		const float h = fSideLength * 0.5f;	// 半边长 正方体以原点为中心

		// 6个面 每个面4个顶点
		// 顶点顺序保证从外部看向该面时为逆时针(右手系 与frontFace=eCounterClockwise的正面判定一致)
		// 即 (v1-v0)x(v2-v0)指向该面朝外的法线方向
		MeshStatic::SPosition arrPosition[24] = {
			// +Z面
			{-h, -h,  h}, { h, -h,  h}, {-h,  h,  h}, { h,  h,  h},
			// -Z面
			{ h, -h, -h}, {-h, -h, -h}, { h,  h, -h}, {-h,  h, -h},
			// +X面
			{ h, -h,  h}, { h, -h, -h}, { h,  h,  h}, { h,  h, -h},
			// -X面
			{-h, -h, -h}, {-h, -h,  h}, {-h,  h, -h}, {-h,  h,  h},
			// +Y面
			{-h,  h, -h}, {-h,  h,  h}, { h,  h, -h}, { h,  h,  h},
			// -Y面
			{ h, -h, -h}, { h, -h,  h}, {-h, -h, -h}, {-h, -h,  h},
		};

		// 法线 与所在面的朝外方向一致
		MeshStatic::SNormal arrNormal[24] = {
			{0.f, 0.f, 1.f}, {0.f, 0.f, 1.f}, {0.f, 0.f, 1.f}, {0.f, 0.f, 1.f},
			{0.f, 0.f, -1.f}, {0.f, 0.f, -1.f}, {0.f, 0.f, -1.f}, {0.f, 0.f, -1.f},
			{1.f, 0.f, 0.f}, {1.f, 0.f, 0.f}, {1.f, 0.f, 0.f}, {1.f, 0.f, 0.f},
			{-1.f, 0.f, 0.f}, {-1.f, 0.f, 0.f}, {-1.f, 0.f, 0.f}, {-1.f, 0.f, 0.f},
			{0.f, 1.f, 0.f}, {0.f, 1.f, 0.f}, {0.f, 1.f, 0.f}, {0.f, 1.f, 0.f},
			{0.f, -1.f, 0.f}, {0.f, -1.f, 0.f}, {0.f, -1.f, 0.f}, {0.f, -1.f, 0.f},
		};

		// UV0 每个面都映射到[0,1]^2 从外部看向该面时贴图为正立方向
		MeshStatic::SUV arrUV[24] = {
			{0.f, 0.f}, {1.f, 0.f}, {0.f, 1.f}, {1.f, 1.f},
			{0.f, 0.f}, {1.f, 0.f}, {0.f, 1.f}, {1.f, 1.f},
			{0.f, 0.f}, {1.f, 0.f}, {0.f, 1.f}, {1.f, 1.f},
			{0.f, 0.f}, {1.f, 0.f}, {0.f, 1.f}, {1.f, 1.f},
			{0.f, 1.f}, {0.f, 0.f}, {1.f, 1.f}, {1.f, 0.f},
			{1.f, 0.f}, {1.f, 1.f}, {0.f, 0.f}, {0.f, 1.f},
		};

		// 索引 每个面2个三角形 (v0,v1,v2)和(v2,v1,v3) 从外部看向该面时均为逆时针
		IMesh::VertexIndex arrIndex[36] = {
			0, 1, 2,  2, 1, 3,
			4, 5, 6,  6, 5, 7,
			8, 9, 10, 10, 9, 11,
			12, 13, 14, 14, 13, 15,
			16, 17, 18, 18, 17, 19,
			20, 21, 22, 22, 21, 23,
		};

		refMesh->SetVertexPosition(arrPosition, 24);
		refMesh->SetNormal(arrNormal, 24);
		refMesh->SetUV(arrUV, 24, 0);
		refMesh->SetIndexBuffer(arrIndex, 36);
		refMesh->SetPrimitive(vk::PrimitiveTopology::eTriangleList);

		return refMesh;
	}

	MeshRef MeshManager::CreateSphere(float fRadius, uint32_t nLongSubdivision, uint32_t nLatSubdivision) {
		// 经细分最小为3 纬细分最小为1 此时退化为三角六面体(北极点+南极点+3个赤道顶点)
		RENDERER_ASSERT(nLongSubdivision >= 3u, "nLongSubdivision must >= 3.");
		RENDERER_ASSERT(nLatSubdivision >= 1u, "nLatSubdivision must >= 1.");

		MeshRef refMesh = CreateStaticMesh();

		constexpr float fPi = 3.14159265f;

		// 顶点数: 北极点 + nLatSubdivision圈纬度环(每圈nLongSubdivision个顶点) + 南极点
		const uint32_t nVertexCount = nLatSubdivision * nLongSubdivision + 2;
		std::vector<IMesh::SPosition> vecPosition(nVertexCount);
		std::vector<IMesh::SNormal> vecNormal(nVertexCount);
		std::vector<IMesh::SUV> vecUV(nVertexCount);

		// 顶点布局: 北极点=0 第i圈纬度环(i=1..nLatSubdivision)的顶点为[1+(i-1)*nLongSubdivision, 1+i*nLongSubdivision) 南极点=nVertexCount-1
		auto RingVertexIndex = [nLongSubdivision](uint32_t nRing, uint32_t nColumn) -> IMesh::VertexIndex {
			return 1 + (nRing - 1) * nLongSubdivision + nColumn;
		};

		// 北极点 极角θ=0
		vecPosition[0] = { 0.f, fRadius, 0.f };
		vecNormal[0] = { 0.f, 1.f, 0.f };
		// 极点处方位角φ无定义 取纹理宽度的中点
		vecUV[0] = { 0.5f, 0.f };

		// 纬度环 从北极到南极均匀分布 θ=i*π/(nLatSubdivision+1)
		for (uint32_t i = 1; i <= nLatSubdivision; i++)
		{
			const float fTheta = fPi * i / static_cast<float>(nLatSubdivision + 1);
			const float fSinTheta = sin(fTheta);
			const float fCosTheta = cos(fTheta);
			for (uint32_t j = 0; j < nLongSubdivision; j++)
			{
				// 方位角 φ=j*2π/nLongSubdivision
				const float fPhi = 2.f * fPi * j / static_cast<float>(nLongSubdivision);
				const float fSinPhi = sin(fPhi);
				const float fCosPhi = cos(fPhi);

				const IMesh::VertexIndex nIndex = RingVertexIndex(i, j);
				// 右手系 Y轴向上
				vecPosition[nIndex] = { fRadius * fSinTheta * fCosPhi, fRadius * fCosTheta, fRadius * fSinTheta * fSinPhi };
				// 球面法线 与位置同向(长度已是1)
				vecNormal[nIndex] = { fSinTheta * fCosPhi, fCosTheta, fSinTheta * fSinPhi };
				// 等距球柱投影 u=φ/2π v=θ/π(北极v=0 南极v=1)
				vecUV[nIndex] = { static_cast<float>(j) / nLongSubdivision, static_cast<float>(i) / (nLatSubdivision + 1) };
			}
		}

		// 南极点 θ=π
		vecPosition[nVertexCount - 1] = { 0.f, -fRadius, 0.f };
		vecNormal[nVertexCount - 1] = { 0.f, -1.f, 0.f };
		vecUV[nVertexCount - 1] = { 0.5f, 1.f };

		// 索引 三角形总数=北极盖nLongSubdivision + 环带2*nLongSubdivision*(nLatSubdivision-1) + 南极盖nLongSubdivision
		std::vector<IMesh::VertexIndex> vecIndex(static_cast<size_t>(2 * nLongSubdivision * nLatSubdivision) * 3);
		uint32_t nIndexPos = 0;

		// 北极盖 (N, col[j+1], col[j]) 保证(v1-v0)x(v2-v0)指向球外
		for (uint32_t j = 0; j < nLongSubdivision; j++)
		{
			vecIndex[nIndexPos++] = 0;
			vecIndex[nIndexPos++] = RingVertexIndex(1, (j + 1) % nLongSubdivision);
			vecIndex[nIndexPos++] = RingVertexIndex(1, j);
		}

		// 纬度环带 每段2个三角形
		for (uint32_t i = 1; i < nLatSubdivision; i++)
		{
			for (uint32_t j = 0; j < nLongSubdivision; j++)
			{
				const IMesh::VertexIndex nUpperJ = RingVertexIndex(i, j);
				const IMesh::VertexIndex nUpperJNext = RingVertexIndex(i, (j + 1) % nLongSubdivision);
				const IMesh::VertexIndex nLowerJNext = RingVertexIndex(i + 1, (j + 1) % nLongSubdivision);
				const IMesh::VertexIndex nLowerJ = RingVertexIndex(i + 1, j);

				vecIndex[nIndexPos++] = nUpperJ;
				vecIndex[nIndexPos++] = nUpperJNext;
				vecIndex[nIndexPos++] = nLowerJNext;

				vecIndex[nIndexPos++] = nUpperJ;
				vecIndex[nIndexPos++] = nLowerJNext;
				vecIndex[nIndexPos++] = nLowerJ;
			}
		}

		// 南极盖 (S, col[j], col[j+1])
		const IMesh::VertexIndex nSouthPoleIndex = nVertexCount - 1;
		for (uint32_t j = 0; j < nLongSubdivision; j++)
		{
			vecIndex[nIndexPos++] = nSouthPoleIndex;
			vecIndex[nIndexPos++] = RingVertexIndex(nLatSubdivision, j);
			vecIndex[nIndexPos++] = RingVertexIndex(nLatSubdivision, (j + 1) % nLongSubdivision);
		}

		refMesh->SetVertexPosition(vecPosition.data(), nVertexCount);
		refMesh->SetNormal(vecNormal.data(), nVertexCount);
		refMesh->SetUV(vecUV.data(), nVertexCount, 0);
		refMesh->SetIndexBuffer(vecIndex.data(), static_cast<uint32_t>(vecIndex.size()));
		refMesh->SetPrimitive(vk::PrimitiveTopology::eTriangleList);

		return refMesh;
	}
} // namespace LT
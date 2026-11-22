#pragma once

namespace LT {
	class Camera2D {
	private:
		glm::vec2 m_position;

		// X轴朝右 Y轴朝上
		// 右手系 绕Z轴旋转
		// 弧度 逆时针为正
		float m_fYaw;

		// 视口宽度(世界空间单位)
		float m_fWidth;
		// 宽高比 (宽 / 高)
		float m_fAspect;

		mutable bool m_bDirt;

		// 缓存 世界空间 -> NDC
		mutable glm::mat2 m_VPMat;
		// 缓存 相机位置在NDC空间的偏移
		// 着色器中将该值加在VP矩阵变换之后
		mutable glm::vec2 m_offset;

	private:
		// 脏标记时重新计算VP矩阵与偏移
		void UpdateMat() const;

	public:

		Camera2D();
		~Camera2D() = default;

		const glm::mat2& GetVPMat() const;
		const glm::vec2& GetOffset() const;

		void SetPosition(const glm::vec2& position);
		const glm::vec2& GetPosition() const;

		void SetWidth(float fWidth);
		float GetWidth() const;

		void SetYaw(float fYaw);
		float GetYaw() const;

		void SetAspect(float fAspect);
		float GetAspect() const;

	};
}

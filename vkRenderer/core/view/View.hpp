#pragma once
#include "Node.hpp"
#include "Layer.hpp"

namespace LT {
	class View {
	protected:
		// 主场景的节点
		Node* m_pMainScene;

		// 索引为0的layer在最底层
		std::vector<Layer*> m_layers;

	public:
		View();
		virtual ~View() = default;

		// 三五法则:删除拷贝和移动
		View(const View&) = delete;
		View& operator=(const View&) = delete;
		View(View&&) = delete;
		View& operator=(View&&) = delete;

		void RenderFrame();
		void SetMainScene(Node* pScene) {m_pMainScene = pScene;}

	};
} // namespace LT

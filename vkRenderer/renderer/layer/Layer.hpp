#pragma once

namespace LT {

	class Pipeline;

	class Layer {
	private:
		std::vector<Pipeline*> m_vecPipelines;
	public:
		Layer();
		virtual ~Layer();

		// 三五法则:删除拷贝和移动
		Layer(const Layer&) = delete;
		Layer& operator=(const Layer&) = delete;
		Layer(Layer&&) = delete;
		Layer& operator=(Layer&&) = delete;

		// 暂时没用
		virtual void Draw() = 0;

		const std::vector<Pipeline*>& GetPipelines() const;

		template<typename TPipeline>
		void RegisterPipeline() {
			m_vecPipelines.push_back(new TPipeline());
		}

		// ----- static -----

	};
} // namespace LT

#pragma once
namespace LT {

	class ImgRes
	{
	private:
		int m_Channals;
		int m_Width;
		int m_Height;
		int m_Depth;
		int m_ResultCode;
		unsigned char* m_Data;
	public:
		ImgRes(const std::string& path, int nTargetDepth);
		ImgRes() = delete;
		~ImgRes();
		ImgRes(const ImgRes&) = delete;
		ImgRes& operator = (const ImgRes&) = delete;

		inline int GetWidth() const {
			return m_Width;
		}
		inline int GetHeight() const {
			return m_Height;
		}
		inline int GetChannal() const {
			return m_Channals;
		}

		inline int GetDepth() const {
			return m_Depth;
		}

		inline int GetPixelCount() const
		{
			return m_Width * m_Height;
		}

		const unsigned char* GetDataPtr() const {
			return m_Data;
		}


		void Release();
		bool Resize(int newSizeWidth, int newSizeHeight);
	};

} // namespace LT
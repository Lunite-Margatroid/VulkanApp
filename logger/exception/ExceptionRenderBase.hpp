#pragma once
namespace LT {
	class ExceptionRenderBase : public std::exception{
	protected:
		std::string m_strMsg;
		
	public:
		explicit ExceptionRenderBase(const std::string& msg);

		const char* what() const throw() override;

	protected:
		void SetMessage(const std::string& newMsg);
		void SetMessage(std::string&& newMsg) noexcept;

	};

	using RunTimeExcept = ExceptionRenderBase;

#define DECLEAR_CUSTOMED_EXCEPTION(Derived_Except_Class, Type_Name, Add_Message)\
	class Type_Name : public Derived_Except_Class{\
	public:\
		explicit Type_Name(const std::string& msg) : Derived_Except_Class(msg){\
			if(m_strMsg.length() > 0) \
				m_strMsg = std::string(Add_Message) + " " + m_strMsg;\
			else\
				m_strMsg = (Add_Message);\
			}\
\
		Type_Name() : Derived_Except_Class(std::string("")){\
				m_strMsg = (Add_Message);\
			}\
	}


#define DECLEAR_CUSTOMED_EXCEPTION_DERIVE_FROM_RUNTIME_EXCEPT(Type_Name, Add_Message) DECLEAR_CUSTOMED_EXCEPTION(RunTimeExcept, Type_Name, Add_Message)

}
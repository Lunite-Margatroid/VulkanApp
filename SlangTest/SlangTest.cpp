// Slang测试程序
#include <vector>
#include <string>
#include <filesystem>
#include "logger.hpp"
#include "SlangCompiler.hpp"

#include <iostream>

#include <sstream>

const char* TypeReflectionKind2String(slang::TypeReflection::Kind);

std::string SlangResourceShape2String(SlangResourceShape);

int main(int argn, const char* const* argv) {
	LT::SlangCompiler::Init();

	// 编译测试

	std::vector<std::string> modules({
		"FragmentShaderMainTex",
		"CommonVertexShader"
		});

	std::vector<std::pair<const char*, const char*>> vecPPMacro(
		{
			{"VERT_POSITION" , "1"},
			{"VERT_UV0", "1"}
		}
	);
	{
		Slang::ComPtr<slang::IComponentType>pProgram;
		Slang::ComPtr<slang::ISession> pSession;
		LT::SlangCompiler::GetInstance().CompileShaderToProgram(modules, vecPPMacro, pSession .writeRef(), pProgram.writeRef());

		if (pProgram)
		{

			// 反射测试
			slang::ProgramLayout* pLayout = pProgram->getLayout();
			if (pLayout)
			{

				pLayout->getGlobalConstantBufferSize();
				pLayout->getGlobalConstantBufferBinding();

				uint32_t nParamCount = pLayout->getParameterCount();
				for (int i = 0; i < nParamCount; ++i)
				{
					slang::VariableLayoutReflection* pVar = pLayout->getParameterByIndex(i);
					if (pVar) {
						
						std::cout << "Var: " << pVar->getName() << std::endl;
						auto pType = pVar->getType();
						std::cout << "\t\tTypeName:" << pType->getName() << std::endl;
						std::cout << "\t\tKind:" << TypeReflectionKind2String(pType->getKind()) << std::endl;
						if (pType->getKind() == slang::TypeReflection::Kind::Resource)
						{
							std::cout << "\t\tShape: " << SlangResourceShape2String(pType->getResourceShape()) << std::endl;
						}

						std::cout << "\t\tBinding Sapce: " << pVar->getBindingSpace() << std::endl;
						std::cout << "\t\tBinding Index: " << pVar->getBindingIndex() << std::endl;
					}

					
				}


				// print json
				if (false)
				{
					Slang::ComPtr<slang::IBlob> pLayoutJson;
					pLayout->toJson(pLayoutJson.writeRef());
					if (pLayoutJson)
					{
						LOG_INFO("Shader Layout Json:\n%s", static_cast<const char*>(pLayoutJson->getBufferPointer()));
					}
				}
			}
		}


	}
	LT::SlangCompiler::Release();
	return 0;
}


const char* TypeReflectionKind2String(slang::TypeReflection::Kind eKind) {
	switch (eKind) {
		case slang::TypeReflection::Kind::None :
			return "None";
			break;
		case slang::TypeReflection::Kind::Struct:
			return "Struct";
			break;
		case slang::TypeReflection::Kind::Array:
			return "Array";
			break;
		case slang::TypeReflection::Kind::Matrix:
			return "Matrix";
			break;
		case slang::TypeReflection::Kind::Vector:
			return "Vector";
			break;
		case slang::TypeReflection::Kind::Scalar:
			return "Scalar";
			break;
		case slang::TypeReflection::Kind::ConstantBuffer:
			return "ConstantBuffer";
			break;
		case slang::TypeReflection::Kind::Resource:
			return "Resource";
			break;
		case slang::TypeReflection::Kind::SamplerState:
			return "SamplerState";
			break;
		case slang::TypeReflection::Kind::TextureBuffer:
			return "TextureBuffer";
			break;
		case slang::TypeReflection::Kind::ShaderStorageBuffer:
			return "ShaderStorageBuffer";
			break;
		case slang::TypeReflection::Kind::ParameterBlock:
			return "ParameterBlock";
			break;
		case slang::TypeReflection::Kind::GenericTypeParameter:
			return "GenericTypeParameter";
			break;
		case slang::TypeReflection::Kind::Interface:
			return "Interface";
			break;
		case slang::TypeReflection::Kind::OutputStream:
			return "OutputStream";
			break;
		case slang::TypeReflection::Kind::Specialized:
			return "Specialized";
			break;
		case slang::TypeReflection::Kind::Feedback:
			return "Feedback";
			break;
		case slang::TypeReflection::Kind::Pointer:
			return "Pointer";
			break;
		case slang::TypeReflection::Kind::DynamicResource:
			return "DynamicResource";
			break;
		case slang::TypeReflection::Kind::MeshOutput:
			return "MeshOutput";
			break;
		default:
			break;
	}

	return "None";

}

std::string SlangResourceShape2String(SlangResourceShape nShape) {
	std::stringstream oss;
	if (nShape & SlangResourceShape::SLANG_TEXTURE_1D)
	{
		oss << "Texture1D";
	}
	else if (nShape & SlangResourceShape::SLANG_TEXTURE_2D)
	{
		oss << "Texture2D";
	}
	else if (nShape & SlangResourceShape::SLANG_TEXTURE_3D)
	{
		oss << "Texture3D";
	}
	else if (nShape & SlangResourceShape::SLANG_TEXTURE_CUBE)
	{
		oss << "TextureCube";
	}

	if (nShape & SlangResourceShape::SLANG_TEXTURE_ARRAY_FLAG)
	{
		oss << " Array";
	}
	return oss.str();
}
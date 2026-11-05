#include "LunarTextIO.hpp"
#include "TextTable.hpp"
#include "ImgRes.hpp"

#include <vector>
#include <span>

int main()
{
	//LTIO::LunarTestIO_Test();

	std::vector<char> ascii;
	for (char c = 32; c < 127; ++c) {
		ascii.push_back(c);
	}

	std::vector<wchar_t> wide{L'你', L'好', L'世', L'界'};
	

	LTIO::TextTable<char> asciiTable(std::span<char>(ascii.data(), ascii.size()), 64);

	asciiTable.GetImage().SavaImage("./asciiTable.png");

	LTIO::TextTable<wchar_t> wcharTable(std::span<wchar_t>(wide.data(), wide.size()), 64);
	wcharTable.GetImage().SavaImage("./helloworld.png");

}
#pragma once
class Converter
{
public:
	static std::wstring ConvertStringToWString(const std::string& string);
	static Elite::FPoint3 NDCtoRasterSpace(const Elite::FPoint3& vertex, float screenWidth, float screenHeight);
private:
	Converter() = default;
};


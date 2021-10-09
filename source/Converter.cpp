#include "pch.h"
#include "Converter.h"
#include <locale>
#include <codecvt>
#include <string>

//https://stackoverflow.com/questions/2573834/c-convert-string-or-char-to-wstring-or-wchar-t
std::wstring Converter::ConvertStringToWString(const std::string& sourceString)
{
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    std::wstring wide = converter.from_bytes(sourceString);
    
    return wide;
}

Elite::FPoint3 Converter::NDCtoRasterSpace(const Elite::FPoint3& vertex, float screenWidth, float screenHeight)
{
    Elite::FPoint3 screenSpace{};
    screenSpace.x = ((vertex.x + 1) / 2.f) * screenWidth;
    screenSpace.y = ((1 - vertex.y) / 2.f) * screenHeight;
    screenSpace.z = vertex.z;

    return screenSpace;
}

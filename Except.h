#pragma once
#include <string>
#include <exception>


class ParserException : public std::exception
{
public:
	ParserException(const char* error,
		int Line, int Col);
        const char* what() const noexcept override;
	

private: 
	std::string m_error;
};

ParserException::ParserException(const char* error, int Line, int Col) {
	m_error = static_cast<std::string> (error) +
		"  line:" + std::to_string(Line + 1) + ", column" + std::to_string(Col + 1);
}
const char* ParserException::what() const noexcept {
	return m_error.c_str();  //преобразует строку C++ в строку в стиле C
}



//nodiscard указывает, что возвращаемое функцией значение 
//нельзя игнорировать и нужно сохранить в какую - либо переменную
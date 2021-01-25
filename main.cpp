// Parser2.cpp : Этот файл содержит функцию "main". Здесь начинается и заканчивается выполнение программы.
//
#include "CSVParser.h"
#include <iostream>
#include "Except.h"
#include <fstream>


    int main(int argc, char* argv[])
    {

		std::string csvtable;
		argc -= (argc > 0); argv += (argc > 0); // skip program name argv[0] if present
		if (argc > 0)
			csvtable = argv[0];
		else 
			csvtable = "tabl.csv";
		try
		{
			std::ifstream file(csvtable);
			//std::ifstream file("tabl.txt");
			if (!file.is_open()) {
				throw std::runtime_error("Can`t open this file");
			}
			if (csvtable.empty()) {
				throw std::runtime_error("File is empty");
			}
			

			//экземпляр класса CSVParser (шаблон) и в контсруктор
			//CSVParser<std:: string, double> parser(file, 0);
			CSVParser<int, std::string, std::string, std::string, double> parser(file, 0);
			

		//	for (std::tuple<std::string, double> pr : parser)
			for (std::tuple<int, std::string, std::string, std::string, double> pr : parser)
				std::cout << pr << std::endl;
		}
		catch (const std::exception& e) {
			std::cout << e.what() << std::endl;
		}

        return 0;
    }
   
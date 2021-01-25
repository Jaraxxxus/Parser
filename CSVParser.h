#pragma once

#ifndef INC_0B_CSVPARSER_H
#define INC_0B_CSVPARSER_H

#include <fstream>
#include <typeinfo>
#include <regex>
#include "Tupl.h"
#include "Except.h"

using namespace std;

template<class ... Args>
class CSVParser
{
private:
    ifstream& input;  //файл
    size_t offset;    //сдвиг по файлу
    int fileLength = -1;      //кол-во строк
    char fieldDelimiter = '"';   //разделители столбцоа, строк, символы зкранировани€
    char columnDelimiter = ';';
    char lineDelimiter = '\n';

    enum class ParsingState  //режим разбора пол€ (символ или экранировани€)
    {
        simpleRead,
        delimiterReading
    };
    //=========получение  строки из файла
    //ќписывает объект, управл€ющий извлечением элементов
    //и закодированных объектов из буфера потока с элементами типа Char_T,
    //также называемого char_type. »х признаки символов определ€ютс€ классом Tr, также называемым traits_type.
    template<typename _CharT, typename _Traits, typename _Alloc>
    void getLine(basic_istream<_CharT, _Traits>& is, basic_string<_CharT, _Traits, _Alloc>& str)
    {
        str.clear();

        char c;
        while (is.get(c))
        {
            if (c == lineDelimiter)
                break;
            str.push_back(c);
        }
    }
    
    bool emptyTailToDelimiter(string a, int pos)
    {
        for (int i = pos; i < a.size(); ++i)
            if (a[i] == columnDelimiter)
                break;
            else
                if (a[i] != ' ')
                    return false;

        return true;
    }
    //работа со строками -замена подстроки: ^начало текста, $конец ткст, +-одно или более, s-пробел
    string ltrim(const string& str)
    {
        return regex_replace(str, regex("^\\s+"), string(""));
    }
    //работа со строками 
    string rtrim(const string& str)
    {
        return regex_replace(str, regex("\\s+$"), string(""));
    }
    //работа со строками 
    string trim(const string& str)
    {
        return ltrim(rtrim(str));
    }

    //==кол-во строк в файле
    int getLength()
    {
        if (fileLength == -1)
        {
            input.clear();
            input.seekg(0, ios::beg);

            string line;
            for (fileLength = 0; getline(input, line); fileLength++);

            input.clear();
            input.seekg(0, ios::beg);
        }
        return fileLength;
    }

    class CSVIterator
    {
    private:
        string strBuffer;  //текуща€ строка
        ifstream& input;   //файл, с которым работаем
        size_t index;      //номер строки
        CSVParser<Args...>& parent;    //экземпл€р парсера
        bool last = false;   //признак конца файда

        friend class CSVParser;

    public:
        CSVIterator(ifstream& ifs, size_t index, CSVParser<Args...>& parent) : index(index), parent(parent), input(ifs)
        {
            for (int i = 0; i < index - 1; i++, parent.getLine(input, strBuffer));

            parent.getLine(input, strBuffer);  //получение строки по номеру
            if (!input)
                throw logic_error("Bad file!");
        }

        CSVIterator operator++()
        {
            if (index < parent.fileLength)
            {
                index++;
                input.clear();
                input.seekg(0, ios::beg);
                for (int i = 0; i < index - 1; ++i, parent.getLine(input, strBuffer));

                parent.getLine(input, strBuffer);
            }
            else
            {
                strBuffer = "";
                last = true;
            }

            return *this;  //строку
        }

        bool operator==(const CSVIterator& it) const
        {
            return this->last == it.last && this->index == it.index && this->strBuffer == it.strBuffer;
        }

        bool operator!=(const CSVIterator& it)
        {
            return !(*this == it);
        }

        tuple<Args...> operator*()
        {
            return parent.parseLine(strBuffer, index);
        }
    };

public:
    explicit CSVParser(ifstream& ifs, size_t offset) : input(ifs), offset(offset)  //файл, смещение
    {  //доп.проверка, в майне она сделана.
        if (!ifs.is_open())
            throw std::invalid_argument("Can't open file");
        if (offset >= getLength())
            throw logic_error("Bad file offset! offset >= file");
        if (offset < 0)
            throw logic_error("Bad file offset! offset < 0");
    }

    void setDelimiters(char newFieldDelimiter, char newColumnDelimiter, char newLineDelimiter)
    {
        lineDelimiter = newLineDelimiter;
        fieldDelimiter = newFieldDelimiter;
        columnDelimiter = newColumnDelimiter;
    }

  
    CSVIterator begin()
    {
        CSVIterator it(input, offset + 1, *this);//файл, смещение, parser (104)
        return it;
    }

    CSVIterator end()
    {
       // int t = getLength();

        CSVIterator it(input, 1, *this);
        it.last = true;
        it.strBuffer = "";
        it.index = getLength();
        return it;

    }
    // работа со строкой файла, разбивка по пол€м
    vector<string> read_string(string& line, int lineNum)
    {
        vector<string> fields{ "" };
        ParsingState state = ParsingState::simpleRead;
        size_t fcounter = 0;  //є пол€
        size_t linePos = 0;//позици€ символа
        bool filled = false;
        bool accessWriteDelim = false;
        line = trim(line);
        for (char c : line)
        {
            if (state == ParsingState::simpleRead)
            {
                if (c == columnDelimiter)  //разделитель полей
                {
                   
                    fields[fcounter] = trim(fields[fcounter]);
                    fields.emplace_back("");  //в конец вектора
                    fcounter++;
                    filled = false;
                }
                else if (c == fieldDelimiter)  //симв экранировани€
                {
                    if (linePos > 0 && filled)
                        throw ParserException("Smth wrong (field delimiter isn`t first!) ", lineNum, fcounter);
                      

                    fields[fcounter] = trim(fields[fcounter]);
                    state = ParsingState::delimiterReading;
                    accessWriteDelim = false;
                }
                else
                {
                    if (c != ' ')
                        filled = true;

                    fields[fcounter].push_back(c);
                }
            }
            else 
            {
                if (c == fieldDelimiter)  //экранирование
                {
                    if (!accessWriteDelim && line.size() > linePos + 1 && line[linePos + 1] != fieldDelimiter)  //2 разделител€ без 3 завершающегос€
                    {
                        if (!emptyTailToDelimiter(line, linePos + 1)) {
                            throw ParserException("Smth wrong (symbols after delimiter in field!) ", lineNum, fcounter);

                        }
                        state = ParsingState::simpleRead;
                    }
                    else if (!accessWriteDelim && linePos == line.size() - 1)  //дошли до конца строки
                    {
                        state = ParsingState::simpleRead;
                    }
                    else if (!accessWriteDelim)
                        accessWriteDelim = true;
                    else
                    {
                        fields[fcounter].push_back(c);
                        accessWriteDelim = false;
                    }
                }
                else
                    fields[fcounter].push_back(c);
            }
            linePos++;
        }
        if (state != ParsingState::simpleRead)
            throw ParserException("Smth wrong (inc field) ", lineNum, fcounter);
        return fields;
    }

    tuple<Args...> parseLine(string& line, int number)
    {
        size_t size = sizeof...(Args);  //размер кортежа

        if (line.empty()) {
            
            throw invalid_argument("Line " + to_string(number) + " is empty!");
        }
        tuple<Args...> table_str;  //структура ожидаема€
        vector<string> fields = read_string(line, number); //получить пол€ строки line  под номером number-массив строк-полей

        if (fields.size() != size) {  //кол-во полей в строке не совпадает с размерностью кортежа
            
            throw invalid_argument("Wrong number of fields in line " + to_string(number) + "!");
        }
        auto a = fields.begin();  //по массиву полей итератор
        try
        {
            parser_utils::parse(table_str, a); //разбор- запуск обхода, печати-tupl
        }
        catch (exception& e)
        {
            throw invalid_argument("Line " + to_string(number) + " contains bad types!");
        }

        return table_str;
    }
};

#endif


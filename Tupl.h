
#pragma once
#include <tuple>
#include <functional>
#include <iostream>
#include <sstream>
#include <vector>
//http://artlang.net/post/c++11-obkhod-elementov-kortezhe-std-tuple/
// Главная роль здесь у шаблона класса iterate_tuple.
    // Первый параметр шаблона класса iterate_tuple имеет тип int (index).
    // Значение этого параметра используется функцией get, 
    // которая "достает" из кортежа элемент по указанной позиции.
    // Мы рекурсивно сдвигаем позицию (уменьшаем index на 1) и таким образом
    // перемещаемся по кортежу.
    // Когда значение индекса становится равно 0, рекурсия завершается,
    // благодаря частичной специализации для индекса равного 0.
    // При этом есть особый случай, когда индекс равен -1. Он соответствует 
    // ситуации, когда кортеж не содержит ни одного элемента.
template<int index, typename Callback, typename... Args>
struct iterate
{
    static void next(std::tuple<Args...>& t, Callback callback, std::ostream& stream)
    {
        // Уменьшаем позицию и рекурсивно вызываем этот же метод
        iterate<index - 1, Callback, Args...>::next(t, callback, stream);
        // Вызываем обработчик и передаем ему позицию и значение элемента
        callback(index, std::get<index>(t), stream);
    }
};
// Частичная специализация для индекса 0 (завершает рекурсию)
template<typename Callback, typename... Args>
struct iterate<0, Callback, Args...>
{
    static void next(std::tuple<Args...>& t, Callback callback, std::ostream& stream)
    {
        callback(0, std::get<0>(t), stream);
    }
};
// Частичная специализация для индекса -1 (пустой кортеж)
template<typename Callback, typename... Args>
struct iterate<-1, Callback, Args...>
{
    static void next(std::tuple<Args...>& t, Callback callback, std::ostream& stream) {
        // ничего не делаем
    }
};
// "Волшебный" for_each для обхода элементов кортежа (compile time!):
template<typename Callback, typename... Args>
void forEach(std::tuple<Args...>& t, Callback callback, std::ostream& stream)
{
    // Размер кортежа
    int const t_size = std::tuple_size<std::tuple<Args...>>::value;
    // Запускаем рекурсивный обход элементов кортежа во время компиляции
    iterate<t_size - 1, Callback, Args...>::next(t, callback, stream);
}

struct callback
{
    template<typename T>
    void operator()(int index, T&& t, std::ostream& stream) // index - это позиция элемента в кортеже
    {                                                       // t - значение элемента
        stream << t << "|";
    }
};

//inline-функция —  чье тело подставляется в каждую точ­ку вызова, 
//вместо  генерации кода вызова.
template<typename _CharT, typename _Traits, typename... Args>
 std::basic_ostream<_CharT, _Traits>& operator<<(std::basic_ostream<_CharT, _Traits>& stream, std::tuple<Args...>& t)
{
    // Обход элементов кортежа и вызвов обработчика
    forEach(t, callback(), stream);

    return stream;
}
//   Работа с кортежом- этап итерации по кортежу (печать другой шаблон, поэтому те же функции но с другим шаблоном выше (с начала до 76стр)
namespace parser_utils
{
    // для работы с универсальными типами
    //для каждого набора параметров компилятор создаёт отдельный экземпляр функции или класса
    //Ключевое слово typename появилось сравнительно недавно, поэтому стандарт[1]
    //допускает использование class вместо typename
    template <class Dest>
    class lexical_cast
    {
        Dest value;
    public: 
        lexical_cast(const std::string& src)  //конвертируем из строк числа
        {
            std::stringstream s;
            s << src;
            s >> value;
            if (!s)
                throw std::exception();
        }

        operator const Dest& () const {
            return value;
        }

        operator Dest& () {
            return value;
        }
    };
    //отдельный случай для строки-поля
//Так начинается полная специализация шаблона для конкретного типа
 //Т.е. если тип предполагает особенную реализацию шаблона.
    template <>
    class lexical_cast<std::string>
    {
        std::string value;
    public:
        lexical_cast(const std::string& src) : value(src) {}

        operator const std::string& () const {
            return value;
        }

        operator std::string& () {
            return value;
        }
    };

    
    template<int index, typename Callback, typename... Args>
    struct iterate
    {
        static void next(std::tuple<Args...>& t, Callback callback, std::vector<std::string>::iterator& it)
        {
            // Уменьшаем позицию и рекурсивно вызываем этот же метод
            iterate<index - 1, Callback, Args...>::next(t, callback, it);
            // Вызываем обработчик и передаем ему позицию и значение элемента
            callback(index - 1, std::get<index>(t), it);
        }
    };
    // Частичная специализация для индекса 0 (завершает рекурсию)
    template<typename Callback, typename... Args>
    struct iterate<0, Callback, Args...>
    {
        static void next(std::tuple<Args...>& t, Callback callback, std::vector<std::string>::iterator& it)
        {
            callback(0, std::get<0>(t), it);
        }
    };
    // Частичная специализация для индекса -1 (пустой кортеж)
    template<typename Callback, typename... Args>
    struct iterate<-1, Callback, Args...>
    {
        static void next(std::tuple<Args...>& t, Callback callback, std::vector<std::string>::iterator& it) {}
    };

    // for_each для обхода элементов кортежа (compile time!):
    template<typename Callback, typename... Args>
    void forEach(std::tuple<Args...>& t, Callback callback, std::vector<std::string>::iterator& it)
    {
        // Размер кортежа
        int const t_size = std::tuple_size<std::tuple<Args...>>::value;
        // Запускаем рекурсивный обход элементов кортежа во время компиляции
        iterate<t_size - 1, Callback, Args...>::next(t, callback, it);
    }

    struct callback
    {
        template<typename T>
        void operator()(int index, T& t, std::vector<std::string>::iterator& it) // t - значение элемента
        {
            t = lexical_cast<T>(*it);
            ++it;
        }
    };
    //запуск обхода
    template <typename ...Args>
    void parse(std::tuple<Args...>& tuple, std::vector<std::string>::iterator& it)
    {
        forEach(tuple, callback(), it);
    }
}


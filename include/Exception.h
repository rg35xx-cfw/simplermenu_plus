#include <stdexcept>


class SimplerMenuException : public std::runtime_error {
public:
    SimplerMenuException(const std::string& what_arg) 
        : std::runtime_error(what_arg) { }
};


class IniValueNotFoundException : public SimplerMenuException {
public:
    IniValueNotFoundException(const std::string& what_arg) 
        : SimplerMenuException(what_arg) { }
};

class ItemIdNotFoundException : public SimplerMenuException {
public:
    ItemIdNotFoundException(const std::string& what_arg) 
        : SimplerMenuException(what_arg) { }
};

class ValueConversionException : public SimplerMenuException {
public:
    ValueConversionException(const std::string& what_arg) 
        : SimplerMenuException(what_arg) { }
};

class OptionValueNotFoundException : public SimplerMenuException {
public:
    OptionValueNotFoundException(const std::string& what_arg) 
        : SimplerMenuException(what_arg) { }
};


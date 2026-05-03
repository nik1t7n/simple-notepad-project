#pragma once

#include <exception>
#include <string>

class notepad_exception : public std::exception {
public:
    explicit notepad_exception(const std::string& message)
        : m_message(message)
    {
    }

    const char* what() const noexcept override
    {
        return m_message.c_str();
    }

private:
    std::string m_message;
};

class file_not_found_exception : public notepad_exception {
public:
    explicit file_not_found_exception(const std::string& message)
        : notepad_exception(message)
    {
    }
};

class file_read_exception : public notepad_exception {
public:
    explicit file_read_exception(const std::string& message)
        : notepad_exception(message)
    {
    }
};

class file_write_exception : public notepad_exception {
public:
    explicit file_write_exception(const std::string& message)
        : notepad_exception(message)
    {
    }
};

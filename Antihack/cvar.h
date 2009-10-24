#ifndef INC_CVAR
#define INC_CVAR

#include <windows.h>
#include <iostream>
#include <string>
#include <sstream>

class CCvarBase {
	public:
		CCvarBase() {}
		virtual std::string ToString() = 0;

};

template <typename T>
class CCvar : public CCvarBase {
	public:
		explicit CCvar(std::string _name, T _value) : name(_name), copy(_value) { pValue = &copy; }
		explicit CCvar(std::string _name, T* _pValue) : name(_name), pValue(_pValue) {}
		std::string ToString();

	private:
		CCvar() {}

	public:
		std::string name;
		T* pValue;
		T copy;
};

template <typename T>
std::string CCvar<T>::ToString()
{
	std::string s;
	std::stringstream ss;

	if (pValue) {
		ss << *pValue;
		s = ss.str();
	}

	s = name + ": " + s;

	return s;
}

#endif
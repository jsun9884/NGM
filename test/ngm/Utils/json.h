#pragma once

#include <string>
#include <sstream>

namespace json {

class base {
protected:
	bool m_bFirst;
	std::ostringstream &m_out;

public:
	base(base &owner): m_bFirst(true), m_out(owner.m_out) {
		if (!owner.m_bFirst) {
			m_out << ",";
		}
		else {
			owner.m_bFirst = false;
		}
	}
	base(std::ostringstream &out): m_bFirst(true), m_out(out) {}
};

class object: public base {
private:
	void construct(std::string name) {
		if (name.length()) {
			m_out << "\"" << name << "\":";
		}
		m_out << "{";
	}

public:
	object(base& owner, std::string name = ""):
		base(owner)
	{
		construct(name);
	}

	object(std::ostringstream& out, std::string name = ""):
		base(out)
	{
		construct(name);
	}

	~object() {
		m_out << "}";
	}

	template<class T>
	void insert(std::string name, const T &value) {
		if (!m_bFirst) {
			m_out << ",";
		}
		else {
			m_bFirst = false;
		}

		m_out << "\"" << name << "\":\"" << value << "\"";
	}
};

class array: public base {
private:
	void construct(std::string name) {
		if (name.length()) {
			m_out << "\"" << name << "\":";
		}
		m_out << "[";
	}

public:
	array(base& owner, std::string name = ""):
		base(owner)
	{
		construct(name);
	}

	array(std::ostringstream& out, std::string name = ""):
		base(out)
	{
		construct(name);
	}

	~array() {
		m_out << "]";
	}
};

} // namespace json

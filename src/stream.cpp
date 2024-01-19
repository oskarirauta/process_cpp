#include "process.hpp"

process_t::STREAM::~STREAM() {

	if ( this -> in != nullptr ) delete this -> in;
	if ( this -> out != nullptr ) delete this -> out;
	if ( this -> err != nullptr ) delete this -> err;
}

static std::string trim_end(std::stringstream& ss) {

	std::string s = ss.str();
	while ( !s.empty() && std::string(" \t\n\r\f\v").find_first_of(s.back()) != std::string::npos )
		s.pop_back();
	return s;
}

std::string process_t::STREAM::str_out() {

	if ( this -> out == nullptr )
		return "";

	std::stringstream ss;
	ss << this -> out -> rdbuf();
	return trim_end(ss);
}

std::string process_t::STREAM::str_err() {

	if ( this -> err == nullptr )
		return "";

	std::stringstream ss;
	ss << this -> err -> rdbuf();
	return trim_end(ss);
}

#include "process.hpp"

process_t::OUTPUT::operator int() {

	if ( this -> parent == nullptr )
		return -1;

	return this -> parent -> status();
}

process_t::OUTPUT::operator std::string() {

	if ( this -> parent == nullptr || this -> parent -> stream == nullptr )
		return "";

	switch ( this -> type ) {
		case STREAM_OUT: return this -> parent -> stream -> str_out();
		case STREAM_ERR: return this -> parent -> stream -> str_err();
		case STREAM_STATUS: return std::to_string(this -> parent -> status());
		default: return "";
	}
}

process_t::OUTPUT::~OUTPUT() {
	this -> parent = nullptr;
}

std::ostream& operator <<(std::ostream& os, process_t::OUTPUT output) {

	if ( output.type == STREAM_STATUS )
		os << output.operator int();
	else os << output.operator std::string();

	return os;
}

#pragma once

#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
	
enum TokenType {
	stock,
	quantity,
	operation,
	needed_stock,
	result_stock,
	delay,
	optimize
};

struct Token {
	TokenType type;
	std::string info;

	Token(TokenType type, std::string info) : type(type), info(info) {};
};
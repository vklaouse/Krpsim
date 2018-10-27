#pragma once

#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>

#define TIME_KEYWORD "time"
#define OPTIMIZE_KEYWORD "optimize"

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

#pragma once

#include <algorithm>
#include <string>

bool case_insensitive_equal(const std::string& s1,const std::string& s2){
	if (s1.length() != s2.length()) return false;
	for(size_t i = 0; i < s1.length(); i++){
		if (toupper(s1[i]) != toupper(s2[i])) return false;
	}
	return true;
}

std::string tolower(const std::string& s){
	std::string lower{s};
	std::transform(lower.begin(), lower.end(), lower.begin(), [](unsigned char c){ return std::tolower(c); });
	return lower;
}

void trim_return_carriage(std::string& s){
	if (s[s.length() - 1] == '\r'){
		s.resize(s.length() - 1);
	}
}
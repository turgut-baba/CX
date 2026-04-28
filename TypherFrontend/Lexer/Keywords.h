#ifndef KEYWORDS_H
#define KEYWORDS_H

#include <cstdint>
#include <unordered_set>
#include <string_view>
#include "Lexer.h"

namespace Lex {

	class Lexer;

	static const std::unordered_set<std::string_view> keywords = { // TODO: Pottentially unused
		"if", "else", "for", "while", "return", "enum"
	};

	class Keywords {
	public:
		Keywords(Lexer* lexer, char32_t start_letter) : lexer_(lexer), start_letter_(start_letter) { }
		void PottentialKeyword();
		void KeyIdentifier();
		void ScanNumber(bool isFloating = false);
		void ScanStringLiteral();

	private:
		std::string GetWord();
		Lexer* lexer_;
		char32_t start_letter_;
		bool isModifier = false;
	};
}

#endif
#include "Keywords.h"
#include "Letters.h"
#include <charconv>

namespace Lex {
	#define FNV_OFFSET_BASIS_32 2166136261u;
	#define FNV_PRIME_32 16777619u;

	constexpr uint32_t hash(std::string_view text) {
		uint32_t hash_value = FNV_OFFSET_BASIS_32;

		for (char ch : text) {
			hash_value ^= static_cast<uint8_t>(ch);
			hash_value *= FNV_PRIME_32;
		}

		return hash_value;
	}

	std::string Keywords::GetWord() {
		std::string final_word(1, static_cast<char>(start_letter_));
		auto token = lexer_->Peek();

		while (LexicalChar::UPPERCASE_A < token && token < LexicalChar::LOWERCASE_Z) {
			std::string letter(1, static_cast<char>(token));
			final_word += letter;
			lexer_->IterForward();
			token = lexer_->Peek();
		}

		return final_word;
	}

	void Keywords::PottentialKeyword()
	{
		std::string word = GetWord();
		Token& token = lexer_->GetToken();
		token.SetIdent(word);
		lexer_->GetToken().SetType(TokenType::Keyword);
		switch (hash(word)) {
		case hash("if"):
			token.SetTokenType<TokenKeyword>(TokenKeyword::IF);
			break;
		case hash("enum"):
			token.SetTokenType<TokenKeyword>(TokenKeyword::ENUM);
			break;
		case hash("for"):
			token.SetTokenType<TokenKeyword>(TokenKeyword::FOR);
			break;
		case hash("while"):
			token.SetTokenType<TokenKeyword>(TokenKeyword::WHILE);
			break;
		case hash("int"):
			token.SetTokenType<TokenKeyword>(TokenKeyword::INT);
			break;
		case hash("return"):
			token.SetTokenType<TokenKeyword>(TokenKeyword::RETURN);
			break;
		default:
			token.SetType(TokenType::Identifier);
		}
	}

	void Keywords::KeyIdentifier() {
		Token& token = lexer_->GetToken();
		token.SetType(TokenType::Identifier);
		token.SetIdent(GetWord());
	}

	void Keywords::ScanStringLiteral() {

	};

	void Keywords::ScanNumber() {
		lexer_->GetToken().SetType(TokenType::Literal);
		// TODO: Handle decimal and other numeric literals
		std::string number_str(1, static_cast<char>(start_letter_));
		auto token = lexer_->Peek();

		while (LexicalChar::NUM_0 < token && token < LexicalChar::NUM_1) {
			std::string letter(1, static_cast<char>(token));
			number_str += letter;
			lexer_->IterForward();
			token = lexer_->Peek();
		}

		int value;
		auto [ptr, ec] = std::from_chars(number_str.data(), number_str.data() + number_str.size(), value);

		if (ec == std::errc{}) {
			lexer_->GetToken().numeric_value = value;
		}

		// TODO: Do error handling
	}
}
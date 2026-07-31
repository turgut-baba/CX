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

		while (
			(LexicalChar::UPPERCASE_A <= token && token <= LexicalChar::UPPERCASE_Z) ||
			(LexicalChar::LOWERCASE_A <= token && token <= LexicalChar::LOWERCASE_Z) ||
			(LexicalChar::NUM_0 <= token && token <= LexicalChar::NUM_9) ||
			(token == LexicalChar::UNDERSCORE)
		) {
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


		case hash("void"):
			token.SetTokenType<TokenKeyword>(TokenKeyword::VOID);
			break;
		case hash("bool"):
			token.SetTokenType<TokenKeyword>(TokenKeyword::BOOL);
			break;
		case hash("char"):
			token.SetTokenType<TokenKeyword>(TokenKeyword::CHAR);
			break;
		case hash("int"):
			token.SetTokenType<TokenKeyword>(TokenKeyword::INT);
			break;
		case hash("float"):
			token.SetTokenType<TokenKeyword>(TokenKeyword::FLOAT);
			break;	
		case hash("double"):
			token.SetTokenType<TokenKeyword>(TokenKeyword::DOUBLE);
			break;


		case hash("return"):
			token.SetTokenType<TokenKeyword>(TokenKeyword::RETURN);
			break;
		case hash("else"):
			token.SetTokenType<TokenKeyword>(TokenKeyword::ELSE);
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
		lexer_->GetToken().SetType(TokenType::Literal);
		auto token = lexer_->Peek();             

		std::string value;
		while (LexicalChar::DOUBLE_QUOTE != token && 
			   LexicalChar::SINGLE_QUOTE != token) {
			std::string letter(1, static_cast<char>(token));
			value += letter;
			lexer_->IterForward();
			token = lexer_->Peek();
		}
		lexer_->GetToken().SetIdent(value);
		
		// TODO: single quote values and string values should be set differently.
		lexer_->GetToken().decimal = (int)value.c_str()[0];

		// TODO: Make sure the last char is a double quote
		lexer_->IterForward(); // Skip over ''' or '"'
	};

	void Keywords::ScanNumber(bool isFloating) {
		lexer_->GetToken().SetType(TokenType::Literal);
		// TODO: Handle other numeric literals

		std::string number_str(1, static_cast<char>(start_letter_));
		auto token = lexer_->Peek();
		while (LexicalChar::NUM_0 <= token && token <= LexicalChar::NUM_9) {
			std::string letter(1, static_cast<char>(token));
			number_str += letter;
			lexer_->IterForward();
			token = lexer_->Peek();
		}
		
		int value;
		auto [ptr, ec] = std::from_chars(number_str.data(), number_str.data() + number_str.size(), value);
		

		if (ec != std::errc{}) {
			// TODO: Error handling
		}

		if(!isFloating) {
			lexer_->GetToken().decimal = value;
			lexer_->GetToken().isFloating = false;
			lexer_->GetToken().SetIdent(number_str);
		} else {
			double float_value;
			std::string float_str = lexer_->GetToken().Ident() + "." + number_str;

			auto [ptr, ec] = std::from_chars(float_str.data(), float_str.data() + float_str.size(), float_value);

			lexer_->GetToken().floating = float_value;
			lexer_->GetToken().isFloating = true;
			lexer_->GetToken().SetIdent(float_str);
		}

		if(token == LexicalChar::DOT) {
			if(isFloating) {
				// TODO: Error handling
			}

			lexer_->IterForward(); // Finish the decimal
			token = lexer_->Peek();
			lexer_->IterForward(); // Skip over '.'

			if(!(LexicalChar::NUM_0 <= token && token <= LexicalChar::NUM_9)) {
				// TODO: Error handling
			}

			start_letter_ = (char32_t)token;
			ScanNumber(true);
		}
	}
}
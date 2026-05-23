#include "Lexer.h"
#include "Letters.h"
#include "filesystem"

namespace Lex {
	Lexer::Lexer(std::string &file_buffer) {
		this->file_buffer = std::move(file_buffer);
		this->iterator_ = this->file_buffer.begin();
		//this->keyw_parser = std::make_unique<Keywords>(this);
	}

	void Lexer::SkipWhiteSpaces() {
		// TODO: CHANGE THİS AFTER THE PRE_PROCESSOR.
		while (true) {
			LexicalChar cp = Peek();
			switch (cp) {
			case LexicalChar::CARRIGE_RETURN:
				IterForward();
				if (Peek() != LexicalChar::LINE_FEED) {
					IterBack();
				}
			case LexicalChar::LINE_FEED:
			case LexicalChar::NEXT_LINE:
				line_ += 1; column_ = 0;
				[[fallthrough]];
			case LexicalChar::VT:
			case LexicalChar::FORM_FEED:
			
			case LexicalChar::NARROW_NO_BREAK_SP:
			case LexicalChar::IDEOGRAPHIC_SP:
			case LexicalChar::ZERO_WIDTH_SP:
			
			case LexicalChar::SPACE:
				IterForward();
				continue;
			case LexicalChar::TAB:
				column_ += 2;
				IterForward();
				continue;
			default:
				return;
			}
		}
	}

	void Lexer::ScanForEquality(std::string ident, TokenOperator single, TokenOperator equal)
	{
		LexicalChar cp = Peek();
		IterForward();
		current_token.SetType(TokenType::Operator);
		std::string eq_ident = "=";
		switch (cp)
		{
		case LexicalChar::EQUALS:
			current_token.SetTokenType<TokenOperator>(equal);
			current_token.SetIdent(ident + eq_ident); // TEMP
			break;
		default:
			current_token.SetTokenType<TokenOperator>(single);
			current_token.SetIdent(ident); // TEMP
			IterBack();
			break;
		}
	}

	void Lexer::NextToken()
	{
		LexicalChar cp = Peek();
		IterForward();

		Keywords keyword_parser(this, (char32_t)cp);
		//std::cout << "Token: " << static_cast<char>(cp) << std::endl;

		current_token.SetType(TokenType::EOS); // TODO: add this as a case.
		current_token.RestTokenTypes();
		current_token.loc_ = Location(file_name, line_, column_);

		switch (cp) {
		case LexicalChar::LEFT_PAREN:
			current_token.SetIdent("("); // TEMP
			current_token.SetTokenType<TokenPunctuator>(TokenPunctuator::LEFT_PARENTHESES);
			break;
		case LexicalChar::RIGHT_PAREN:
			current_token.SetIdent(")"); // TEMP
			current_token.SetTokenType<TokenPunctuator>(TokenPunctuator::RIGHT_PARENTHESES);
			break;
		case LexicalChar::AT:
			current_token.SetTokenType<TokenPunctuator>(TokenPunctuator::AT);
			
			break;
		case LexicalChar::SEMICOLON:
			current_token.SetIdent(";"); // TEMP
			current_token.SetTokenType<TokenPunctuator>(TokenPunctuator::SEMICOLON);
			break;
		case LexicalChar::LEFT_CURLY_BRACE:
			current_token.SetIdent("{"); // TEMP
			current_token.SetTokenType<TokenPunctuator>(
								TokenPunctuator::LEFT_CURLY_BRACE);
			break;
		case LexicalChar::RIGHT_CURLY_BRACE:
			current_token.SetIdent("}"); // TEMP
			current_token.SetTokenType<TokenPunctuator>(
								TokenPunctuator::RIGHT_CURLY_BRACE);
			break;
		case LexicalChar::EQUALS:
			ScanForEquality(
				"=", TokenOperator::ASSIGNMENT, TokenOperator::EQUALS);
			break;
		case LexicalChar::EXCLAMATION:
			ScanForEquality(
				"!", TokenOperator::LOGICAL_NOT, TokenOperator::NOT_EQUALS);
			break;
		case LexicalChar::GREATER_THAN:
			ScanForEquality(
				">", TokenOperator::GREATER, TokenOperator::GREAT_OR_EQUAL);
			break;
		case LexicalChar::LESS_THAN:
			ScanForEquality(
				"<", TokenOperator::LESSER, TokenOperator::LESS_OR_EQUAL);
			break;
		case LexicalChar::COMMA:
			current_token.SetIdent(","); // TEMP
			current_token.SetType(TokenType::Punctuator);
			break;
		case LexicalChar::PLUS:
			current_token.SetIdent("+"); // TEMP
			current_token.SetTokenType<TokenOperator>(
								TokenOperator::ADD);
			break;
		case LexicalChar::MINUS:
			current_token.SetIdent("-"); // TEMP
			current_token.SetTokenType<TokenOperator>(
								TokenOperator::SUBTRACT);
			break;
		case LexicalChar::ASTERISK:
			current_token.SetIdent("*"); // TEMP
			current_token.SetTokenType<TokenOperator>(
								TokenOperator::MULTIPLY);
			break;
		case LexicalChar::SLASH:
			current_token.SetIdent("/"); // TEMP
			current_token.SetTokenType<TokenOperator>(
								TokenOperator::DIVIDE);
			break;
		case LexicalChar::PERCENT:
			current_token.SetIdent("%"); // TEMP
			current_token.SetTokenType<TokenOperator>(
								TokenOperator::MODULUS);
			break;
		case LexicalChar::AMPERSAND:
			current_token.SetIdent("&"); // TEMP
			current_token.SetTokenType<TokenOperator>(
								TokenOperator::BITWISE_AND);
			break;
		case LexicalChar::DOUBLE_QUOTE:
			current_token.SetTokenType<TokenLiteral>(
				TokenLiteral::STRING);
			keyword_parser.ScanStringLiteral();
			break;
		case LexicalChar::SINGLE_QUOTE:
			current_token.SetTokenType<TokenLiteral>(
				TokenLiteral::CHARACTER);
			keyword_parser.ScanStringLiteral();
			// TODO: raise error on multiple chars.
			break;
		case LexicalChar::NUM_0:
		case LexicalChar::NUM_1:
		case LexicalChar::NUM_2:
		case LexicalChar::NUM_3:
		case LexicalChar::NUM_4:
		case LexicalChar::NUM_5:
		case LexicalChar::NUM_6:
		case LexicalChar::NUM_7:
		case LexicalChar::NUM_8:
		case LexicalChar::NUM_9: {

			keyword_parser.ScanNumber();

			if(current_token.isFloating) {
				current_token.SetTokenType<TokenLiteral>(
					TokenLiteral::FLOATING_PONINT);
			} else {
				current_token.SetTokenType<TokenLiteral>(
					TokenLiteral::DECIMAL);
			}
			break;
		}
		case LexicalChar::LOWERCASE_A:
		case LexicalChar::LOWERCASE_B:
		case LexicalChar::LOWERCASE_C:
		case LexicalChar::LOWERCASE_D:
		case LexicalChar::LOWERCASE_E:
		case LexicalChar::LOWERCASE_F:
		case LexicalChar::LOWERCASE_G:
		case LexicalChar::LOWERCASE_H:
		case LexicalChar::LOWERCASE_I:
		case LexicalChar::LOWERCASE_J:
		case LexicalChar::LOWERCASE_K:
		case LexicalChar::LOWERCASE_L:
		case LexicalChar::LOWERCASE_M:
		case LexicalChar::LOWERCASE_N:
		case LexicalChar::LOWERCASE_O:
		case LexicalChar::LOWERCASE_P:
		case LexicalChar::LOWERCASE_Q:
		case LexicalChar::LOWERCASE_R:
		case LexicalChar::LOWERCASE_S:
		case LexicalChar::LOWERCASE_T:
		case LexicalChar::LOWERCASE_U:
		case LexicalChar::LOWERCASE_V:
		case LexicalChar::LOWERCASE_W:
		case LexicalChar::LOWERCASE_X:
		case LexicalChar::LOWERCASE_Y:
		case LexicalChar::LOWERCASE_Z: {
			keyword_parser.PottentialKeyword();
			break;
		}
		case LexicalChar::UPPERCASE_A:
		case LexicalChar::UPPERCASE_B:
		case LexicalChar::UPPERCASE_C:
		case LexicalChar::UPPERCASE_D:
		case LexicalChar::UPPERCASE_E:
		case LexicalChar::UPPERCASE_F:
		case LexicalChar::UPPERCASE_G:
		case LexicalChar::UPPERCASE_H:
		case LexicalChar::UPPERCASE_I:
		case LexicalChar::UPPERCASE_J:
		case LexicalChar::UPPERCASE_K:
		case LexicalChar::UPPERCASE_L:
		case LexicalChar::UPPERCASE_M:
		case LexicalChar::UPPERCASE_N:
		case LexicalChar::UPPERCASE_O:
		case LexicalChar::UPPERCASE_P:
		case LexicalChar::UPPERCASE_Q:
		case LexicalChar::UPPERCASE_R:
		case LexicalChar::UPPERCASE_S:
		case LexicalChar::UPPERCASE_T:
		case LexicalChar::UPPERCASE_U:
		case LexicalChar::UPPERCASE_V:
		case LexicalChar::UPPERCASE_W:
		case LexicalChar::UPPERCASE_X:
		case LexicalChar::UPPERCASE_Y:
		case LexicalChar::UPPERCASE_Z: {
			keyword_parser.KeyIdentifier();
			break;
		}
		default:
			break;
		}

		SkipWhiteSpaces();
	}
}
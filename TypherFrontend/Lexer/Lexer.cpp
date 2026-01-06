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
				[[fallthrough]];
			case LexicalChar::VT:
			case LexicalChar::TAB:
			case LexicalChar::FORM_FEED:
			case LexicalChar::SPACE:

			case LexicalChar::NARROW_NO_BREAK_SP:
			case LexicalChar::NEXT_LINE:
			case LexicalChar::IDEOGRAPHIC_SP:
			case LexicalChar::ZERO_WIDTH_SP:

			case LexicalChar::LINE_FEED:
				IterForward();
				// Increment line for line feed
				continue;
			default:
				return;
			}
		}
	}

	void Lexer::ScanEqualSign()
	{
		IterForward();
		LexicalChar cp = Peek();
		current_token.SetType(TokenType::Operator);
		switch (cp)
		{
		case LexicalChar::GREATER_THAN:
			break;
		case LexicalChar::LESS_THAN:
			break;
		case LexicalChar::EQUALS:
			current_token.SetTokenType<TokenOperator>(TokenOperator::EQUALS);
			current_token.SetIdent("=="); // TEMP
			break;
		default:
			current_token.SetIdent("="); // TEMP
			current_token.SetTokenType<TokenOperator>(TokenOperator::ASSIGNMENT);
			IterBack();
			break;
		}
	}

	void Lexer::NextToken()
	{
		LexicalChar cp = Peek();
		IterForward();

		Keywords keyword_parser(this, (char32_t)cp);

		current_token.SetType(TokenType::EOS); // TODO: add this as a case.

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
			ScanEqualSign();
			break;
		case LexicalChar::COMMA:
			current_token.SetIdent(","); // TEMP
			current_token.SetType(TokenType::Punctuator);
			break;
		case LexicalChar::DOUBLE_QUOTE:
			keyword_parser.ScanStringLiteral();
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
			current_token.SetIdent(std::to_string(current_token.numeric_value)); // TEMP
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
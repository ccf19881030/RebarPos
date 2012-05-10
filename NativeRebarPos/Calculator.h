//-----------------------------------------------------------------------------
//----- Calculator.h : Declaration of the Calculator
//-----------------------------------------------------------------------------
#pragma once

#include <string>
#include <vector>
#include <stack>
#include <queue>
#include <math.h>

class Calculator
{
public:
	enum Errors
	{
		eParenthesisMisMatch = 1,
		eNotAnOperator = 2,
		eUnknownOperator = 3,
		eInvalidFormula = 4
	};

protected:
	struct Token
	{
	public:
		wchar_t Op;
		double Value;
		bool IsOp;
		bool IsLeftAssoc;
		int Rank;

	public:
		bool operator==(const Token &other) const 
		{
			if(!IsOp || !other.IsOp)
				throw eNotAnOperator;
			return Op == other.Op;
		}
		bool operator!=(const Token &other) const 
		{
			if(!IsOp || !other.IsOp)
				throw eNotAnOperator;
			return Op != other.Op;
		}
		bool operator<(const Token &other) const 
		{
			if(!IsOp || !other.IsOp)
				throw eNotAnOperator;
			return Rank < other.Rank;
		}
		bool operator>(const Token &other) const 
		{
			if(!IsOp || !other.IsOp)
				throw eNotAnOperator;
			return Rank > other.Rank;
		}
		bool operator<=(const Token &other) const 
		{
			if(!IsOp || !other.IsOp)
				throw eNotAnOperator;
			return Rank <= other.Rank;
		}
		bool operator>=(const Token &other) const 
		{
			if(!IsOp || !other.IsOp)
				throw eNotAnOperator;
			return Rank >= other.Rank;
		}

	public:
		const double Eval(const double a, const double b) const
		{
			if(!IsOp)
				throw eNotAnOperator;

			switch(Op)
			{
				case L'+': return a + b; break;
				case L'-': return a - b; break;
				case L'*': return a * b; break;
				case L'/': return a / b; break;
				case L'^': return pow(a, b); break;
				default:
					throw eUnknownOperator;
			}
		}

	public:
		Token(const wchar_t op)
		{
			IsOp = true;
			Op = op;
			IsLeftAssoc = (op == L'+' || op == L'-' || op == L'*' || op == L'/');
			Rank = ((op == L'^') ? 2 : ((op == L'*' || op == L'/') ? 1 : 0));

			Value = 0.0;
		}
		Token(const double value)
		{
			IsOp = false;
			Op = L'\0';
			IsLeftAssoc = false;
			Rank = 0;

			Value = value;
		}
		Token(const std::wstring value)
		{
			IsOp = false;
			Op = L'\0';
			IsLeftAssoc = false;
			Rank = 0;

			wchar_t* endstr;
			Value = wcstod(value.c_str(), &endstr);
		}
	};

private:
	Calculator() { }
	Calculator(Calculator const&) { }
	void operator=(Calculator const&) { }

protected:
	static bool IsOperator(wchar_t c);
	static bool IsNumeric(wchar_t c);

	static std::queue<Token> Tokenize(std::wstring str);
	static std::queue<Token> InfixToRPN(std::queue<Calculator::Token> tokens);

public:
	static double Evaluate(std::wstring formula);
	static bool IsValid(std::wstring formula);
};
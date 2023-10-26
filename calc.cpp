#include <cstdint>
#include <cctype>
#include <iostream>
#include <string>
#include <map>
#include <functional>
#include <vector>
#include <stack>
#include <exception>
#include <memory>
#include <utility>
#include <stdexcept>

using std::cout;
using std::cin;
using std::endl;
using std::string;
using std::stack;
using std::exception;
using std::vector;
using std::ostream;
using std::unique_ptr;
using std::map;
using std::pair;
using std::make_pair;
using std::domain_error;


//////////////////////////////////////////////////////////////////////
// UnCalc物件需要以一個運算式字串初始化
// 呼叫eval()方法進行計算

class UnCalc
{
public:
	//計算所使用的資料型別
	//雖然運算式中的數字有另外的大小限制，
	//但計算途中的數值範圍則是由ArithmeticType決定
	using ArithmeticType = int32_t;
	
	//接受運算式字串，並立即檢查語法
	//有語法錯誤會丟出syntax_error例外
	// 運算式字串格式：
	// * 最大99999999的整數
	// * 整數四則運算(+ - * /)與取模(%)，先乘除取模後加減
	// * 可使用括號改變運算子的優先度
	// * 一元負號(-)反轉一個數字的正負號
	explicit UnCalc(const string& text_expression);
	
	//計算結果
	//如果途中 /0 或 %0，會丟出domain_error例外
	ArithmeticType eval();
	
	//語法錯誤例外。呼叫what()取得進一步資訊。
	class syntax_error : public exception
	{
	public:
		syntax_error(const char *what_arg) : s(what_arg) {  }
		const char *what() const noexcept override { return s; }
	private:
		const char *s;
	};
	
	//印出UnCalc目前的內部結構：後序形式的運算式
	//一元負號(-)會被轉換成'#'
	friend ostream& operator<<(ostream&, const UnCalc&);
	
	
private:
	static constexpr char OP_UNARY_MINUS = '#';
	static constexpr ArithmeticType NUMBER_MAX = 99999999;
	
	//運算式抽象單元
	class ExpUnit
	{
	public:
		//回傳一個實際物件的副本
		virtual unique_ptr<ExpUnit> clone() const = 0;
		
		//傳入一個作為accumulator用途的堆疊
		//eval()會拿堆疊上的數值做運算，並將結果放回堆疊
		virtual void eval(stack<ArithmeticType>& accumulator) const = 0;
		
		//印出運算式單元
		virtual void print(ostream&) const = 0;
	};
	
	class Operand;	//運算元
	
	//運算子抽象類別
	class Oprtr : public ExpUnit
	{
	public:
		const uint_fast8_t priority;	//運算子優先度
		const ssize_t operand_number;	//需要的運算元數量
	protected:
		Oprtr(uint_fast8_t pri, ssize_t opd_n) : priority(pri), operand_number(opd_n) {}
	};
	
	class BinaryOprtr;	//二元運算子
	class UnaryOprtr;	//一元運算子
	class AdditionOp;	//加法
	class SubstractionOp;	//減法
	class MultiplicationOp;	//乘法
	class DivisionOp;	//除法
	class ModulusOp;	//取模
	class ExponentOp;	//指數運算
	class MinusOp;	//一元負號
	
	//運算式容器
	//採用智慧指標管理每個元素
	vector<unique_ptr<ExpUnit>> exp;
	
	class Parser;	//語法剖析器
	
	//運算子清單
	//效能上來說使用map並不好，但這支程式是在練習STL
	//否則用列舉為各種運算子定義一個值，對應到表格中的唯一索引效能會更好
	static const map<char, const Oprtr*> operator_table;
};


//////////////////////////////////////////////////////////////////////


//運算子抽象類別
class UnCalc::Operand : public ExpUnit
{
public:
	//傳入一個只有數字的字串，轉成整數
	Operand(const string& s) : val(0)
	{
		for (auto d : s)
		{
			if (val > NUMBER_MAX) throw syntax_error("operand too big");
			
			val = val * 10 + (d - '0');
		}
		
		if (val > NUMBER_MAX) throw syntax_error("operand too big");
	}
	
	unique_ptr<ExpUnit> clone() const override
	{
		return unique_ptr<ExpUnit>(new Operand(*this));
	}
	
	void eval(stack<ArithmeticType>& acm) const override
	{
		acm.push(val);
	}
	
	void print(ostream& os) const override
	{
		os << val;
	}
	
private:
	ArithmeticType val;
};

//二元運算子
class UnCalc::BinaryOprtr : public Oprtr
{
protected:
	BinaryOprtr(uint_fast8_t pri) : Oprtr(pri, 2) {}
	
	inline auto get_operand(stack<ArithmeticType>& st) const
	{
		auto rhs = st.top();
		st.pop();
		auto lhs = st.top();
		st.pop();
		
		return make_pair(lhs, rhs);
	}
};

//一元運算子
class UnCalc::UnaryOprtr : public Oprtr
{
protected:
	UnaryOprtr(uint_fast8_t pri) : Oprtr(pri, 1) {}
	
	inline ArithmeticType get_operand(stack<ArithmeticType>& st) const
	{
		auto rhs = st.top();
		st.pop();
		
		return rhs;
	}
};

//加法
class UnCalc::AdditionOp : public BinaryOprtr
{
public:
	AdditionOp() : BinaryOprtr(1) {}
	
	unique_ptr<ExpUnit> clone() const override
	{
		return unique_ptr<ExpUnit>(new AdditionOp(*this));
	}
	
	void eval(stack<ArithmeticType>& acm) const override
	{
		auto operand = get_operand(acm);
		acm.push(operand.first + operand.second);
	}
	
	void print(ostream& os) const override
	{
		os << '+';
	}
};

//減法
class UnCalc::SubstractionOp : public BinaryOprtr
{
public:
	SubstractionOp() : BinaryOprtr(1) {}
	
	unique_ptr<ExpUnit> clone() const override
	{
		return unique_ptr<ExpUnit>(new SubstractionOp(*this));
	}
	
	void eval(stack<ArithmeticType>& acm) const override
	{
		auto operand = get_operand(acm);
		acm.push(operand.first - operand.second);
	}
	
	void print(ostream& os) const override
	{
		os << '-';
	}
};

//乘法
class UnCalc::MultiplicationOp : public BinaryOprtr
{
public:
	MultiplicationOp() : BinaryOprtr(2) {}
	
	unique_ptr<ExpUnit> clone() const override
	{
		return unique_ptr<ExpUnit>(new MultiplicationOp(*this));
	}
	
	void eval(stack<ArithmeticType>& acm) const override
	{
		auto operand = get_operand(acm);
		acm.push(operand.first * operand.second);
	}
	
	void print(ostream& os) const override
	{
		os << '*';
	}
};

//除法
class UnCalc::DivisionOp : public BinaryOprtr
{
public:
	DivisionOp() : BinaryOprtr(2) {}
	
	unique_ptr<ExpUnit> clone() const override
	{
		return unique_ptr<ExpUnit>(new DivisionOp(*this));
	}
	
	void eval(stack<ArithmeticType>& acm) const override
	{
		auto operand = get_operand(acm);
		
		if (operand.second == 0)
		{
			throw domain_error("divided by zero in #UnCalc");
		}
		
		acm.push(operand.first / operand.second);
	}
	
	void print(ostream& os) const override
	{
		os << '/';
	}
};

//取模
class UnCalc::ModulusOp : public BinaryOprtr
{
public:
	ModulusOp() : BinaryOprtr(2) {}
	
	unique_ptr<ExpUnit> clone() const override
	{
		return unique_ptr<ExpUnit>(new ModulusOp(*this));
	}
	
	void eval(stack<ArithmeticType>& acm) const override
	{
		auto operand = get_operand(acm);
		
		if (operand.second == 0)
		{
			throw domain_error("modulo by zero in #UnCalc");
		}
		
		acm.push(operand.first % operand.second);
	}
	
	void print(ostream& os) const override
	{
		os << '%';
	}
};

//指數
class UnCalc::ExponentOp : public BinaryOprtr
{
public:
	ExponentOp() : BinaryOprtr(3) {}
	
	unique_ptr<ExpUnit> clone() const override
	{
		return unique_ptr<ExpUnit>(new ExponentOp(*this));
	}
	
	void eval(stack<ArithmeticType>& acm) const override
	{
		auto operand = get_operand(acm);
		
		ArithmeticType res = (operand.second >= 0)? 1 : 0;
		
		for (; operand.second > 0; operand.second--)
		{
			res *= operand.first;
		}
		
		acm.push(res);
	}
	
	void print(ostream& os) const override
	{
		os << '^';
	}
};

//一元負號
class UnCalc::MinusOp : public UnaryOprtr
{
public:
	MinusOp() : UnaryOprtr(4) {}
	
	unique_ptr<ExpUnit> clone() const override
	{
		return unique_ptr<ExpUnit>(new MinusOp(*this));
	}
	
	void eval(stack<ArithmeticType>& acm) const override
	{
		auto operand = get_operand(acm);
		acm.push(-operand);
	}
	
	void print(ostream& os) const override
	{
		os << OP_UNARY_MINUS;
	}
};


//////////////////////////////////////////////////////////////////////


//建構時指定接收運算式單元的容器
//對表達式字串每個字元呼叫feed()
//結束時再呼叫finish()

class UnCalc::Parser
{
public:
	Parser(vector<unique_ptr<ExpUnit>>& expression_container)
		: exp(expression_container)
	{
		operand_number = 0;
		after_operand = false;
	}
	
	void feed(char c);
	void finish();
	
private:
	//嘗試產生運算元單元
	//回傳是否真的有生成運算元
	bool produce_operand_unit();
	//產生運算子單元
	void produce_operator_unit(char oprtr);
	//左括號處理
	void left_parentheses();
	//右括號處理
	//在finish()中會呼叫一次finale=true，相當於處理一個虛擬的右括號
	void right_parentheses(bool finale =false);
	
	//token的元素有兩種型態：
	// 1. 一般的運算子，pair的另一個欄位dont care
	// 2. 左括號'('，pair的另一個欄位保存遇到左括號當下的運算元數量
	stack<pair<char, ssize_t>> token;
	
	string num_buf;
	
	vector<unique_ptr<ExpUnit>>& exp;
	ssize_t operand_number;	//目前的運算元數量
	bool after_operand;	//目前位在一個運算元之後。數字或右括號都是運算元
};


bool UnCalc::Parser::produce_operand_unit()
{
	if (num_buf.length() == 0) return false;
	
	if (after_operand)
	{
		throw syntax_error("Missing operator");
	}
	
	exp.push_back(unique_ptr<ExpUnit>(new Operand(num_buf)));
	num_buf.clear();
	operand_number++;
	
	return true;
}


void UnCalc::Parser::produce_operator_unit(char oprtr)
{
	operand_number -= (operator_table.at(oprtr)->operand_number - 1);
	
	if (operand_number <= 0)
	{
		throw syntax_error("Missing operand");
	}
	
	exp.push_back(operator_table.at(oprtr)->clone());
}


void UnCalc::Parser::left_parentheses()
{
	if (after_operand)
	{
		throw syntax_error("Missing operator before '('");
	}
	
	//備份上一層的運算元數量
	token.push(make_pair('(', operand_number));
	operand_number = 0;
}


void UnCalc::Parser::right_parentheses(bool finale)
{
	while (!token.empty())
	{
		auto c = token.top();
		token.pop();
		
		if (c.first == '(')
		{
			if (finale)
			{
				//還有剩左括號就是少了右括號
				throw syntax_error("Missing ')'");
			}
			
			if (operand_number < 1) throw syntax_error("Missing operand before ')'");
			
			//還原上一層的運算元數量，並且再+1
			operand_number = c.second + 1;
			return;
		}
		
		produce_operator_unit(c.first);
	}
	
	//意外清空堆疊
	//但在最後的虛擬右括號反而要清空堆疊
	if (!finale)
	{
		throw syntax_error("Missing '('");
	}
}


void UnCalc::Parser::feed(char c)
{
	if (isdigit(c))
	{
		num_buf += c;
	}
	else
	{
		//在數字中斷時才產生運算元
		if (produce_operand_unit())
		{
			after_operand = true;
		}
		
		if (isblank(c))
		{
			//ignore blank character
		}
		else if (c == ')')
		{
			right_parentheses();
			after_operand = true;
		}
		else if (c == '(')
		{
			left_parentheses();
			after_operand = false;
		}
		else 
		{
			//判斷是一元負號或二元減法運算子
			if (c == '-' && !after_operand)
			{
				c = OP_UNARY_MINUS;
			}
			
			auto new_op = operator_table.find(c);
			
			if (new_op == operator_table.end())
			{
				throw syntax_error("Unknown operator");
			}
			else
			{
				after_operand = false;
				
				while (!token.empty())
				{
					auto old_op = token.top().first;
					
					//持續輸出堆疊上的運算子，直到：
					// 1. 碰到左括號
					// 2. 碰到優先度較低的運算子
					
					if (old_op == '(' || operator_table.at(old_op)->priority < new_op->second->priority)
					{
						break;
					}
					else
					{
						token.pop();
						produce_operator_unit(old_op);
					}
				}
				
				token.push(make_pair(new_op->first, 0));
			}
		}
	}
}


void UnCalc::Parser::finish()
{
	produce_operand_unit();
	right_parentheses(true);
	
	if (exp.empty())
	{
		throw syntax_error("empty expression");
	}
}


//////////////////////////////////////////////////////////////////////


UnCalc::UnCalc(const string& text_expression)
{
	Parser parser(exp);
	
	for (auto it = text_expression.begin(); it != text_expression.end(); ++it)
	{
		parser.feed(*it);
	}
	
	parser.finish();
}


auto UnCalc::eval() -> ArithmeticType
{
	stack<ArithmeticType> accumulator;
	
	for (auto &unit : exp)
	{
		unit->eval(accumulator);
	}
	
	return accumulator.top();
}


ostream &operator<<(ostream &os, const UnCalc &calc)
{
	bool padding = false;
	
	for (auto &s : calc.exp)
	{
		if (padding) os << ' ';
		else padding = !padding;
		s->print(os);
	}
	
	return os;
}


const map<char, const UnCalc::Oprtr*> UnCalc::operator_table =
{
	{'+', new UnCalc::AdditionOp},
	{'-', new UnCalc::SubstractionOp},
	{'*', new UnCalc::MultiplicationOp},
	{'/', new UnCalc::DivisionOp},
	{'%', new UnCalc::ModulusOp},
	{'^', new UnCalc::ExponentOp},
	{OP_UNARY_MINUS, new UnCalc::MinusOp},
};


//////////////////////////////////////////////////////////////////////


int main()
{
	string line;
	
	while (1)
	{
		cout << "> " << std::flush;
		
		if (!getline(cin, line)) break;
		
		try
		{
			UnCalc calc(line);
			//cout << "debug: " << calc << endl;
			cout << calc.eval() << endl;
		}
		catch(const UnCalc::syntax_error& e)
		{
			cout << "UnCalc: " << e.what() << endl;
		}
		catch(const domain_error& e)
		{
			cout << e.what() << endl;
		}
		catch(...)
		{
			throw;
		}
		
		cout << endl;
	}
}

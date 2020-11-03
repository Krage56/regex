#include "iostream"
#include "fstream"
#include "regex"
#include "map"
#include "list"
#include <queue>
#include <algorithm>
#include <stack>
using namespace std;
enum class Token {
	Number,
	String,
	ColumnName
};

#if 0
#define NODISCARD [[nodiscard]]
#else
#define NODISCARD
#endif

NODISCARD vector<string> selection_tokens(const string& command) {
	vector<string>result;
	regex select_exp(R"((^SELECT)\s+([\w_,\s]+)\s+([^(from)]*)\s+)", regex::icase);
	regex select_spec_exp(R"((^SELECT)\s+([*\s]))", regex::icase);
	regex columns_exp("([\\w_]+)");
	smatch matches{};
	regex_search(command, matches, select_exp);
	const int delta = 2;
	if (!matches.empty()) {
		auto iter = matches.begin() + delta;
		for (iter; iter != matches.end(); ++iter) {
			smatch tmp{};
			string tmp_str{ iter->str() };
			auto begin = tmp_str.cbegin();
			auto end = tmp_str.cend();
			while (regex_search(begin, end, tmp, columns_exp)) {
				result.push_back(tmp.str());
				begin += tmp.position() + tmp.str().size();
			}
		}
	}
	else {
		regex_search(command, matches, select_spec_exp);
		if (!matches.empty()) {
			result.emplace_back("*");
		}
	}
	return result;
}

NODISCARD vector<string> from_token(const string& command) {
	regex from_expr(R"((from)\s+([\w_.csv]+))", regex::icase);
	smatch matches;
	regex_search(command, matches, from_expr);
	vector<string> result;
	const int delta = 2;
	auto iter = matches.cbegin() + delta;
	for (; iter != matches.cend(); ++iter) {
		result.push_back(iter->str());
	}
	return result;
}

NODISCARD vector<string> where_tokens(const string& command) {
	regex where_expr(R"((WHERE) (...+))", regex::icase);
	smatch where_match;
	vector<string>result(0);
	regex_search(command, where_match, where_expr);
	if (where_match.empty()) {
		return result;
	}
	regex tokenize_expr(R"(([^\s]+))", regex::icase);
	smatch tokens;
	string raw_string = where_match.str();
	regex_search(raw_string, tokens, tokenize_expr);
	auto begin = raw_string.cbegin();
	auto end = raw_string.cend();
	while (regex_search(begin, end, tokens, tokenize_expr)) {
		result.push_back(tokens.str());
		begin += tokens.position() + tokens.str().size();
	}
	return result;
}

void data_reading(map<string, list<string>*>& columns, ifstream& stream) {
	string tmp;
	getline(stream, tmp);
	regex col_names_expr(R"(([\w_]+))", regex::icase);
	regex col_value(R"(([-\w_]+)|("[^"]*"))", regex::icase);
	smatch columns_list, values_matches;
	auto begin = tmp.cbegin();
	auto end = tmp.cend();
	vector<string> result;
	while (regex_search(begin, end, columns_list, col_names_expr)) {
		result.push_back(columns_list.str());
		begin += columns_list.position() + columns_list.str().size();
	}

	list<list<string>*> lists;
	for (const auto& el : result) {
		auto* new_list = new list<string>;
		lists.push_back(new_list);
		columns.insert(make_pair(el, new_list));
	}
	while (getline(stream, tmp)) {
		begin = tmp.cbegin();
		end = tmp.cend();
		auto list_iter = lists.begin();
		while (regex_search(begin, end, columns_list, col_value)) {
			(*list_iter)->push_back(columns_list[0].str());
			++list_iter;
			begin += columns_list.position() + columns_list.str().size();
		}
	}
}
bool is_number(const string& s)
{
	return !s.empty() && (s.find_first_not_of("0123456789") == string::npos);
}
bool is_operator(const string& s) {
	vector<string> operators = { "<", ">", "<=", ">=","==", "AND", "OR" };
	for (const auto& el : operators) {
		if (el == s) {
			return true;
		}
	}
	return false;
}
int getOperatorPriority(const string& oper) {
	vector<string> low = { "AND", "OR" };
	vector<string> mid = { "<", ">", "<=", ">=","==" };
	for (const auto& el : low) {
		if (oper == el) {
			return 1;
		}
	}
	for (const auto& el : mid) {
		if (oper == el) {
			return 2;
		}
	}
	return -1;
}

queue<string> shuntingYard(const vector<string>& conditions) {
	queue<string> q;
	stack<string> st;

	for (const auto & condition : conditions) {
		if (is_number(condition)) {
			q.push(condition);
		}
		else {
			bool column = true;
			if (is_operator(condition)) {
				while (!st.empty() &&
					getOperatorPriority(st.top()) >= getOperatorPriority(condition)) {
					q.push(st.top());
					st.pop();
				}
				st.push(condition);
				column = false;
			}
			if (column) {
				q.push(condition);
			}
		}
	}
	while (!st.empty()) {
		q.push(st.top());
		st.pop();
	}
	return q;
}
Token getType(const string& s, const map<string, list<string>::iterator>& row) {
	if (is_number(s)) {
		return Token::Number;
	}
	else if (row.find(s) != row.end()) {
		return Token::ColumnName;
	}
	else {
		return Token::String;
	}
}
bool calc(queue<string> q, const map<string, list<string>::iterator>& row) {
	stack<string> localStack;
	string tmp;
	while (!q.empty()) {
		tmp = q.front();
		if (is_operator(tmp)) {
			vector<string> operands_str;
			vector<int> operands_int;
			for (int i = 0; i < 2; ++i) {
				operands_str.push_back(localStack.top());
				localStack.pop();
			}
			if (getType(operands_str[0], row) == Token::Number) {
				for_each(operands_str.begin(), operands_str.end(), [&operands_int](const string& s) {
					operands_int.push_back(atoi(s.c_str())); });
			}
			if (tmp == "<=") {
				bool tmp_bool;
				if (!operands_int.empty()) {
					tmp_bool = (operands_int[1] <= operands_int[0]);
				}
				else {
					tmp_bool = (operands_str[1] <= operands_str[0]);
				}
				localStack.push(to_string(tmp_bool));
			}
			if (tmp == ">=") {
				bool tmp_bool;
				if (!operands_int.empty()) {
					tmp_bool = (operands_int[1] >= operands_int[0]);
				}
				else {
					tmp_bool = (operands_str[1] >= operands_str[0]);
				}
				localStack.push(to_string(tmp_bool));
			}
			if (tmp == "<") {
				bool tmp_bool;
				if (!operands_int.empty()) {
					tmp_bool = (operands_int[1] < operands_int[0]);
				}
				else {
					tmp_bool = (operands_str[1] < operands_str[0]);
				}
				localStack.push(to_string(tmp_bool));
			}
			if (tmp == ">") {
				bool tmp_bool;
				if (!operands_int.empty()) {
					tmp_bool = (operands_int[1] > operands_int[0]);
				}
				else {
					tmp_bool = (operands_str[1] > operands_str[0]);
				}
				localStack.push(to_string(tmp_bool));
			}
			if (tmp == "==") {
				bool tmp_bool;
				if (!operands_int.empty()) {
					tmp_bool = (operands_int[1] == operands_int[0]);
				}
				else {
					tmp_bool = (operands_str[1] == operands_str[0]);
				}
				localStack.push(to_string(tmp_bool));
			}
			if (tmp == "AND") {
				bool tmp_bool;
				if (!operands_int.empty()) {
					tmp_bool = (operands_int[1] && operands_int[0]);
				}
				else {
					throw runtime_error("bad condition");
				}
				localStack.push(to_string(tmp_bool));
			}
			if (tmp == "OR") {
				bool tmp_bool;
				if (!operands_int.empty()) {
					tmp_bool = (operands_int[1] || operands_int[0]);
				}
				else {
					throw runtime_error("bad condition");
				}
				localStack.push(to_string(tmp_bool));
			}
		}
		else {
			Token type = getType(tmp, row);
			if (type == Token::String || type == Token::Number) {
				localStack.push(tmp);
			}
			else {
				while (type == Token::ColumnName) {
					type = getType(*row.at(tmp), row);
					tmp = *row.at(tmp);
				}
				localStack.push(tmp);
			}
		}
		q.pop();
	}
	return atoi(localStack.top().c_str());
}

struct Lam {
	template<typename T>
	void operator()(T& i) {
		cout << *i.second << " ";
	}
};
struct Lam2 {
	template<typename T>
	void operator()(T& i) {
		++(i.second);
	}
};

void scriptProcessing(ifstream& script) {
	string tmp;
	getline(script, tmp);
	vector<string> result = move(from_token(tmp));
	string table_file = result[0];
	result = move(selection_tokens(tmp));

	bool all = false;
	if (result.size() == 1 && result[0] == "*") {
		all = true;
	}
	map<string, list<string>*> columns;
	ifstream file("C:/Users/krage56/Desktop/Введение в РПО/ConsoleApplication1/Debug/database.csv");
	if (file.is_open()) {
		cout << "Open1" << endl;
	}
	else {
		throw runtime_error("not kek");
	}
	data_reading(columns, file);
	vector<string> conditions = move(where_tokens(tmp));
	if (conditions.empty()) {
		getline(script, tmp);
		conditions = move(where_tokens(tmp));
	}
	if (!conditions.empty()) {
		conditions.erase(conditions.begin(), conditions.begin() + 1);
	}

	queue<string> q = move(shuntingYard(conditions));
	map<string, list<string>::iterator> row;
	for (const auto& el : columns) {
		if (all || any_of(result.begin(), result.end(), [el](const string& s) {return el.first == s; })) {
			row.insert(make_pair(el.first, el.second->begin()));
		}
	}
	while (row.begin()->second != columns.at(row.begin()->first)->end()) {
		bool row_accept = calc(q, row);
		if (row_accept) {
			/*std::for_each(row.begin(), row.end(), [](auto& i){cout << *i.second << " ";});*/
			std::for_each(row.begin(), row.end(), Lam());
			cout << "\n";
		}
		std::for_each(row.begin(), row.end(), Lam2());
	}
}


int main() {
	ifstream instructions("C:/Users/krage56/Desktop/Введение в РПО/ConsoleApplication1/Debug/script.sql");
	if (instructions.is_open()) {
		cout << "open2" << endl;
	}
	else {
		return 1;
	}
	scriptProcessing(instructions);
	instructions.close();
	cin.get();
}

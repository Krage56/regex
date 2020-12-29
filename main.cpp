#include "iostream"
#include "fstream"
#include "regex"
#include "map"
#include "list"
#include <queue>
#include <algorithm>
#include <stack>
using namespace std;
enum class TokenType {
    Number,
    String,
    ColumnName
};


[[nodiscard]] vector<string> selectionTokens(const string& command) {
    vector<string> tokenized;
    regex any_word_expr("([\\w_*]+)");
    smatch matches{};
    auto current_pos = command.cbegin();
    size_t pos = 0;
    while(pos < command.size()){
        regex_search(current_pos, command.cend(), matches, any_word_expr);
        tokenized.push_back(matches[0].str());
        current_pos += (matches[0].str().size() + 1);
        pos += (matches[0].str().size() + 1);
        if(!tokenized.empty() && tokenized[tokenized.size() - 1] == "FROM"){
            break;
        }
    }
    if(tokenized[tokenized.size() - 1] == "FROM"){
        tokenized.pop_back();
    }
    if(tokenized[0] != "SELECT"){
        throw runtime_error("There is no 'SELECT' token");
    }
    tokenized.erase(tokenized.begin(), tokenized.begin() + 1);
    return tokenized;
}

[[nodiscard]] vector<string> fromToken(const string& command) {
    regex from_expr(R"((from)\s+([\w_.csv]+))", regex::icase);
    smatch matches;
    regex_search(command, matches, from_expr);
    vector<string> result;
    if(matches.empty()){
        return result;
    }
    const int delta = 2;
    auto iter = matches.cbegin() + delta;
    for (; iter != matches.cend(); ++iter) {
        result.push_back(iter->str());
    }
    return result;
}

[[nodiscard]] vector<string> whereTokens(const string& command) {
    regex where_expr(R"((WHERE) (...+))", regex::icase);
    smatch where_match;
    vector<string>result(0);
    regex_search(command, where_match, where_expr);
    if (where_match.empty()) {
        return result;
    }
    regex tokenize_expr(R"(([^\s&^;]+))", regex::icase);
    smatch tokens;
    string raw_string = where_match.str();
    regex_search(raw_string, tokens, tokenize_expr);
    auto begin = raw_string.cbegin();
    auto end = raw_string.cend();
    while (regex_search(begin, end, tokens, tokenize_expr)) {
        result.push_back(tokens.str());
        begin += tokens.position() + tokens.str().size();
    }
    if(result[0] != "WHERE"){
        throw runtime_error("Incorrect WHERE statement");
    }
    result.erase(result.begin(), result.begin() + 1);
    return result;
}

void printSelectedColumns(const map<string, vector<string>::iterator>& rows){
    for(auto& row: rows){
        cout << *row.second << " ";
    }
    cout << "\n";
}

void printSelectedColumns(const map<string, vector<string>::iterator>& rows, const vector<string>& sel_tokens){
    for(const auto& row: rows){
        for(const auto& token: sel_tokens){
            if(token == row.first){
                cout << *row.second << " ";
            }
        }
    }
    cout << "\n";
}


bool conditionsExists(string& container, istream& stream){
    getline(stream, container);
    if(container.empty()){
        return false;
    }
    regex where_exists_expr(R"((\b[WHERE]+)|(\b[where]+))", regex::icase);
    smatch where_match;
    regex_search(container, where_match, where_exists_expr);
    return !where_match.empty();
}

void dataReading(map<string, vector<string>*>& columns, istream& stream) {
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

    list<vector<string>*> list_of_vec;
    for (const auto& el : result) {
        auto* new_vec = new vector<string>;
        list_of_vec.push_back(new_vec);
        columns.insert(make_pair(el, new_vec));
    }
    while (getline(stream, tmp)) {
        begin = tmp.cbegin();
        end = tmp.cend();
        auto list_iter = list_of_vec.begin();
        while (regex_search(begin, end, columns_list, col_value)) {
            (*list_iter)->push_back(columns_list[0].str());
            ++list_iter;
            begin += columns_list.position() + columns_list.str().size();
        }
    }
}
bool isNumber(const string& s)
{
    return !s.empty() && (s.find_first_not_of("0123456789") == string::npos);
}
bool isOperator(const string& s) {
    vector<string> operators = { "<", ">", "<=", ">=","=", "AND", "OR" };
    for (const auto& el : operators) {
        if (el == s) {
            return true;
        }
    }
    return false;
}
int getOperatorPriority(const string& oper) {
    vector<string> low = { "AND", "OR" };
    vector<string> mid = { "<", ">", "<=", ">=","=" };
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
//Without processing of brackets
queue<string> shuntingYard(const vector<string>& conditions) {
    queue<string> q;
    stack<string> st;
    for (const auto & condition : conditions) {
        if (isNumber(condition)) {
            q.push(condition);
        }
        else {
            bool column = true;
            if (isOperator(condition)) {
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
TokenType getType(const string& s, const map<string, vector<string>::iterator>& row) {
    if (isNumber(s)) {
        return TokenType::Number;
    }
    else if (row.find(s) != row.end()) {
        return TokenType::ColumnName;
    }
    else {
        return TokenType::String;
    }
}


bool calc(queue<string> q, const map<string, vector<string>::iterator>& row) {
    stack<string> localStack;
    string tmp;
    while (!q.empty()) {
        tmp = q.front();
        if (isOperator(tmp)) {
            vector<string> operands_str;
            vector<int> operands_int;
            for (int i = 0; i < 2; ++i) {
                operands_str.push_back(localStack.top());
                localStack.pop();
            }
            if (getType(operands_str[0], row) == TokenType::Number) {
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
            if (tmp == "=") {
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
            TokenType type = getType(tmp, row);
            if (type == TokenType::String || type == TokenType::Number) {
                localStack.push(tmp);
            }
            else {
                while (type == TokenType::ColumnName) {
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


void scriptProcessing(istream& script) {
    string tmp;
    vector<string> selection_tokens;
    vector<string> from_tokens;
    string table_file;
    do{
        getline(script, tmp);
        if(tmp == R"(\quit)")
            return;
        selection_tokens = selectionTokens(tmp);
    }while(selection_tokens.empty());

    do{
        from_tokens = fromToken(tmp);
        if(from_tokens.empty()) {
            getline(script, tmp);
        }
    }while(from_tokens.empty());
    table_file = from_tokens[0];

    bool all = false;
    if (selection_tokens.size() == 1 && selection_tokens[0] == "*") {
        all = true;
    }

    //It works on Linux & Unix
    //Windows OS is stupid
    table_file = "./" + table_file;
    ifstream file(table_file);

    if (!file.is_open()) {
        throw runtime_error("Can't open csv file");
    }
    bool conditions_exists = conditionsExists(tmp, script);
    vector<string> conditions;
    if(conditions_exists)
        conditions = whereTokens(tmp);

    map<string, vector<string>*> columns;
    dataReading(columns, file);

    queue<string> q = shuntingYard(conditions);
    map<string, vector<string>::iterator> rows;
    for(const auto& cell: columns){
        rows.insert(make_pair(cell.first, cell.second->begin()));
    }
    while(rows.begin()->second != columns.at(rows.begin()->first)->end()){
        bool row_accept = q.empty() || calc(q, rows);
        if (row_accept) {
            all ? printSelectedColumns(rows) : printSelectedColumns(rows, selection_tokens);
        }
        for(auto& item: rows){
            ++(item.second);
        }
    }
}


int main() {
    while(true){
        scriptProcessing(cin);
        break;
    }
    return 0;
}

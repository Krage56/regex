#include "iostream"
#include "fstream"
#include "regex"
#include "map"
#include "list"
using namespace std;

[[nodiscard]] vector<string> selection_tokens(const string& command){
    vector<string>result;
    regex select_exp(R"((^SELECT)\s+([\w_,\s]+)\s+([^(from)]*)\s+)", regex::icase);
    regex select_spec_exp(R"((^SELECT)\s+([*\s]))", regex::icase);
    regex columns_exp("([\\w_]+)");
    smatch matches{};
    regex_search(command, matches, select_exp);
    const int delta = 2;
    if(!matches.empty()){
        auto iter = matches.begin() + delta;
        for(iter; iter != matches.end(); ++iter){
            smatch tmp{};
            string tmp_str(iter->str());
            auto begin = tmp_str.cbegin();
            auto end = tmp_str.cend();
            while(regex_search(begin, end, tmp, columns_exp)){
                result.push_back(tmp.str());
                begin += tmp.position() + tmp.str().size();
            }
        }
    }
    else{
        regex_search(command, matches, select_spec_exp);
        if(!matches.empty()){
            result.emplace_back("*");
        }
    }
    return result;
}
[[nodiscard]] vector<string> from_token(const string& command){
    regex from_expr(R"((from)\s+([\w_.csv]+))", regex::icase);
    smatch matches;
    regex_search(command, matches, from_expr);
    vector<string> result;
    const int delta = 2;
    auto iter = matches.cbegin() + delta;
    for(; iter != matches.cend(); ++iter){
        result.push_back(iter->str());
    }
    return result;
}
[[nodiscard]] vector<string> where_tokens(const string& command){
    regex where_expr(R"((WHERE) (...+))", regex::icase);
    smatch where_match;
    vector<string>result(0);
    regex_search(command, where_match, where_expr);
    if(where_match.empty()){
        return result;
    }
    regex tokenize_expr(R"(([^\s]+))", regex::icase);
    smatch tokens;
    string raw_string = where_match.str();
    regex_search(raw_string, tokens, tokenize_expr);
    auto begin = raw_string.cbegin();
    auto end = raw_string.cend();
    while(regex_search(begin, end, tokens, tokenize_expr)){
        result.push_back(tokens.str());
        begin += tokens.position() + tokens.str().size();
    }
    return result;
}

void data_reading(map<string, list<string>*>& columns, ifstream& stream){
    string tmp;
    getline(stream, tmp);
    regex col_names_expr(R"(([\w_]+))", regex::icase);
    regex col_value(R"((["\w_]+)(["\s\w]+))", regex::icase);
    smatch columns_list, values_matches;

    auto begin = tmp.cbegin();
    auto end = tmp.cend();
    vector<string> result;
    while(regex_search(begin, end, columns_list, col_names_expr)){
        result.push_back(columns_list.str());
        begin += columns_list.position() + columns_list.str().size();
    }

    list<list<string>*> lists;
    for(const auto& el: result){
        auto* new_list = new list<string>;
        lists.push_back(new_list);
        columns.insert(make_pair(el, new_list));
    }
    while(getline(stream, tmp)){
        begin = tmp.cbegin();
        end = tmp.cend();
        auto list_iter = lists.begin();
        while(regex_search(begin, end, columns_list, col_value)){
            (*list_iter)->push_back(columns_list[0].str());
            ++list_iter;
            begin += columns_list.position() + columns_list.str().size();
        }
    }
}
int main(){
    //ifstream file("input.txt");
    string command(R"(SELECT * from trash_table.csv)");
    string command1("SELECT res1, test2,test3,test4 from trash_table.csg");
    string command_where(R"(WHERE count <= 5 AND id <= 100 OR jean xor beam)");
    /*auto selection_arguments = move(selection_tokens(command));
    for(const auto& el: selection_arguments){
        cout << el << endl;
    }
    cout << "___________________" << endl;
    auto from_argument = move(from_token(command));
    for(const auto& el: from_argument){
        cout << el << endl;
    }
    cout << "___________________" << endl;
    auto where_tokens_vec = move(where_tokens(command_where));
    for(const auto& el: where_tokens_vec) {
        cout << el << endl;
    }
    string general_string = command1 + " " + command_where;
    cout << "General string processed:" << endl;
    selection_arguments = move(selection_tokens(general_string));
    for(const auto& el: selection_arguments){
        cout << el << endl;
    }
    cout << "___________________" << endl;
    from_argument = move(from_token(general_string));
    for(const auto& el: from_argument){
        cout << el << endl;
    }
    cout << "___________________" << endl;
    where_tokens_vec = move(where_tokens(general_string));
    for(const auto& el: where_tokens_vec) {
        cout << el << endl;
    }
    cout << general_string << endl;*/
    map<string, list<string>*> columns;
    ifstream file("dataBase.csv");
/*    if(file.is_open()){
//        cout << "open" << endl;
        }*/
    data_reading(columns, file);
    for(const auto& column: columns){
        cout << column.first << ": ";
        for(const auto& elem: (*column.second)){
            cout << elem << " ";
        }
        cout << "\n";
    }
    file.close();
}

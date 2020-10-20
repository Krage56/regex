#include "iostream"
#include "fstream"
#include "regex"
using namespace std;

[[nodiscard]] vector<string> selection_args(const string& command){
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
            while(regex_search(begin, end, tmp, columns_exp))
            {
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
[[nodiscard]] vector<string> from_args(const string& command){
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
int main()
{
    ifstream file("input.txt");
    string command(R"(SELECT * from trash_table.csv)");
    string command1("SELECT res1, test2,test3,test4 from trash_table.csg");

    auto selection_arguments = move(selection_args(command));
    for(const auto& el: selection_arguments){
        cout << el << endl;
    }
    cout << "___________________" << endl;
    auto from_argument = move(from_args(command));
    for(const auto& el: from_argument){
        cout << el << endl;
    }

}

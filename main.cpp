#include "iostream"
#include "fstream"
#include "regex"
using namespace std;
int main()
{
    ifstream file("input.txt");
    string command(R"(SELECT * from trash_table.csv)");
    string command1("SELECT res1, test2,test3,test4 from, trash_table.csg");
    //getline(cin, command);
    //cmatch res;
    smatch res1{};
    regex select_expr(R"((^SELECT)\s+([\w_,\s]+)\s+([^(from)]*)\s+)", regex::icase);
    regex select_spec_exp(R"((^SELECT)\s+([*\s]))", regex::icase);
    regex from_expr(R"((from)\s+([\w_.csv]+))", regex::icase);
    regex rex(R"((^SELECT)\s+([\w_,\s]+)\s+(from)\s+([\w_.csv]+))", regex::icase);
    regex rex1("([\\w_]+)");
    vector<string> array;
    regex_search(command, res1, rex);
    for(const auto& el: res1){
        cout << el << endl;
    }
    cout << "___________________" << endl;
    smatch res2{}, res3{};
    regex_search(command, res2, select_expr);
    regex_search(command, res3, from_expr);
    if(res2.size()){
//        for(const auto& el: res2){
//            cout << el << endl;
//        }
        auto iter = res2.begin() + 2;
        for(iter; iter != res2.end(); ++iter){
            smatch tmp{};
            string tmp_str(iter->str());
            //cout << iter->str() << endl;
            auto begin = tmp_str.cbegin();
            auto end = tmp_str.cend();
            while(regex_search(begin, end, tmp, rex1))
            {
                array.push_back(tmp.str());
                cout << tmp.str() << endl;
                begin += tmp.position() + tmp.str().size();
            }
        }
    }
    else{
        regex_search(command, res2, select_spec_exp);
        if(res2.size()){
            cout << "*" << endl;
        }
    }

    cout << "___________________" << endl;
    for(const auto& el: res3){
        cout << el << endl;
    }
    cout << "___________________" << endl;
//    regex_search(command.c_str(), res, rex);
//    if (res.empty())
//        throw std::runtime_error("notkek");
//    string s1(res[2].str());
    //regex_search(s1.c_str(), res1, rex1);
//    auto begin = s1.cbegin();
//    auto end = s1.cend();
//    while(regex_search(begin, end, res1, rex1))
//    {
//        array.push_back(res1.str());
//        begin += res1.position() + res1.str().size();
//    }
//    if (res1.empty())
//        throw std::runtime_error("notkek1");
//    for (const auto& el: res1)
//    {
//
//        cout << el<<endl;
//    }
//        cout << array[0] << endl;
//        cout << array[1] << endl;
//        cout << array[2];

}

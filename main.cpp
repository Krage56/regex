#include "iostream"
#include "fstream"
#include "regex"
using namespace std;
int main()
{
    ifstream file("input.txt");
    string command("SELECT res1, test2,test3 from trash_table.csv");
    //getline(cin, command);
    //cmatch res;
    smatch res1{};
    regex rex(R"((^SELECT)\s+([\w_,\s]+)\s+(from)\s+([\w_.csv]+))", regex::icase);
    regex rex1("([\\w_]+)");
    vector<string> array;
    regex_search(command, res1, rex);
    for(const auto& el: res1){
        cout << el << endl;
    }
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

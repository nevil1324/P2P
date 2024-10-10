#include <bits/stdc++.h>

using namespace std;

vector <int> tokenize(string buffer, char separator){
    vector <int> ans;
    string temp;
    for(auto it: buffer){
        if(it == separator) {
            if(temp.size()) ans.push_back(stoi(temp));
            temp.clear();
        }
        else temp.push_back(it);
    }
    if(temp.size()) ans.push_back(stoi(temp));
    return ans;
}

int main(){

    // string s = "5 10 4 9 8 6 11 7 2 0 16 15 13 17 19 18 21 14 24 1 20 26 12 23 43 58 60 25 27 22 56 59 54 57 51 55 50 28 53 3 42 41 52 40 49 45 48 47 46 44 61 39 34 32 29 30 38 92 33 37 93 36 98 35 107 101 96 95 99 100 104 91 31 102 106 103 105 110 111 112 113 118 119 115 114 116 109 108 121 65 63 64 68 66 67 62 120 117 71 70 76 72 69 74 79 122 77 75 73 78 80 82 83 87 81 85 88 90 89 86 84";
    string s = "9 10 4 5 3 16 7 6 11 8 13 1 2 20 15 19 21 0 17 14 18 25 12 26 22 23 24 28 27 58 52 59 43 55 51 56 54 50 46 48 60 61 45 47 49 57 44 53 38 36 41 42 40 39 33 32 92 34 35 29 37 93 30 31 95 107 94 103 105 96 106 91 99 101 109 102 100 97 104 111 98 112 110 108 113 118 114 120 117 63 62 64 115 119 121 76 66 116 65 69 122 72 71 68 77 67 79 78 82 80 84 73 75 74 81 85 83 86 88 87 89";

    vector <int> v = tokenize(s, ' ');

    sort(v.begin(), v.end());

    for(int i = 0; i < v.size(); i++){
        cout << i << " " << v[i] << endl;
    }

    return 0;
}





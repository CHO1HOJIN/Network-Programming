#include <iostream>
#include <fstream>
#include <sstream>
#include <bitset>
#include <string>
#include <algorithm>
#include <vector>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <cstdio>

using namespace std;

vector<bool> encode(vector<bool>& generator, vector<bool>& dataword, int k);
vector<bool> make_generator(char* generator, int k);
vector<bool> padding_bit(vector<bool>& code);
vector<bool> int_to_bit(int num, int size);
int stoi(const char* str);

char* convert_to_string(vector<bool>& code);

int main(int argc, char* argv[]) {
    if (argc != 5) {
        cout << "usage: ./crc_encoder input_file output_file generator dataword_size" << endl;
        exit(0);
    }

    size_t dataword_size = stoi(argv[4]);
    size_t gen_size = strlen(argv[3]);
    ifstream input_file;
    input_file.open(argv[1]);

    ofstream output_file;
    output_file.open(argv[2]);

    //input file open error
    if (input_file.fail()) {
        cout << "input file open error." << endl;
        exit(0);
    }

    //output file open error
    if (output_file.fail()) {
        cout << "output file open error." << endl;
        exit(0);
    }
    
    //if dataword size is not 4 or 8
    if (dataword_size != 4 && dataword_size != 8) {
        cout << "dataword size must be 4 or 8." << endl;
        exit(0);
    }
    //make generator
    vector<bool> generator = make_generator(argv[3], dataword_size);

    //read 1 byte from input file and encode it with crc
    char byte;
    vector <bool> code;
    int cnt = 0;
    while(input_file.get(byte)){
        cnt++;
        vector<bool> bits;
        vector<bool> codeword;

        //convert byte to bits
        for(int i = 0; i < 8; i++) bits.push_back(byte & (1 << (7 - i)));

        //if dataword size is 4
        if(dataword_size == 4) {
            vector<bool> first, second;
            
            for(int i = 0; i < 4; i++) first.push_back(bits[i]);

            codeword = encode(generator, first, 4);
            code.insert(code.end(), codeword.begin(), codeword.end());
            
            for(int i = 0; i < 4; i++) second.push_back(bits[i+4]);
            codeword = encode(generator, second, 4);
            code.insert(code.end(), codeword.begin(), codeword.end());
        }

        //if dataword size is 8
        else {
            vector<bool> dataword;
            for(int i = 0; i < 8; i++) dataword.push_back(bits[i]);
            codeword = encode(generator, dataword, 8);
            code.insert(code.end(), codeword.begin(), codeword.end());
        }
    }

    //padding 0s in front of the code to make the length of code multiple of 8
    vector<bool> padded_code = padding_bit(code);
    
    /*Debuging*/
    // for(int i = 8; i < padded_code.size(); i++) cout << padded_code[i];
    // cout << endl;
    
    //write the code to output file
    int words_count = padded_code.size() / 8;
    char* str = convert_to_string(padded_code);
    for(int i = 0; i < words_count; i++) {
        output_file << str[i];
    }

    input_file.close();
    output_file.close();

    return 0;
}

vector<bool> encode(vector<bool>& generator, vector<bool>& dataword, int k) {
    int gen_size = generator.size(); //generator size = n - k + 1
    vector<bool> dividend = dataword;
    
    //append n-k 0s to dividend
    for(int i = 0; i < gen_size - 1; i++) dividend.push_back(false);

    //calculate remainder
    int start = 0, end = gen_size - 1;
    for(int i = 0; i < k; i++) {
        if(dividend[start] == true) {
            for(int j = start; j <= end; j++) {
                dividend[j] = dividend[j] ^ generator[j - start];
            }
        }
        start++; end++;
    }

    //append remainder to dataword
    vector<bool> codeword;  
    for(int i = 0; i < k; i++) codeword.push_back(dataword[i]);
    for(int i = 0; i < gen_size - 1; i++) codeword.push_back(dividend[k + i]);

    return codeword;
}

vector<bool> make_generator(char *generator, int k) {
    int gen_size = strlen(generator);
    vector<bool> gen;
    for(int i = 0; i < strlen(generator); i++) {
        if(generator[i] == '1') gen.push_back(true);
        else gen.push_back(false);
    }
    return gen;
}

///padding 0s in front of the code to make the length of code multiple of 8 
///And padding number of padding bits in front of the code
vector<bool> padding_bit(vector<bool>& code) {
    int padding = 8 - code.size() % 8;
    if(padding == 8) padding = 0;
    
    vector<bool> padded_code;
    for(int i = 0; i < padding; i++) padded_code.push_back(false);
    padded_code.insert(padded_code.end(), code.begin(), code.end());
    
    //write the number of padding bits in front of the code
    vector<bool> padding_bits = int_to_bit(padding, 8);
    padded_code.insert(padded_code.begin(), padding_bits.begin(), padding_bits.end());
    return padded_code;

}

/// convert integer to binary
vector<bool> int_to_bit(int num, int size) {
    vector<bool> bits;
    for (int i = size - 1; i >= 0; i--) bits.push_back((num >> i) & 1);
    return bits;
}

/// convert code to string
char* convert_to_string(vector<bool>& code){
    int words_count = code.size() / 8;
    char codeword;
    char* string_code = new char[words_count + 1];
    for(int i = 0; i < words_count; i++){
        codeword = 0;
        for(int j = 0; j < 8; j++){
            codeword += code[i * 8 + j] << (7 - j);
        }
        string_code[i] = codeword;
    }

    string_code[words_count] = '\0';
    return string_code;
    
}

int stoi(const char* str) {
    int num = 0;
    for(int i = 0; i < strlen(str); i++) {
        num = num * 10 + (str[i] - '0');
    }
    return num;
}

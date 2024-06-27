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
vector<bool> make_generator(char* generator, int k);
size_t count_padded_bits(char first);
vector<bool> erase_padded_bits(vector<bool>& code, size_t padded_bits);
bool count_error(vector<bool>& generator, vector<bool>& code, size_t k);
vector<bool> decode(vector<bool>& code, size_t dataword_size, size_t codeword_size, size_t codewords_count);
char* convert_to_string(vector<bool>& data);
int stoi(const char* str);
int main(int argc, char* argv[]){
    if (argc != 6) {
        cout << "usage: ./crc_decoder input_file output_file result_file generator dataword_size" << endl;
        exit(0);
    }    

    size_t dataword_size = stoi(argv[5]);
    size_t gen_size = strlen(argv[4]);

    ifstream input_file;
    input_file.open(argv[1]);

    ofstream output_file;
    output_file.open(argv[2]);

    ofstream result_file;
    result_file.open(argv[3]);

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

    //result file open error
    if (result_file.fail()) {
        cout << "result file open error." << endl;
        exit(0);
    }

    //if dataword size is not 4 or 8
    if (dataword_size != 4 && dataword_size != 8) {
        cout << "dataword size must be 4 or 8." << endl;
        exit(0);
    }

    //make generator
    vector<bool> generator = make_generator(argv[4], dataword_size);

    //read first byte from input file
    char first;
    input_file.read(&first, 1);

    //count padded bits
    size_t padded_bits = count_padded_bits(first);

    //read a input file and store it in a vector
    vector<bool> code;
    char byte;
    int cnt = 0;
    while(input_file.read(&byte, 1)){
        cnt ++;
        vector<bool> bits;
        for(int i = 0; i < 8; i++){
            bits.push_back(byte & (1 << (7 - i)));
        }

        for(int i = 0; i < 8; i++){
            code.push_back(bits[i]);
        }
    }
    /*Debugging*/
    // for(int i = 0; i < code.size(); i++) cout << code[i];
    // cout << endl;
    // fflush(stdout);

    // cout << "byte count: " << cnt << endl;
    // cout << "code size: " << code.size() << endl;

    //erase padded bits
    vector<bool> new_code = erase_padded_bits(code, padded_bits);

    //calculate codeword size
    size_t codeword_size = gen_size + dataword_size - 1;
    
    //count the number of codewords
    size_t codewords_count = new_code.size() / codeword_size;
    
    //count the number of errors
    int error_count = 0;
    for(int i = 0; i < codewords_count; i++){
        vector<bool> codeword;
        for(int j = 0; j < codeword_size; j++){
            codeword.push_back(new_code[i * codeword_size + j]);
        }

        //if decoding is failed, increase error count
        if(!count_error(generator, codeword, dataword_size)){
            error_count++;
        }
    }

    //cout << "error count: " << error_count << endl;
    
    //write the number of codewords and the number of errors in the result file
    result_file << codewords_count << " " << error_count << endl;


    //write the decoded data in the output file
    vector<bool> data = decode(new_code, dataword_size, codeword_size, codewords_count);

    /*Debuging*/
    // cout << "decoded data" << endl;
    // for (int i = 0; i < data.size(); i++) cout << data[i];
    // cout << endl;
    // fflush(stdout);


    char* string_data = convert_to_string(data);
    output_file << string_data;

    input_file.close();
    output_file.close();
    result_file.close();

    return 0;

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


size_t count_padded_bits(char first){
    vector<bool> bits;
    for (int i = 0; i < 8; i++) bits.push_back(first & (1 << i));

    size_t decimal = 0;
    for(int i = 0; i < bits.size(); i++){
        decimal += bits[i] * pow(2, i);
    }
    
    //cout << "padded bits: " << decimal << endl;
    
    return decimal;
}

vector<bool> erase_padded_bits(vector<bool>& code, size_t padded_bits){
    vector<bool> erase_code;
    for(int i = padded_bits; i < code.size(); i++){
        erase_code.push_back(code[i]);
    }

    return erase_code;
}

bool count_error(vector<bool>& generator, vector<bool>& code, size_t k){
    int gen_size = generator.size(); //generator size = n - k + 1
    vector<bool> dividend = code;
    
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

    //check if remainder is all 0s
    for(int i = 0; i < gen_size - 1; i++){
        if(dividend[k + i] == true) return false;
    }

    return true;
}

//decode codeword even if there is an error
vector<bool> decode(vector<bool>& code, size_t dataword_size, size_t codeword_size,size_t codewords_count){
    vector<bool> data;

    for(int i = 0; i < codewords_count; i++){
        vector<bool> decoded;
        for(int j = 0; j < dataword_size; j++){
            decoded.push_back(code[i * codeword_size + j]);
        }

        for(int j = 0; j < dataword_size; j++){
            data.push_back(decoded[j]);
        }

    }

    return data;
}

char* convert_to_string(vector<bool>& data){
    int dataword_count = data.size() / 8;
    char dataword;
    char* string_data = new char[dataword_count + 1];

    for(int i = 0; i < dataword_count; i++){
        dataword = 0;
        for(int j = 0; j < 8; j++){
            dataword += data[i * 8 + j] * pow(2, 7 - j);
        }
        string_data[i] = dataword;
    }

    string_data[data.size() / 8] = '\0';

    // cout << "convert end" << endl;
    // fflush(stdout);

    return string_data;
}

int stoi(const char* str) {
    int num = 0;
    for(int i = 0; i < strlen(str); i++) {
        num = num * 10 + (str[i] - '0');
    }
    return num;
}

#include <iostream>
#include <string>
#include <sstream>
#include <vector>
using namespace std;

enum FormatType {
    R,
    I,
    S,
    U,
    SB,
    UJ,
    ERR
};

const vector<string> rfm{ "add", "sub", "lr.w", "sc.w", "and", "or", "xor", "sll", "srl", "sra" };
const vector<string> ifm{ "addi", "ld", "lw", "lwu", "lh", "lhu", "lb", "lbu", "andi", "ori", "xori", "slli", "srli", "srai", "jalr" };
const vector<string> sfm{ "sd", "sw", "sh", "sb" };
const vector<string> ufm{ "lui" };
const vector<string> sbfm{ "beq", "bne", "blt", "bge", "bltu", "bgeu" };
const vector<string> ujfm{ "jal" };

const vector<unsigned int> rfunct3{ 0b000, 0b000, 0b010, 0b010, 0b111, 0b110, 0b100, 0b001, 0b101, 0b101 };
const vector<unsigned int> ifunct3{ 0b000, 0b011, 0b010, 0b110, 0b001, 0b101, 0b000, 0b100, 0b111, 0b110, 0b100, 0b001, 0b101, 0b101, 0b000 };
const vector<unsigned int> sfunct3{ 0b011, 0b010, 0b001, 0b000 };
const vector<unsigned int> sbfunct3{ 0b000, 0b001, 0b100, 0b101, 0b110, 0b111 };


struct Format {
    unsigned int opcode : 7;
    unsigned int rd : 5;
    unsigned int funct3 : 3;
    unsigned int rs1 : 5;
    unsigned int rs2 : 5;
    unsigned int funct7 : 7;
};

union Encode {
    unsigned long int bin;
    Format code;
};

void setData(Encode& input, unsigned int opcode, unsigned int rd, unsigned int funct3, unsigned int rs1, unsigned int rs2, unsigned int funct7);
FormatType getFormat(string input);
int getIndex(vector<string> v, string K);
int getImm(string& input);
int getInt(string inputString);

int main() {
    while (1) {
        string ins;
        cout << "Input RISC V instruction (press 0 to exit): ";
        getline(cin, ins);
        if (ins == "0") return 0;
        stringstream s{ ins };
        string name, des, source1, source2;
        s >> name >> des >> source1 >> source2;

        Encode temp{};

        FormatType fm{ getFormat(name) };
        unsigned int imm;
        unsigned long int uj;
        switch (fm) {
        case R:
            if (name == "lr.w") setData(temp, 0b0101111, getInt(des), rfunct3.at(getIndex(rfm, name)), getInt(source1), 0, 0b0001000);
            else if (name == "sc.w") setData(temp, 0b0101111, getInt(des), rfunct3.at(getIndex(rfm, name)), getInt(source1), getInt(source2), 0b0001100);
            else {
                if (name == "sub" || name == "sra") setData(temp, 0b0110011, getInt(des), rfunct3.at(getIndex(rfm, name)), getInt(source1), getInt(source2), 0b0100000);
                else setData(temp, 0b0110011, getInt(des), rfunct3.at(getIndex(rfm, name)), getInt(source1), getInt(source2), 0b0000000);
            }
            break;
        case I:
            temp.code.funct3 = ifunct3.at(getIndex(ifm, name));
            if (name.front() == 'l') {
                imm = getImm(source1);
                setData(temp, 0b0000011, getInt(des), ifunct3.at(getIndex(ifm, name)), getInt(source1), imm, imm >> 5);
            }
            else {
                imm = getInt(source2);
                if (name == "jalr") setData(temp, 0b1100111, getInt(des), ifunct3.at(getIndex(ifm, name)), getInt(source1), imm, imm >> 5);
                else if (name == "slli" || name == "srli") setData(temp, 0b0010011, getInt(des), ifunct3.at(getIndex(ifm, name)), getInt(source1), imm, 0b0000000);
                else if (name == "srai") setData(temp, 0b0010011, getInt(des), ifunct3.at(getIndex(ifm, name)), getInt(source1), imm, 0b0100000);
                else setData(temp, 0b0010011, getInt(des), ifunct3.at(getIndex(ifm, name)), getInt(source1), imm, imm >> 5);
            }
            break;
        case S:
            imm = getImm(source1);
            setData(temp, 0b0100011, imm, sfunct3.at(getIndex(sfm, name)), getInt(source1), getInt(des), imm >> 5);
            break;
        case U:
            imm = getInt(source1);
            setData(temp, 0b0110111, getInt(des), imm, imm >> 3, imm >> 8, imm >> 13);
            break;
        case SB:
            imm = getInt(source2);
            setData(temp, 0b1100011, (((imm >> 1) << 1) ^ (imm >> 11)), sbfunct3.at(getIndex(sbfm, name)), getInt(des), getInt(source1), (((imm >> 12) << 6) ^ ((imm >> 5) & 0b0111111)));
            break;
        case UJ:
            imm = getInt(source1);
            uj = (((imm >> 1) & 0b10000000000000000000) ^ ((imm << 11 >> 3) & 0b1111111111000000000));
            uj = uj ^ ((imm << 9 >> 12) & 0b100000000) ^ (imm << 1 >> 13);
            setData(temp, 0b1101111, getInt(des), uj, uj >> 3, uj >> 8, uj >> 13);
            break;
        case ERR:
            cout << "Wrong instruction!\n";
            continue;
        }
        cout << "Encode instruction: ";
        cout << hex << (temp.bin) << '\n';
    }
}

void setData(Encode& input, unsigned int opcode, unsigned int rd, unsigned int funct3, unsigned int rs1, unsigned int rs2, unsigned int funct7) {
    input.code.opcode = opcode;
    input.code.rd = rd;
    input.code.funct3 = funct3;
    input.code.rs1 = rs1;
    input.code.rs2 = rs2;
    input.code.funct7 = funct7;
}

bool contains(vector<string> vec, const string& elem)
{
    bool result = false;
    if (find(vec.begin(), vec.end(), elem) != vec.end())
    {
        result = true;
    }
    return result;
}

FormatType getFormat(string input) {
    if (contains(rfm, input)) return R;
    else if (contains(ifm, input)) return I;
    else if (contains(sfm, input)) return S;
    else if (contains(ufm, input)) return U;
    else if (contains(sbfm, input)) return SB;
    else if (contains(ujfm, input)) return UJ;
    else return ERR;
}

int getIndex(vector<string> v, string K) {
    auto it = find(v.begin(), v.end(), K);
    if (it != v.end()) {
        int index = it - v.begin();
        return index;
    }
    else {
        return -1;
    }
}

int getImm(string& input) {
    string immS;
    int i{ 0 };
    while (input[0] != '(') {
        immS += input[0];
        input.erase(0, 1);
    }
    return getInt(immS);
}

int getInt(string inputString) {
    string input{ inputString };
    if (input.back() == ',') input.pop_back();
    if (input[0] == '0' && input[1] == 'x') return stoi(input.erase(0, 2), 0, 16);
    if (input[0] == '-') return -1 * stoi(input.erase(0, 1));
    if (input[0] == '(') {
        if (input[1] == 'x') input.erase(0, 2);
        else input.erase(0, 1);
        input.pop_back();
        return stoi(input);
    }
    if (input[0] == 'x') return stoi(input.erase(0, 1));
    else return stoi(input);
}
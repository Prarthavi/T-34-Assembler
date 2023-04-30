// T34 Assembler.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <algorithm>
#include <map>
#include <vector>
#include <regex>
#include <iomanip>
using namespace std;

string trimandUpper(string str);
void generateAssembly(vector<string> og,string fileName);

map<int, char> dec_value_of_hex();
map<char, int> hex_value_of_dec();
bool compare(string a, string b, char op);
bool cmp(pair<string, string>& a,
	pair<string, string>& b);
int convertBin(string a);
int octalToDecimal(int n);
string performOp(string a, string b, char op);
string subtractHex(string a, string b,bool & negative);
string findPattern(string operand, map<string, string> symbolTable);
string parseOperand(string operand, map<string, string> symbolTable,bool & negative);
enum ERRORS { NONE, BADOPCODE, BADADDRESSMODE, BADBRANCH, BADOPERAND, DUPLICATESYMBOL, MEMORYFULL };
enum ADDRESSINGMODES { IMMEDIATE, INDIRECT, ZEROPAGE, ZEROPAGEX, ZEROPAGEY, ABSOLUTE, ABSOLUTEX, ABSOLUTEY, INDIRECTX, INDIRECTY, IMPLIED, ACCUMULATOR, RELATIVE,CHK };
map<string, map< ADDRESSINGMODES, string>> generateInstructionSet();
struct info {
	ADDRESSINGMODES modeType;
	string address;
	ERRORS error = NONE;
	int num;
};
struct result {
	string address = "";
	string opcode = "";
	string bytes = "";
	int num;
	string line = "";
	ERRORS error = NONE;

};
bool firstPass(map<string, string>& symbolTable, map<string, vector<info>>& instructions, vector<string> og);
bool addToSymbolTable(map<string, string>& symbolTable, map<string, vector<info>>& instructions, ADDRESSINGMODES mode, string label, string& addr, string line, string offset,int i);




int main(int argc, char** argv) {
	fstream infile;
	fstream outfile;
	string fileName;
	vector<string> og;
	vector<string> nonComments;
	vector<string> allFile;
	string ext = ".s";
	map<string, map<string, string>> opcodes;
		
		//cout << "Enter file name: ";
		//cin >> fileName;
	if (argc == 1)
		fileName = argv[0];
	else
		fileName = argv[1];
		std::string::size_type i = fileName.find(ext);
		if (i != std::string::npos)
			fileName.erase(i, ext.length());
		infile.open(fileName + ext, ios::in);

		if (!infile) {
			cout << "File not created!";
		}
		else {

			string line;
			while (getline(infile, line)) {

				og.push_back(line);
				cout << line << endl;
			}

			infile.close();
		}

		generateAssembly(og,fileName);

	
	
	//outfile.open(fileName + ".o", ios::out);
	//if (!outfile) {
	//	cout << "File not created!";
	//	outfile << "Assembling" << endl;
	//}
	//else {
	//	cout << "File created successfully!";
	//	outfile.close();
	//}
	return 0;
}

// values to decimal
map<char, int> hex_value_of_dec()
{
	// Map the values to decimal values
	map<char, int> m{ { '0', 0 }, { '1', 1 },
					  { '2', 2 }, { '3', 3 },
					  { '4', 4 }, { '5', 5 },
					  { '6', 6 }, { '7', 7 },
					  { '8', 8 }, { '9', 9 },
					  { 'A', 10 }, { 'B', 11 },
					  { 'C', 12 }, { 'D', 13 },
					  { 'E', 14 }, { 'F', 15 } };

	return m;
}


map<int, char> dec_value_of_hex()
{

	map<int, char> m{ { 0, '0' }, { 1, '1' },
					  { 2, '2' }, { 3, '3' },
					  { 4, '4' }, { 5, '5' },
					  { 6, '6' }, { 7, '7' },
					  { 8, '8' }, { 9, '9' },
					  { 10, 'A' }, { 11, 'B' },
					  { 12, 'C' }, { 13, 'D' },
					  { 14, 'E' }, { 15, 'F' } };

	return m;
}

string performOp(string a, string b, char op)
{
	stringstream  ss;
	std::string::size_type pos = a.find("#");
	if (pos != std::string::npos)
		a.erase(pos, 1);
	pos = b.find("#");
	if (pos != std::string::npos)
		b.erase(pos, 1);
	a = "0x" + regex_replace(a, regex("\\$"), "");
	b = "0x" + regex_replace(b, regex("\\$"), "");
	const char* astr = a.c_str();
	const char* bstr = b.c_str();
	int aHex = strtol(astr, NULL, 16);
	int bHex = strtol(bstr, NULL, 16);

	switch (op)
	{
		case '+':
			ss << std::showbase << std::uppercase << hex << (aHex + bHex);
			break;
		case '*':
			ss << std::showbase << std::uppercase << hex << (aHex * bHex);
			break;
		case '/':
			ss << std::showbase << std::uppercase << hex << (aHex / bHex);
			break;
		case '&':
			ss << std::showbase << std::uppercase << hex << (aHex & bHex);
			break;
		case '.':
			ss << std::showbase << std::uppercase << hex << (aHex | bHex);
			break;
		default:
			ss << std::showbase << std::uppercase << hex << (aHex ^ bHex);
			break;
	}
	
	std::string result = ss.str();

	result = regex_replace(result, regex("0X"), "");
	if (result.size() > 4)
	{
		result = result.substr(result.size() - 4, 4);
	}
	if (result.size() == 1)
	{
		result = "0" + result;
	}
	return result;

}

int convertBin(string a)
{
	int result = 0;
	for (size_t count = 0; count < a.length(); ++count)
	{
		
		result *= 2;
		result += a[count] == '1' ? 1 : 0;
	}
	return result;
}

int octalToDecimal(int n)
{
	int num = n;
	int dec_value = 0;

	// Initializing base value
	// to 1, i.e 8^0
	int base = 1;

	int temp = num;
	while (temp)
	{

		// Extracting last digit
		int last_digit = temp % 10;
		temp = temp / 10;

		// Multiplying last digit with
		// appropriate base value and
		// adding it to dec_value
		dec_value += last_digit * base;

		base = base * 8;
	}
	return dec_value;
}
string correctFormat(string a)
{
	//TODO: if starts with O convert from octal to hex
//TODO: if starts with < convert take two byte
//TODO: if starts with > convert take high byte
// TODO: else convert from dec to hex
	switch (a[0])
	{
		case '%':
			a = convertBin(a);
		case 'O':
			a = octalToDecimal(stoi(a));
			break;
		case '$':
			break;
		case '<':
			break;
		case '>':
			break;
		default:
			break;
	}
	return a;
}
string subtractHex(string a, string b,bool &neg)
{
	stringstream  ss;
	std::string::size_type pos = a.find("#");
	if (pos != std::string::npos)
		a.erase(pos, 1);
	pos = b.find("#");
	if (pos != std::string::npos)
		b.erase(pos, 1);

	correctFormat(a);
	correctFormat(b);

	//if starts with $ 
	a = "0x" + regex_replace(a, regex("\\$"), "");
	b = "0x" + regex_replace(b, regex("\\$"), "");
	
	const char* astr = a.c_str();
	const char* bstr = b.c_str();
	int aHex = strtol(astr, NULL, 16);
	int bHex = strtol(bstr, NULL, 16);
	ss << std::showbase << std::uppercase << hex << (aHex - bHex);
	std::string result = ss.str();
	
	result = regex_replace(result, regex("0X"), "");
	if (result.size() > 4 && regex_match(result.substr(0, result.size() - 4),regex("(^F+$)")))
	{
		result = result.substr(result.size() - 4,4);
		neg = true;
	}
	if (result.size() == 1)
	{
		result = "0" + result;
	}
	return result;
	
}


string parseOperand(string operand, map<string, string> symbolTable,bool & negative)
{
	map<string, string>::iterator itr;


	std::string::size_type pos = operand.find("#");
	if (pos != std::string::npos)
	{
		operand.erase(pos, 1);
	}

	pos = operand.find("(");
	if (pos != std::string::npos)
	{
		operand.erase(pos, 1);
	}
	pos = operand.find(")");
	if (pos != std::string::npos)
	{
		int size = operand.size() - pos + 1;
		operand.erase(pos, size);
	}

	pos = operand.find(",");
	if (pos != std::string::npos)
	{
		int size = operand.size() - pos + 1;
		operand.erase(pos, size);
	}

	string elem = "";
	vector<string>values;
	vector<char> operations;
	operand = findPattern(operand, symbolTable);
	//assuming that correct number of operations and value and that expression is valid
	for (int i = 0; i < operand.size();i++)
	{
		if (operand[i] != '*' && operand[i] != '+' && operand[i] != '/' && operand[i] != '-' && operand[i] != '&' && operand[i] != '.' &&  operand[i] != '!')
		{

			elem = elem + operand[i];
		}
		
		else
		{
			operations.push_back(operand[i]);
			values.push_back(elem);
			elem = "";
		}
	}
	if(!elem.empty())
	values.push_back(elem);
	string a = "";
	for (int i = 0; i < operations.size();i++)
	{
		string b = "0";
		if(i == 0)
			a = values[0];
		if(i + 1 < values.size())
			 b = values[i + 1];
		else if (operations[i] == '*' || operations[i] == '&' || operations[i] == '/')
			b = "1";
		
		switch (operations[i])
		{
			
			case '-':
				a = "$" + subtractHex(a, b, negative);
				break;	
			default:
				a = "$" + performOp(a, b, operations[i]);
				break;
		}
	}
	if (!a.empty())
		operand = a;
	//operand = performOperations(operand, "*", symbolTable);
	//operand = performOperations(operand, "+", symbolTable);
	//TODO: implement the & and other ones
	return operand;
}


bool addToSymbolTable(map<string, string>& symbolTable, map<string, vector<info>>& instructions, ADDRESSINGMODES mode, string label, string& addr, string line, string offset, int num)
{
	info in;
	if (symbolTable.size() > 255 || (compare(addr, "$FFFF", '>') || compare(addr, "$FFFF", '=')))
	{
		cout << "Memory Full" << endl;
		return true;
		in.error = MEMORYFULL;
	}
	if (symbolTable.count(label) > 0)
	{
		in.error = DUPLICATESYMBOL;
		cout << "Duplicate symbol in line: " << num << endl;
		
		cin.get();
		in.address = addr;
		in.modeType = mode;
		addr = "$" + performOp(addr, "$" + offset,'+');
	}
	else
	{
		in.modeType = mode;
		in.address = addr;
		if (!label.empty())
			symbolTable.insert(pair<string, string>(label, addr));
		addr = "$" + performOp(addr, "$" + offset,'+');
	}

	if (instructions.find(line) == instructions.end())
	{
		vector<info> vec;
		vec.push_back(in);
		instructions.insert(pair<string, vector<info>>(line,vec));
	}
	else
	{
		instructions[line].push_back(in);
	}
	return false;
}

bool firstPass(map<string, string>& symbolTable, map<string, vector<info>>& instructions, vector<string> og)
{
	map<string, map< ADDRESSINGMODES, string>> instructionSet = generateInstructionSet();
	string line;
	string label;
	string instr;
	string operand;

	string addr = "$8000";
	ADDRESSINGMODES modeType;
	bool neg;
	bool full = false;
	for (int i = 0; i < og.size() && !full; i++)
	{
		if (og[i].at(0) != '*')
		{
			operand = "";
			label = "";
			instr = "";
			line = og[i];
			
			if (line.size() > 1)
				label = line.substr(0, 9);
			if (line.size() > 10)
				instr = line.substr(9, 5);
			if (line.size() > 15)
				operand = line.substr(14, 10);

			label = trimandUpper(label);
			instr = trimandUpper(instr);
			operand = trimandUpper(operand);
			
			if (instructionSet.count(instr) > 0 || instr == "ORG" || instr == "EQU" || instr == "CHK" || instr == "END")
			{
				if (instr == "ORG")
					addr = operand;
				else if (instr == "EQU")
				{
					if (symbolTable.count(label) <= 0)
						symbolTable.insert(pair<string, string>(label, operand));
					else
					{
						cout << "Bad operand error at line: " << i + 1;
						full = true;
					}
				}
				else if (operand.empty())
				{
					if (instr == "ASL")
						full = addToSymbolTable(symbolTable,instructions,ACCUMULATOR,  label, addr, line,"1",i + 1);
					else
						full = addToSymbolTable(symbolTable, instructions, IMPLIED, label, addr, line, "1", i + 1);
				}
				else if (instr == "BCC" || instr == "BCS" || instr == "BEQ" || instr == "BMI" || instr == "BNE" || instr == "BPL" || instr == "BVC" || instr == "BVS")
					full = addToSymbolTable(symbolTable, instructions, RELATIVE, label, addr, line, "2",i + 1);

				else if (instr == "JMP" || instr == "JSR")
				{
					if (regex_match(operand, regex("(^[^#\\(][^,]+)")))
						full = addToSymbolTable(symbolTable, instructions, ABSOLUTE, label, addr, line, "3", i + 1);

					if (regex_match(operand, regex("(^\\([^,]+\\)$)")))
						full = addToSymbolTable(symbolTable, instructions, INDIRECT, label, addr, line, "3", i + 1);
				}
				else if (operand.find('#') != string::npos)
					full = addToSymbolTable(symbolTable, instructions, IMMEDIATE, label, addr, line, "2", i + 1);
				else if (regex_match(operand, regex("(^\\$[0-9][0-9])")))
				{
					full = addToSymbolTable(symbolTable, instructions, ZEROPAGE, label, addr, line, "2", i + 1);
				}
				else if (regex_match(operand, regex("(^.+,X)")))
				{
					
					operand = parseOperand(operand,symbolTable,neg);
					if(operand.size() > 3)
						full = addToSymbolTable(symbolTable, instructions, ABSOLUTEX, label, addr, line, "3", i + 1);
						
					else
						full = addToSymbolTable(symbolTable, instructions, ZEROPAGEX, label, addr, line, "2", i + 1);
				}
				else if (regex_match(operand, regex("(^\\(.+,X\\)$)")))
					full = addToSymbolTable(symbolTable, instructions, INDIRECTX, label, addr, line, "2", i + 1);
				else if (regex_match(operand, regex("(^\\(.+\\),Y$)")))
					full = addToSymbolTable(symbolTable, instructions, INDIRECTY, label, addr, line, "2", i + 1);
				else if (regex_match(operand, regex("(^\\([^,]+\\)$)")))
					full = addToSymbolTable(symbolTable, instructions, INDIRECT, label, addr, line, "3", i + 1);
				else if (regex_match(operand, regex("(^.+,Y)")))
				{
					operand = parseOperand(operand, symbolTable,neg);
					if (operand.size() > 3)
						full = addToSymbolTable(symbolTable, instructions, ABSOLUTEY, label, addr, line, "3", i + 1);

					else
						full = addToSymbolTable(symbolTable, instructions, ZEROPAGEY, label, addr, line, "2", i + 1);
				}
				else if (regex_match(operand, regex("(^\\$[0-9][0-9][0-9][0-9]$)")))
				{
	
					full = addToSymbolTable(symbolTable, instructions, ABSOLUTE, label, addr, line, "3", i + 1);
				}
				
				else if (regex_match(operand, regex("(^.+,\s*X$)")))
				{
					full = addToSymbolTable(symbolTable, instructions, ABSOLUTEX, label, addr, line, "3", i + 1);
				}
				else if (regex_match(operand, regex("(^.+,Y$)")))
				{
					full = addToSymbolTable(symbolTable, instructions, ABSOLUTEY, label, addr, line, "3", i + 1);
				}
				else if (instr == "CHK")
				{
					full = addToSymbolTable(symbolTable, instructions, CHK, label, addr, line, "0", i + 1);
				}
				else if (symbolTable.count(operand) > 0)
				{
					
					string loc = symbolTable[operand];
					if (loc.size() == 3)
					{
						full = addToSymbolTable(symbolTable, instructions, ZEROPAGE, label, addr, line, "2", i + 1);
					}
					else
						full = addToSymbolTable(symbolTable, instructions, ABSOLUTE, label, addr, line, "3", i + 1);
					
				}
				else
				{
					//parse the label for operations
					string tempOp = parseOperand(operand,symbolTable,neg);

						if (tempOp.size() == 3)
						{
							full = addToSymbolTable(symbolTable, instructions, ZEROPAGE, label, addr, line, "2", i + 1);
						}
						else
						{
							full = addToSymbolTable(symbolTable, instructions, ABSOLUTE, label, addr, line, "3", i + 1);
						}
					
				}
			
			}
			else
			{
				info in;
				cout << "“Bad opcode in line: " << i + 1;
				full = true;
				/*if (instructions.find(line) == instructions.end())
				{
					vector<info> vec;
					vec.push_back(in);
					instructions.insert(pair<string, vector<info>>(line, vec));
				}
				else
				{
					instructions[line].push_back(in);
				}*/
			}
			
		}
		else if (og[i].size() > 64)
		{
			cout << "Bad operand at line: " << i + 1;
			
			full = true;

		}

	}

	return full;
}

bool compare(string a, string b,char op)
{
	stringstream  ss;
	std::string::size_type pos = a.find("#");
	if (pos != std::string::npos)
		a.erase(pos, 1);
	pos = b.find("#");
	if (pos != std::string::npos)
		b.erase(pos, 1);

	correctFormat(a);
	correctFormat(b);

	a = "0x" + regex_replace(a, regex("\\$"), "");
	b = "0x" + regex_replace(b, regex("\\$"), "");

	const char* astr = a.c_str();
	const char* bstr = b.c_str();
	long result = strtol(astr, NULL, 16);
	long result2 = strtol(bstr, NULL, 16);
	bool truth;
	if (op == '>')
		truth = result > result2;
	else if (op == '<')
		truth = result < result2;
	else
		truth = result == result2;
	return truth;
}
//IT IS BAD OPERAND if a instr uses a label and then we try to equ the label to a zero page value ?
//TODO support all the label format
//TODO support exclusive or
//TODO error code

vector<result> secondPass(map<string, string> symbolTable, map<string, vector<info>>& instructions, vector<string> og, int & bytesNum) {
	//second pass
	string line;
	string label;
	string instr;
	string operand;
	string comment;
	vector<result> results;
	map<string, map< ADDRESSINGMODES, string>> instructionSet = generateInstructionSet();
	vector<string> opcodes;
	for (int i = 0; i < og.size(); i++)
	{
		
		operand = "";
		label = "";
		instr = "";
		line = og[i];

			if (line.size() > 1)
				label = line.substr(0, 9);
			if (line.size() > 10)
				instr = line.substr(9, 5);
			if (line.size() > 15)
				operand = line.substr(14, 10);

			label = trimandUpper(label);
			instr = trimandUpper(instr);
			operand = trimandUpper(operand);
			//handle if operand not proper;
			result res;
			if (instr == "BCC" || instr == "BCS" || instr == "BEQ" || instr == "BMI" || instr == "BNE" || instr == "BPL" || instr == "BVC" || instr == "BVS")
			{

				info in = instructions[line][0];
				instructions[line].erase(instructions[line].begin());
				bool neg = false;
				string op = parseOperand(operand, symbolTable,neg);
				string temp = subtractHex(op, in.address,neg);
				temp = subtractHex(temp, "$2",neg);
				if (!neg && ((compare(temp, "$FF",'>') && !compare(temp, "$FF", '=')) || (compare(temp,"$00",'<') && !compare(temp, "$00", '='))))
				{
					res.error = BADBRANCH;
					res.opcode = "";
					res.bytes = temp.size() > 2 ? temp.substr(0, 2) : temp;
					res.bytes = res.bytes + " ";
				}
				else
				{
					res.opcode = instructionSet[instr][in.modeType];
					res.bytes = temp.size() > 2 ? temp.substr(temp.size() - 2, 2) : temp;
					res.bytes = res.bytes + " ";
					res.address = in.address;
				}
				
				
				
				res.line = line;
				res.num = i + 1;
				
				if (instructionSet[instr].count(in.modeType) <= 0 && instr != "CHK" && instr != "END" && instr != "ORG" && instr != "EQU")
				{
					res.error = BADADDRESSMODE;
					res.opcode = "";
					res.address = "";
				}
				opcodes.push_back(res.opcode);
				opcodes.push_back(res.bytes);

					results.push_back(res);
			}
			else if (instr == "CHK")
			{
				info in = instructions[line][0];
				instructions[line].erase(instructions[line].begin());
				string s = "0x" + opcodes[0];
				const char* sstr = s.c_str();
				long xorResult = strtol(sstr, NULL, 16);
				stringstream  ss;
				for (int r =  1; r < opcodes.size(); r++)
				{
					if (!opcodes[r].empty())
					{
						s = "0x" + opcodes[r];
						sstr = s.c_str();
						long op = strtol(sstr, NULL, 16);
						xorResult = xorResult ^ op;
					}

				}
				ss << std::showbase << std::uppercase << hex << xorResult;
				string result3 = ss.str();
				result3 = result3.substr(2, result3.size() - 1);
				res.opcode = result3;
				res.line = line;
				res.num = i + 1;
				res.address = in.address;
				results.push_back(res);
				opcodes.push_back(result3);
			}
			else
			{
				if (instructions.count(line) > 0)
				{
					info in = instructions[line][0];
					instructions[line].erase(instructions[line].begin());
					res.address = instr == "END" ? "" : in.address;

					if (instructionSet[instr].count(in.modeType) <= 0 && instr != "CHK" && instr != "END" && instr != "ORG" && instr != "EQU")
					{
						res.error = BADADDRESSMODE;
						res.opcode = "";
						res.address = "";
					}
					
					else
					{
						res.opcode = instructionSet[instr][in.modeType];
						opcodes.push_back(res.opcode);
					}
					res.line = line;
					res.num = i + 1;
					string first;
					string second;
					bool neg;
					operand = parseOperand(operand, symbolTable,neg);
					if (in.error != NONE)
					{

						res.error = in.error;
					}
					std::string::size_type pos = operand.find("$");
					if (pos != std::string::npos)
						operand.erase(pos, 1);

					if (operand.size() > 3 && res.error != BADADDRESSMODE)
					{
						opcodes.push_back(operand.substr(operand.size() - 2, 2));
						opcodes.push_back(operand.substr(operand.size() - 4, 2));
						res.bytes = operand.substr(operand.size() - 2, 2) + " " + operand.substr(operand.size() - 4, 2);
					}
					else if (operand.size() == 3 && res.error != BADADDRESSMODE)
					{
						opcodes.push_back(operand.substr(operand.size() - 2, 2));
						opcodes.push_back("0" + operand.substr(operand.size() - 3, 1));
						res.bytes = operand.substr(operand.size() - 2, 2) + " 0" + operand.substr(operand.size() - 3, 1);
					}
					else if( res.error != BADADDRESSMODE)
					{
						opcodes.push_back(operand);
						res.bytes = operand + "   ";
					}

				}
				else
				{
					res.line = line;
					res.num = i + 1;

				}

				results.push_back(res);
			}
			
	}
	bytesNum = 0;
	for (int r = 0; r < opcodes.size(); r++)
	{
		if (!opcodes[r].empty())
		{
			bytesNum += 1;
		}
	}
	return results;
}



void generateAssembly(vector<string> og,string fileName)
{
	fstream outfile;
	map<string, map< ADDRESSINGMODES, string>> instructionSet = generateInstructionSet();

	string addr = "$8000";
	map<string, string> symbolTable;
	ADDRESSINGMODES modeType;
	
	map<string,vector<info>> instructions;
	int count = 0;

	//first pass
	if (firstPass(symbolTable, instructions, og))
		return;
	int byteNum = 0;
	vector<result> results = secondPass(symbolTable,instructions,og,byteNum);

	fileName = fileName + ".o";
	outfile.open(fileName, ios::out);
	if (!outfile) {
		cout << "File not created!";
	}

	cout << "Assembling";
	cout << endl << endl;
	int errorCount = 0;
	char input;
	for (int i = 0; i < results.size();i++)
	{
		result temp = results[i];
		string space = temp.address.empty() ? "  " : ": ";
		string address = regex_replace(temp.address, regex("\\$"), "");
		if (temp.error != NONE)
		{

			if (temp.error == BADADDRESSMODE)
			{
				cout << "Bad address mode in line: " << temp.num << endl;
				cin.get();
				errorCount++;
			}
			if (temp.error == DUPLICATESYMBOL)
			{
				
				errorCount++;
			}
			if (temp.error == BADBRANCH)
			{
				
				cout << "Bad branch in line: " << temp.num << endl;
				cin.get();
				errorCount++;
			}
		}
		else if(!temp.opcode.empty())
		{
			outfile << temp.address << ": " << temp.opcode << " " << temp.bytes << endl;
		}
			cout << setw(4) << address << space;
			cout << setw(3) << temp.opcode << setw(7)  << temp.bytes;
			cout << setw(4) << temp.num << " " << temp.line << endl;
		

	}
	outfile.close();
	cout << endl << "--End assembly, " << byteNum << " bytes, Errors: " << errorCount << endl << endl;
	cout << "Symbol Table - alphabetical order:" << endl;
	map<string, string>::iterator itr;
	for (itr = symbolTable.begin(); itr != symbolTable.end(); ++itr) {
		cout << '\t' << itr->first << '=' << itr->second;
		
	}
	cout << endl << endl << "Symbol Table - numerical order:" << endl;

	vector<pair<string, string> > S;
	for (auto& it : symbolTable) {
		S.push_back(it);
	}
	sort(S.begin(), S.end(), cmp);

	for (auto& it : S) {

		cout << '\t' << it.first << '=' << it.second;
	}
	cout << endl << endl;
}

bool cmp(pair<string, string>& a,pair<string, string>& b)
{
	return compare(a.second,b.second,'<');
}



string findPattern(string operand, map<string, string> symbolTable)
{
	for (int i = operand.size(); i > 0; i--)
	{
		string pos = operand.substr(0, i);
		if (symbolTable.count(pos) > 0)
			operand.replace(0, i, symbolTable[pos]);
	}
	return operand;
}

string trimandUpper(string str) {
	size_t endpos = str.find_last_not_of(" \t");
	size_t startpos = str.find_first_not_of(" \t");
	if (std::string::npos != endpos)
	{
		str = str.substr(0, endpos + 1);
		str = str.substr(startpos);
	}
	else {

		str.erase(std::remove(std::begin(str), std::end(str), ' '), std::end(str));
	}
	if (string::npos != startpos)
	{
		str = str.substr(startpos);
	}
	transform(str.begin(), str.end(), str.begin(), ::toupper);
	return str;
}

bool greater() { return true; }

//IF ADDRESS MORE THAN FFFF BAD KUBW
map<string, map< ADDRESSINGMODES, string>> generateInstructionSet()
{
	map<string, map< ADDRESSINGMODES, string>>instructionSet;
	instructionSet.insert(make_pair("ADC", map< ADDRESSINGMODES, string>()));
	instructionSet["ADC"].insert(make_pair(IMMEDIATE, "69"));
	instructionSet["ADC"].insert(make_pair(ZEROPAGE, "65"));
	instructionSet["ADC"].insert(make_pair(ZEROPAGEX, "75"));
	instructionSet["ADC"].insert(make_pair(ABSOLUTE, "6D"));
	instructionSet["ADC"].insert(make_pair(ABSOLUTEX, "7D"));
	instructionSet["ADC"].insert(make_pair(ABSOLUTEY, "79"));
	instructionSet["ADC"].insert(make_pair(INDIRECTX, "61"));
	instructionSet["ADC"].insert(make_pair(INDIRECTY, "71"));

	instructionSet.insert(make_pair("AND", map< ADDRESSINGMODES, string>()));
	instructionSet["AND"].insert(make_pair(IMMEDIATE, "29"));
	instructionSet["AND"].insert(make_pair(ZEROPAGE, "25"));
	instructionSet["AND"].insert(make_pair(ZEROPAGEX, "35"));
	instructionSet["AND"].insert(make_pair(ABSOLUTE, "2D"));
	instructionSet["AND"].insert(make_pair(ABSOLUTEX, "3D"));
	instructionSet["AND"].insert(make_pair(ABSOLUTEY, "39"));
	instructionSet["AND"].insert(make_pair(INDIRECTX, "21"));
	instructionSet["AND"].insert(make_pair(INDIRECTY, "31"));

	instructionSet.insert(make_pair("ASL", map< ADDRESSINGMODES, string>()));
	instructionSet["ASL"].insert(make_pair(ACCUMULATOR, "0A"));
	instructionSet["ASL"].insert(make_pair(ZEROPAGE, "06"));
	instructionSet["ASL"].insert(make_pair(ZEROPAGEX, "16"));
	instructionSet["ASL"].insert(make_pair(ABSOLUTE, "0E"));
	instructionSet["ASL"].insert(make_pair(ABSOLUTEX, "1E"));

	instructionSet.insert(make_pair("BCC", map< ADDRESSINGMODES, string>()));
	instructionSet["BCC"].insert(make_pair(RELATIVE, "90"));

	instructionSet.insert(make_pair("BCS", map< ADDRESSINGMODES, string>()));
	instructionSet["BCS"].insert(make_pair(RELATIVE, "B0"));

	instructionSet.insert(make_pair("BEQ", map< ADDRESSINGMODES, string>()));
	instructionSet["BEQ"].insert(make_pair(RELATIVE, "F0"));

	instructionSet.insert(make_pair("BIT", map< ADDRESSINGMODES, string>()));
	instructionSet["BIT"].insert(make_pair(ZEROPAGE, "24"));
	instructionSet["BIT"].insert(make_pair(ABSOLUTE, "2C"));

	instructionSet.insert(make_pair("BMI", map< ADDRESSINGMODES, string>()));
	instructionSet["BMI"].insert(make_pair(RELATIVE, "30"));

	instructionSet.insert(make_pair("BNE", map< ADDRESSINGMODES, string>()));
	instructionSet["BNE"].insert(make_pair(RELATIVE, "D0"));

	instructionSet.insert(make_pair("BPL", map< ADDRESSINGMODES, string>()));
	instructionSet["BPL"].insert(make_pair(RELATIVE, "10"));

	instructionSet.insert(make_pair("BRK", map< ADDRESSINGMODES, string>()));
	instructionSet["BRK"].insert(make_pair(IMPLIED, "00"));

	instructionSet.insert(make_pair("BVC", map< ADDRESSINGMODES, string>()));
	instructionSet["BVC"].insert(make_pair(RELATIVE, "50"));

	instructionSet.insert(make_pair("CLC", map< ADDRESSINGMODES, string>()));
	instructionSet["CLC"].insert(make_pair(IMPLIED, "18"));

	instructionSet.insert(make_pair("CLD", map< ADDRESSINGMODES, string>()));
	instructionSet["CLD"].insert(make_pair(IMPLIED, "D8"));

	instructionSet.insert(make_pair("CLI", map< ADDRESSINGMODES, string>()));
	instructionSet["CLI"].insert(make_pair(IMPLIED, "58"));

	instructionSet.insert(make_pair("CLV", map< ADDRESSINGMODES, string>()));
	instructionSet["CLV"].insert(make_pair(IMPLIED, "B8"));

	instructionSet.insert(make_pair("CMP", map< ADDRESSINGMODES, string>()));
	instructionSet["CMP"].insert(make_pair(IMMEDIATE, "C9"));
	instructionSet["CMP"].insert(make_pair(ZEROPAGE, "C5"));
	instructionSet["CMP"].insert(make_pair(ZEROPAGEX, "D5"));
	instructionSet["CMP"].insert(make_pair(ABSOLUTE, "CD"));
	instructionSet["CMP"].insert(make_pair(ABSOLUTEX, "DD"));
	instructionSet["CMP"].insert(make_pair(ABSOLUTEY, "D9"));
	instructionSet["CMP"].insert(make_pair(INDIRECTX, "C1"));
	instructionSet["CMP"].insert(make_pair(INDIRECTY, "D1"));

	instructionSet.insert(make_pair("CPX", map< ADDRESSINGMODES, string>()));
	instructionSet["CPX"].insert(make_pair(IMMEDIATE, "E0"));
	instructionSet["CPX"].insert(make_pair(ZEROPAGE, "E4"));
	instructionSet["CPX"].insert(make_pair(ABSOLUTE, "EC"));

	instructionSet.insert(make_pair("CPY", map< ADDRESSINGMODES, string>()));
	instructionSet["CPY"].insert(make_pair(IMMEDIATE, "C0"));
	instructionSet["CPY"].insert(make_pair(ZEROPAGE, "C4"));
	instructionSet["CPY"].insert(make_pair(ABSOLUTE, "CC"));

	instructionSet.insert(make_pair("DEC", map< ADDRESSINGMODES, string>()));
	instructionSet["DEC"].insert(make_pair(ZEROPAGE, "C6"));
	instructionSet["DEC"].insert(make_pair(ZEROPAGEX, "D6"));
	instructionSet["DEC"].insert(make_pair(ABSOLUTE, "CE"));
	instructionSet["DEC"].insert(make_pair(ABSOLUTEX, "DE"));

	instructionSet.insert(make_pair("DEX", map< ADDRESSINGMODES, string>()));
	instructionSet["DEX"].insert(make_pair(IMPLIED, "CA"));

	instructionSet.insert(make_pair("DEY", map< ADDRESSINGMODES, string>()));
	instructionSet["DEY"].insert(make_pair(IMPLIED, "88"));

	instructionSet.insert(make_pair("E0R", map< ADDRESSINGMODES, string>()));
	instructionSet["E0R"].insert(make_pair(IMMEDIATE, "49"));
	instructionSet["E0R"].insert(make_pair(ZEROPAGE, "45"));
	instructionSet["E0R"].insert(make_pair(ZEROPAGEX, "55"));
	instructionSet["E0R"].insert(make_pair(ABSOLUTE, "4D"));
	instructionSet["E0R"].insert(make_pair(ABSOLUTEX, "5D"));
	instructionSet["E0R"].insert(make_pair(ABSOLUTEY, "59"));
	instructionSet["E0R"].insert(make_pair(INDIRECTX, "41"));
	instructionSet["E0R"].insert(make_pair(INDIRECTY, "51"));

	instructionSet.insert(make_pair("INC", map< ADDRESSINGMODES, string>()));
	instructionSet["INC"].insert(make_pair(ZEROPAGE, "E6"));
	instructionSet["INC"].insert(make_pair(ZEROPAGEX, "F6"));
	instructionSet["INC"].insert(make_pair(ABSOLUTE, "RE"));
	instructionSet["INC"].insert(make_pair(ABSOLUTEX, "FE"));

	instructionSet.insert(make_pair("INX", map< ADDRESSINGMODES, string>()));
	instructionSet["INX"].insert(make_pair(IMPLIED, "E8"));

	instructionSet.insert(make_pair("INY", map< ADDRESSINGMODES, string>()));
	instructionSet["INY"].insert(make_pair(IMPLIED, "C8"));

	instructionSet.insert(make_pair("JMP", map< ADDRESSINGMODES, string>()));
	instructionSet["JMP"].insert(make_pair(ABSOLUTE, "4C"));
	instructionSet["JMP"].insert(make_pair(INDIRECT, "6C"));

	instructionSet.insert(make_pair("JSR", map< ADDRESSINGMODES, string>()));
	instructionSet["JSR"].insert(make_pair(ABSOLUTE, "20"));

	instructionSet.insert(make_pair("LDA", map< ADDRESSINGMODES, string>()));
	instructionSet["LDA"].insert(make_pair(IMMEDIATE, "A9"));
	instructionSet["LDA"].insert(make_pair(ZEROPAGE, "A5"));
	instructionSet["LDA"].insert(make_pair(ZEROPAGEX, "B5"));
	instructionSet["LDA"].insert(make_pair(ABSOLUTE, "AD"));
	instructionSet["LDA"].insert(make_pair(ABSOLUTEX, "BD"));
	instructionSet["LDA"].insert(make_pair(ABSOLUTEY, "B9"));
	instructionSet["LDA"].insert(make_pair(INDIRECTX, "A1"));
	instructionSet["LDA"].insert(make_pair(INDIRECTY, "B1"));

	instructionSet.insert(make_pair("LDX", map< ADDRESSINGMODES, string>()));
	instructionSet["LDX"].insert(make_pair(IMMEDIATE, "A2"));
	instructionSet["LDX"].insert(make_pair(ZEROPAGE, "A6"));
	instructionSet["LDX"].insert(make_pair(ZEROPAGEY, "B6"));
	instructionSet["LDX"].insert(make_pair(ABSOLUTE, "AE"));
	instructionSet["LDX"].insert(make_pair(ABSOLUTEY, "BE"));

	instructionSet.insert(make_pair("LDY", map< ADDRESSINGMODES, string>()));
	instructionSet["LDY"].insert(make_pair(IMMEDIATE, "A0"));
	instructionSet["LDY"].insert(make_pair(ZEROPAGE, "A4"));
	instructionSet["LDY"].insert(make_pair(ZEROPAGEX, "B4"));
	instructionSet["LDY"].insert(make_pair(ABSOLUTE, "AC"));
	instructionSet["LDY"].insert(make_pair(ABSOLUTEX, "BC"));

	instructionSet.insert(make_pair("LSR", map< ADDRESSINGMODES, string>()));
	instructionSet["LSR"].insert(make_pair(ACCUMULATOR, "4A"));
	instructionSet["LSR"].insert(make_pair(ZEROPAGE, "46"));
	instructionSet["LSR"].insert(make_pair(ZEROPAGEX, "56"));
	instructionSet["LSR"].insert(make_pair(ABSOLUTE, "4E"));
	instructionSet["LSR"].insert(make_pair(ABSOLUTEX, "5E"));

	instructionSet.insert(make_pair("NOP", map< ADDRESSINGMODES, string>()));
	instructionSet["NOP"].insert(make_pair(IMPLIED, "EA"));

	instructionSet.insert(make_pair("ORA", map< ADDRESSINGMODES, string>()));
	instructionSet["ORA"].insert(make_pair(IMMEDIATE, "09"));
	instructionSet["ORA"].insert(make_pair(ZEROPAGE, "05"));
	instructionSet["ORA"].insert(make_pair(ZEROPAGEX, "15"));
	instructionSet["ORA"].insert(make_pair(ABSOLUTE, "0D"));
	instructionSet["ORA"].insert(make_pair(ABSOLUTEX, "1D"));
	instructionSet["ORA"].insert(make_pair(ABSOLUTEY, "19"));
	instructionSet["ORA"].insert(make_pair(INDIRECTX, "01"));
	instructionSet["ORA"].insert(make_pair(INDIRECTY, "11"));

	instructionSet.insert(make_pair("PHA", map< ADDRESSINGMODES, string>()));
	instructionSet["PHA"].insert(make_pair(IMPLIED, "48"));

	instructionSet.insert(make_pair("PHP", map< ADDRESSINGMODES, string>()));
	instructionSet["PHP"].insert(make_pair(IMPLIED, "08"));

	instructionSet.insert(make_pair("PLA", map< ADDRESSINGMODES, string>()));
	instructionSet["PLA"].insert(make_pair(IMPLIED, "68"));
	instructionSet.insert(make_pair("PLP", map< ADDRESSINGMODES, string>()));
	instructionSet["PLP"].insert(make_pair(IMPLIED, "28"));

	instructionSet.insert(make_pair("ROL", map< ADDRESSINGMODES, string>()));
	instructionSet["ROL"].insert(make_pair(ACCUMULATOR, "2A"));
	instructionSet["ROL"].insert(make_pair(ZEROPAGE, "26"));
	instructionSet["ROL"].insert(make_pair(ZEROPAGEX, "36"));
	instructionSet["ROL"].insert(make_pair(ABSOLUTE, "2E"));
	instructionSet["ROL"].insert(make_pair(ABSOLUTEX, "3E"));

	instructionSet.insert(make_pair("ROR", map< ADDRESSINGMODES, string>()));
	instructionSet["ROR"].insert(make_pair(ACCUMULATOR, "6A"));
	instructionSet["ROR"].insert(make_pair(ZEROPAGE, "66"));
	instructionSet["ROR"].insert(make_pair(ZEROPAGEX, "76"));
	instructionSet["ROR"].insert(make_pair(ABSOLUTE, "6E"));
	instructionSet["ROR"].insert(make_pair(ABSOLUTEX, "7E"));

	instructionSet.insert(make_pair("RTI", map< ADDRESSINGMODES, string>()));
	instructionSet["RTI"].insert(make_pair(IMPLIED, "40"));

	instructionSet.insert(make_pair("RTS", map< ADDRESSINGMODES, string>()));
	instructionSet["RTS"].insert(make_pair(IMPLIED, "60"));

	instructionSet.insert(make_pair("SBC", map< ADDRESSINGMODES, string>()));
	instructionSet["SBC"].insert(make_pair(IMMEDIATE, "E9"));
	instructionSet["SBC"].insert(make_pair(ZEROPAGE, "E5"));
	instructionSet["SBC"].insert(make_pair(ZEROPAGEX, "F5"));
	instructionSet["SBC"].insert(make_pair(ABSOLUTE, "ED"));
	instructionSet["SBC"].insert(make_pair(ABSOLUTEX, "FD"));
	instructionSet["SBC"].insert(make_pair(ABSOLUTEY, "F9"));
	instructionSet["SBC"].insert(make_pair(INDIRECTX, "E1"));
	instructionSet["SBC"].insert(make_pair(INDIRECTY, "F1"));

	instructionSet.insert(make_pair("SEC", map< ADDRESSINGMODES, string>()));
	instructionSet["SEC"].insert(make_pair(IMPLIED, "38"));

	instructionSet.insert(make_pair("SED", map< ADDRESSINGMODES, string>()));
	instructionSet["SED"].insert(make_pair(IMPLIED, "F8"));

	instructionSet.insert(make_pair("SEI", map< ADDRESSINGMODES, string>()));
	instructionSet["SEI"].insert(make_pair(IMPLIED, "78"));

	instructionSet.insert(make_pair("STA", map< ADDRESSINGMODES, string>()));
	instructionSet["STA"].insert(make_pair(ZEROPAGE, "85"));
	instructionSet["STA"].insert(make_pair(ZEROPAGEX, "95"));
	instructionSet["STA"].insert(make_pair(ABSOLUTE, "8D"));
	instructionSet["STA"].insert(make_pair(ABSOLUTEX, "9D"));
	instructionSet["STA"].insert(make_pair(ABSOLUTEY, "99"));
	instructionSet["STA"].insert(make_pair(INDIRECTX, "81"));
	instructionSet["STA"].insert(make_pair(INDIRECTY, "91"));

	instructionSet.insert(make_pair("STX", map< ADDRESSINGMODES, string>()));
	instructionSet["STX"].insert(make_pair(ZEROPAGE, "86"));
	instructionSet["STX"].insert(make_pair(ZEROPAGEY, "96"));
	instructionSet["STX"].insert(make_pair(ABSOLUTE, "8E"));

	instructionSet.insert(make_pair("STY", map< ADDRESSINGMODES, string>()));
	instructionSet["STY"].insert(make_pair(ZEROPAGE, "84"));
	instructionSet["STY"].insert(make_pair(ZEROPAGEY, "94"));
	instructionSet["STY"].insert(make_pair(ABSOLUTE, "8C"));

	instructionSet.insert(make_pair("TAX", map< ADDRESSINGMODES, string>()));
	instructionSet["TAX"].insert(make_pair(IMPLIED, "AA"));

	instructionSet.insert(make_pair("TAY", map< ADDRESSINGMODES, string>()));
	instructionSet["TAY"].insert(make_pair(IMPLIED, "A8"));

	instructionSet.insert(make_pair("TSX", map< ADDRESSINGMODES, string>()));
	instructionSet["TSX"].insert(make_pair(IMPLIED, "BA"));

	instructionSet.insert(make_pair("TXA", map< ADDRESSINGMODES, string>()));
	instructionSet["TXA"].insert(make_pair(IMPLIED, "8A"));

	instructionSet.insert(make_pair("TXS", map< ADDRESSINGMODES, string>()));
	instructionSet["TXS"].insert(make_pair(IMPLIED, "9A"));

	instructionSet.insert(make_pair("TYA", map< ADDRESSINGMODES, string>()));
	instructionSet["TYA"].insert(make_pair(IMPLIED, "98"));

	instructionSet.insert(make_pair("CHK", map<ADDRESSINGMODES, string>()));
	instructionSet.insert(make_pair("END", map<ADDRESSINGMODES, string>()));
	instructionSet.insert(make_pair("ORG", map<ADDRESSINGMODES, string>()));
	instructionSet.insert(make_pair("EQU", map<ADDRESSINGMODES, string>()));
	return instructionSet;
}


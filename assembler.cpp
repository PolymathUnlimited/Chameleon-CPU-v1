/*

An assembler for the Chameleon ISA

Instruction set:
	0000xxxx: NOP - No operation (do nothing)
	0001xxxx: ALM - ALU operation xxxx from memory
	0010xxxx: ALA - ALU operation xxxx from accumulator
	0011xxxx: ALI - ALU operation xxxx from immediate value
	0100xxxx: ALS - ALU operation xxxx from stack
	0101xxxx: LOD - Load accumulator from memory
	0110xxxx: LDI - Load accumulator from immediate value
	0111xxxx: STO - Store accumulator to memory
	1000xxxx: PSH - Push accumulator to stack
	1001xxxx: POP - Pop stack to accumulator
	1010xxxx: JMP - Jump to instruction
	1011xxxx: BR  - Jump if condition xxxx is met
	1100xxxx: BN  - Jump if condition xxxx is not met
	1101xxxx: JSR - Jump to subroutine
	1110xxxx: RSR - Return from subroutine
	1111xxxx: HLT - Halt

ALU operations:
	0000: ADD - addition
	0001: ADC - addition with carry from flags register
	0010: SUB - subtraction
	0011: SBB - subtraction with borrow from flags register
	0100: ONC - one's complement (inversion)
	0101: TWC - two's complement (negation)
	0110: AND - logical and
	0111: OR  - logical or
	1000: XOR - logical exclusive or
	1001: LSL - logical left-shift
	1010: LSR - logical right-shift
	1011: ASR - arithmetic right-shift
	1100: ROL - rotate left
	1101: ROR - rotate right
	1110: RCL - rotate left through carry
	1111: RCR - rotate right through carry

ALU flags: CZNV (for conditions)
	C - Carry Flag
	Z - Zero Flag
	N - Negative Flag
	V - Overflow Flag
*/

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <unordered_map>

using namespace std;

bool isInstruction(string str)
{
	if (str == "NOP") return true;
	if (str == "ADD") return true;
	if (str == "ADC") return true;
	if (str == "SUB") return true;
	if (str == "SBB") return true;
	if (str == "ONC") return true;
	if (str == "TWC") return true;
	if (str == "AND") return true;
	if (str == "OR") return true;
	if (str == "XOR") return true;
	if (str == "LSL") return true;
	if (str == "LSR") return true;
	if (str == "ASR") return true;
	if (str == "ROL") return true;
	if (str == "ROR") return true;
	if (str == "RDL") return true;
	if (str == "RCR") return true;
	if (str == "LOD") return true;
	if (str == "STO") return true;
	if (str == "PSH") return true;
	if (str == "POP") return true;
	if (str == "JMP") return true;
	if (str == "BRC") return true;
	if (str == "BRZ") return true;
	if (str == "BRN") return true;
	if (str == "BRV") return true;
	if (str == "BNC") return true;
	if (str == "BNZ") return true;
	if (str == "BNN") return true;
	if (str == "BNV") return true;
	if (str == "JSR") return true;
	if (str == "RSR") return true;
	if (str == "HLT") return true;
	return false;
}

bool isDirective(string str)
{
	if (str == ".reserve") return true;
	if (str == ".org") return true;
	if (str == ".byte") return true;
	if (str == ".string") return true;
	if (str == ".data") return true;
	return false;
}

bool isInteger(string str)
{
	// if the number is negative, then ignore the minus sign
	if (str[0] == '-') str.erase(0, 1);

	// hexadecimal numbers
	if (str.length() >= 3 && str[0] == '0' && (str[1] == 'X' || str[1] == 'x'))
		for (int i = 2; i < str.length(); ++i)
		{
			if ((str[i] < '0' || str[i] > '9') && (str[i] < 'a' || str[i] > 'f') && (str[i] < 'A' || str[i] > 'F')) return false;
		}
	// binary numbers
	else if (str.length() >= 3 && str[0] == '0' && (str[1] == 'B' || str[1] == 'b'))
		for (int i = 2; i < str.length(); ++i)
		{
			if (str[i] != '0' && str[i] != '1') return false;
		}
	else // decimal numbers
		for (int i = 0; i < str.length(); ++i)
		{
			if (str[i] < '0' || str[i] > '9') return false;
		}

	return true;
}

int integer(string str)
{
	// negative numbers
	bool negative = false;
	if (str[0] == '-')
	{
		negative = true;
		str.erase(0, 1);
	}

	// calculate the number
	int number = 0;
	// hexadecimal numbers
	if (str.length() >= 3 && str[0] == '0' && (str[1] == 'X' || str[1] == 'x'))
	{
		for (int i = 2; i < str.length(); ++i)
		{
			char charDigit = str[i];
			int digit = charDigit - '0';
			if (charDigit >= 'a' && charDigit <= 'f') digit = charDigit - 'a' + 10;
			if (charDigit >= 'A' && charDigit <= 'F') digit = charDigit - 'F' + 10;
			number = (number << 4) + digit;
		}
	}
	// binary numbers
	else if (str.length() >= 3 && str[0] == '0' && (str[1] == 'B' || str[1] == 'b'))
	{
		for (int i = 2; i < str.length(); ++i)
		{
			int bit = str[i] - '0';
			number = (number << 1) + bit;
		}
	}
	else //decimal numbers
	{
		for (int i = 0; i < str.length(); ++i)
		{
			int digit = str[i] - '0';
			number = number * 10 + digit;
		}
	}

	if (negative) return -number;
	return number;
}

string to_hex(string str)
{
	if (str.substr(0, 2) == "0x")
	{
		str.erase(0, 2);
		return str;
	}
	else if (str.substr(0, 2) == "0b")
	{
		str.erase(0, 2);
		while (str.size() % 4) str = "0" + str;

		string result = "";
		while (str.size())
		{
			string hexDigit = str.substr(0, 4);
			str.erase(0, 4);

			char digit = integer("0b" + hexDigit);

			digit += (digit > 10) ? 'A' - 10 : '0';

			result += digit;
		}

		return result;
	}
	else
	{
		string result = "";

		while (str.size())
		{
			int remainder = 0;
			string quotient = "";

			for (char digit : str)
			{
				int current = remainder * 10 + digit - '0';
				quotient += (current / 16) + '0';
				remainder = current % 16;
			}

			while (quotient.size() && quotient[0] == '0') quotient.erase(0, 1);

			char hexChar = (remainder > 10) ? remainder + 'A' - 10 : remainder + '0';

			result = hexChar + result;

			str = quotient;
		}

		return result;
	}
}

string to_immediate(string str)
{
	if (str[0] == '!') str.erase(0, 1);
	int value = integer(str);
	int a = value / 16;
	int b = value % 16;

	a = (a < 10) ? a + '0' : a - 10 + 'A';
	b = (b < 10) ? b + '0' : b - 10 + 'A';

	string immediate = "";

	immediate.push_back(a);
	immediate.push_back(b);

	return immediate;
}

string to_address(string str)
{
	int value = integer(str);

	int d = value % 16;
	value /= 16;
	int c = value % 16;
	value /= 16;
	int b = value % 16;
	value /= 16;
	int a = value % 16;

	a = (a < 10) ? a + '0' : a - 10 + 'A';
	b = (b < 10) ? b + '0' : b - 10 + 'A';
	c = (c < 10) ? c + '0' : c - 10 + 'A';
	d = (d < 10) ? d + '0' : d - 10 + 'A';

	string address = "";

	// little-endian, hence the order of terms
	address.push_back(c);
	address.push_back(d);
	address.push_back(a);
	address.push_back(b);

	return address;
}

string loadFile(string filename)
{
	ifstream fin(filename);
	string line;
	string code = "";
	while(getline(fin, line)) code += line + "\n";
	fin.close();

	return code;
}

int numPreceedingBackslashes(string str, int index)
{
	int n = 0;
	for (int i = index - 1; i >= 0 && str[i] == '\\'; --i) ++n;
	return n;
}

string assemble(string& asmCode)
{
	if (asmCode.empty()) return "";

	// DEBUG: output initial code file
	cout << endl << "ASSEMBLY CODE: " << endl << endl;
	cout << asmCode << endl << endl;

	// remove single-line coments
	for (int i = 0; i < asmCode.length() - 1; ++i)
		if (asmCode[i] == '/' && asmCode[i + 1] == '/')
			while (i < asmCode.size() && asmCode[i] != '\n' && asmCode[i] != '\0')
				asmCode.erase(i, 1);

	// remove block comments
	for (int i = 0; i < asmCode.length() - 1; ++i)
		if (asmCode[i] == '/' && asmCode[i + 1] == '*')
			while (i < asmCode.size() - 1 && (asmCode[i] != '*' || asmCode[i] != '/'))
				asmCode.erase(i, 1);

	// remove unneccessary whitespace
	bool inQuotes = false;
	for (int i = 0; i < asmCode.length(); ++i)
	{
		if (asmCode[i] == '"' && !(numPreceedingBackslashes(asmCode, i) % 2)) inQuotes = !inQuotes;
		if (inQuotes) continue;
		if (asmCode[i] == '\n' || asmCode[i] == '\t') asmCode[i] = ' ';
	}
	inQuotes = false;
	for (int i = 0; i < asmCode.length() - 1; ++i)
	{
		if (asmCode[i] == '"' && !(numPreceedingBackslashes(asmCode, i) % 2)) inQuotes = !inQuotes;
		if (inQuotes) continue;
		if (asmCode[i] == ' ') while (asmCode[i + 1] == ' ') asmCode.erase(i + 1, 1);
	}

	// make sure all mathematical symbols are separated by a space
	inQuotes = false;
	for (int i = 1; i < asmCode.length() - 1; ++i)
	{
		if (asmCode[i] == '"' && !(numPreceedingBackslashes(asmCode, i) % 2)) inQuotes = !inQuotes;
		if (inQuotes) continue;
		if (asmCode[i] == ':' || asmCode[i] == '=' || asmCode[i] == '+' || asmCode[i] == '-' || asmCode[i] == '*' || asmCode[i] == '/' || asmCode[i] == '(' || asmCode[i] == ')' || asmCode[i] == '%')
		{
			if (asmCode[i + 1] != ' ') asmCode.insert(i + 1, " ");
			if (asmCode[i - 1] != ' ') asmCode.insert(i, " ");
		}
	}

	// generate symbols
	cout << "FORMATTED CODE:" << endl << endl;
	cout << asmCode << endl << endl;
	vector<string> symbols;
	inQuotes = false;
	for (int i = 0; i < asmCode.length(); ++i)
	{
		if (asmCode[i] == '"' && !(numPreceedingBackslashes(asmCode, i) % 2)) inQuotes = !inQuotes;
		if (inQuotes)
		{
			string symbol = "";
			for (int index = i; i < asmCode.length(); ++index)
			{
				symbol += asmCode[index];
				++i;
				if (asmCode[i] == '"' && !(numPreceedingBackslashes(asmCode, i) % 2)) break;
			}
			symbol += "\"";
			inQuotes = false;
			symbols.push_back(symbol);
			++i;
		}
		if (asmCode[i] != ' ')
		{
			string symbol = "";
			while (asmCode[i] != ' ') symbol += asmCode.substr(i++, 1);
			symbols.push_back(symbol);
		}
	}

	// add leading zeros for unitary operators
	for (int i = 0; i < symbols.size(); ++i)
	{
		if (symbols[i] == "+" || symbols[i] == "-")
		{
			if (i == 0 || symbols[i - 1] == "(" || symbols[i - 1] == "=" || isInstruction(symbols[i - 1]) || isDirective(symbols[i - 1]))
			{
				symbols.insert(symbols.begin() + i, "0");
				++i;
			}
		}
	}

	// place parentheses to enforce order of operations
	for (int i = 0; i < symbols.size(); ++i)
	{
		if (symbols[i] == "*" || symbols[i] == "/" || symbols[i] == "%")
		{
			// place left parenthese
			int parnum = 0;
			for (int index = i - 1; index >= 0; --index)
			{
				if (symbols[index] == ")") ++parnum;
				else if (symbols[index] == "(") --parnum;
				if (parnum == 0)
				{
					symbols.insert(symbols.begin() + index, "(");
					++i;
					break;
				}
			}
			// place right parenthese
			parnum = 0;
			for (int index = i + 1; index < symbols.size(); ++index)
			{
				if (symbols[index] == "(") ++parnum;
				else if (symbols[index] == ")") --parnum;
				if (parnum == 0)
				{
					symbols.insert(symbols.begin() + index + 1, ")");
					break;
				}
			}
		}
	}
	for (int i = 0; i < symbols.size(); ++i)
	{
		if (symbols[i] == "+" || symbols[i] == "-")
		{
			// place left parenthese
			int parnum = 0;
			for (int index = i - 1; index >= 0; --index)
			{
				if (symbols[index] == ")") ++parnum;
				else if (symbols[index] == "(") --parnum;
				if (parnum == 0)
				{
					symbols.insert(symbols.begin() + index, "(");
					++i;
					break;
				}
			}
			// place right parenthese
			parnum = 0;
			for (int index = i + 1; index < symbols.size(); ++index)
			{
				if (symbols[index] == "(") ++parnum;
				else if (symbols[index] == ")") --parnum;
				if (parnum == 0)
				{
					symbols.insert(symbols.begin() + index + 1, ")");
					break;
				}
			}
		}
	}

	// DEBUG: output symbols
	cout << "SYMBOLS:" << endl << endl;
	for (int i = 0; i < symbols.size(); ++i) cout << symbols[i] << endl;
	cout << endl;

	// generate tags
	unordered_map<string, int> tags;

	// directly defined tags
	int address = 0;
	for (int i = 0; i < symbols.size(); ++i)
	{
		// tag definitions
		if (symbols[i] == ":" && i)
		{
			tags[symbols[i - 1]] = address;
			symbols.erase(symbols.begin() + i, symbols.begin() + i + 1);
			symbols.erase(symbols.begin() + i - 1, symbols.begin() + i);
			--i;
			cout << "ADDRESS ADDED TO TAG: " << address << endl;
		}
		if (i >= symbols.size() - 1) break;
		// update the address based on instruction byte-size
		if (symbols[i] == "NOP" || symbols[i] == "PSH" || symbols[i] == "POP" || symbols[i] == "RSR") ++address;
		if (symbols[i] == "ADD" || symbols[i] == "ADC" || symbols[i] == "SUB" || symbols[i] == "SBB" ||
			symbols[i] == "ONC" || symbols[i] == "TWC" || symbols[i] == "AND" || symbols[i] == "OR" ||
			symbols[i] == "XOR" || symbols[i] == "LSL" || symbols[i] == "LSR" || symbols[i] == "ASR" ||
			symbols[i] == "ROL" || symbols[i] == "ROR" || symbols[i] == "RCL" || symbols[i] == "RCR")
		{
			// immediate operand
			if (symbols[i + 1][0] == '!') address += 2;
			// accumulator operand
			else if (symbols[i + 1] == "#reg_a") ++address;
			// stack operand
			else if (symbols[i + 1] == "#stack") ++address;
			// address operand
			else address += 3;
		}
		if (symbols[i] == "LOD")
		{
			// immediate operand
			if (symbols[i + 1][0] == '!') address += 2;
			// stack operand
			else if (symbols[i + 1] == "#stack") ++address;
			// address operand
			else address += 3;
		}
		if (symbols[i] == "STO")
		{
			// stack operand
			if (symbols[i + 1] == "#stack") ++address;
			// address operand
			else address += 3;
		}
		if (symbols[i] == "JMP" || symbols[i] == "JSR" || symbols[i] == "BRC" || symbols[i] == "BRZ" || symbols[i] == "BRN" ||
			symbols[i] == "BRV" || symbols[i] == "BNC" || symbols[i] == "BNZ" || symbols[i] == "BNN" || symbols[i] == "BNV")
			address += 3;
		if (symbols[i] == "HLT") ++address;
		if (symbols[i] == ".reserve" && i < symbols.size() - 1)
		{
			if (!isInteger(symbols[i + 1]))
			{
				cout << "ERROR: .reserve " << symbols[i + 1] << " is not a valid directive!" << endl;
				return "";
			}
			address += integer(symbols[i + 1]);
		}
		if (symbols[i] == ".org" && i < symbols.size() - 1)
		{
			if (!isInteger(symbols[i + 1]))
			{
				cout << "ERROR: .org " << symbols[i + 1] << " is not a valid directive!" << endl;
				return "";
			}
			address = integer(symbols[i + 1]);
		}
		if (symbols[i] == ".byte" && i < symbols.size() - 1)
			++address;
		if (symbols[i] == ".string" && i < symbols.size() - 1 && symbols[i + 1][0] == '"' && symbols[i + 1].back() == '"')
		{
			int length = symbols[i + 1].length() - 1;
			for (int j = 0; j < symbols[i + 1].length(); ++j) if (symbols[i + 1][j] == '\\') --length;
			address += length;
		}
		if (symbols[i] == ".data" && i < symbols.size() - 1 && isInteger(symbols[i + 1]))
		{
			string data = to_hex(symbols[i + 1]);
			int dataSize = data.size();
			if (dataSize % 2) ++dataSize;
			address += dataSize / 2;
		}
	}

	// indirectly defined tags
	int numUndefined = 1;
	bool progress = false;
	while (numUndefined)
	{
		numUndefined = 0;
		progress = false;
		// define tags with a valid definition
		for (int i = 1; i < symbols.size() - 1; ++i)
		{
			if (symbols[i] == "=")
			{
				if (isInteger(symbols[i + 1]) && (i >= symbols.size() - 2 || (symbols[i + 2] != "=" && symbols[i + 2] != "+" &&
					symbols[i + 2] != "-" && symbols[i + 2] != "*" && symbols[i + 2] != "/" && symbols[i + 2] != "%")))
				{
					tags[symbols[i - 1]] = integer(symbols[i + 1]);
					symbols.erase(symbols.begin() + i - 1, symbols.begin() + i + 2);
					--i;
					progress = true;
				}
				else ++numUndefined;
			}
		}

		// replace defined tags
		for (int i = 0; i < symbols.size(); ++i)
		{
			if (tags.find(symbols[i]) != tags.end())
			{
				symbols[i] = to_string(tags[symbols[i]]);
				progress = true;
			}
		}

		// evaluate expressions
		for (int i = 0; i < symbols.size(); ++i)
		{
			if (symbols[i] == "*" && isInteger(symbols[i - 1]) && isInteger(symbols[i + 1]))
			{
				symbols[i] = to_string(integer(symbols[i - 1]) * integer(symbols[i + 1]));
				symbols.erase(symbols.begin() + i + 1);
				symbols.erase(symbols.begin() + i - 1);
				--i;
				progress = true;
			}
			if (symbols[i] == "/" && isInteger(symbols[i - 1]) && isInteger(symbols[i + 1]))
			{
				symbols[i] = to_string(integer(symbols[i - 1]) / integer(symbols[i + 1]));
				symbols.erase(symbols.begin() + i + 1);
				symbols.erase(symbols.begin() + i - 1);
				--i;
				progress = true;
			}
			if (symbols[i] == "%" && isInteger(symbols[i - 1]) && isInteger(symbols[i + 1]))
			{
				symbols[i] = to_string(integer(symbols[i - 1]) % integer(symbols[i + 1]));
				symbols.erase(symbols.begin() + i + 1);
				symbols.erase(symbols.begin() + i - 1);
				--i;
				progress = true;
			}
			if (symbols[i] == "+" && isInteger(symbols[i - 1]) && isInteger(symbols[i + 1]))
			{
				symbols[i] = to_string(integer(symbols[i - 1]) + integer(symbols[i + 1]));
				symbols.erase(symbols.begin() + i + 1);
				symbols.erase(symbols.begin() + i - 1);
				--i;
				progress = true;
			}
			if (symbols[i] == "-" && isInteger(symbols[i - 1]) && isInteger(symbols[i + 1]))
			{
				symbols[i] = to_string(integer(symbols[i - 1]) - integer(symbols[i + 1]));
				symbols.erase(symbols.begin() + i + 1);
				symbols.erase(symbols.begin() + i - 1);
				--i;
				progress = true;
			}
			if (isInteger(symbols[i]) && i > 0 && symbols[i - 1] == "(" && i < symbols.size() - 1 && symbols[i + 1] == ")")
			{
				symbols.erase(symbols.begin() + i + 1);
				symbols.erase(symbols.begin() + i - 1);
				--i;
				progress = true;
			}
		}

		if (!progress && numUndefined)
		{
			cout << "ERROR: " << numUndefined << " undefined tag" << (numUndefined > 1 ? "s" : "") << "!" << endl;
			for (int i = 0; i < symbols.size(); ++i)
			{
				if (symbols[i] == "=") cout << "\t" << symbols[i - 1] << endl;
			}
			cout << endl;
			cout << "CODE TO THIS POINT" << endl << endl;
			for (int i = 0; i < symbols.size(); ++i)
			{
				cout << symbols[i] << endl;
			}
			return "";
		}
	}

	// DEBUG: output all tag definitions

	cout << "TAG DEFINITIONS:" << endl << endl;
	for (auto it = tags.begin(); it != tags.end(); ++it)
		cout << "\ttag: " << it->first << "\tdef: " << it->second << endl;
	cout << endl;

	// remove any extra parentheses
	for (int i = 0; i < symbols.size(); ++i)
	{
		if (symbols[i] == "(" || symbols[i] == ")")
		{
			symbols.erase(symbols.begin() + i, symbols.begin() + i + 1);
		}
	}

	// reattach any hanging immediate operators
	for (int i = 0; i < symbols.size(); ++i)
	{
		if (symbols[i] == "!" && i < symbols.size() - 1)
		{
			symbols[i] += symbols[i + 1];
			symbols.erase(symbols.begin() + i + 1, symbols.begin() + i + 2);
		}
	}

	// DEBUG: output code for assembly
	cout << "CODE FOR ASSEMBLY: " << endl;
	for (int i = 0; i < symbols.size(); ++i)
	{
		if (isInstruction(symbols[i]) || isDirective(symbols[i])) cout << endl;
		cout << symbols[i] << "\t";
	}
	cout << endl << endl;

	// assemble the machine code
	address = 0;
	string machineCode = "";
	for (int i = 0; i < symbols.size(); ++i)
	{
		if (symbols[i] == "NOP")
		{
			machineCode += "00";
			++address;
		}
		else if (symbols[i] == "ADD")
		{
			if (i == symbols.size() - 1 || symbols[i + 1] == "#a_reg")
			{
				machineCode += "20";
				++address;
			}
			else if (i < symbols.size() - 1 && symbols[i + 1] == "#stack")
			{
				machineCode += "40";
				++address;
			}
			else if (i < symbols.size() && symbols[i + 1][0] == '!')
			{
				machineCode += "30" + to_immediate(symbols[i + 1]);
				address += 2;
			}
			else if (i < symbols.size() - 1 && isInteger(symbols[i + 1]))
			{
				machineCode += "10" + to_address(symbols[i + 1]);
				address += 3;
			}
		}
		else if (symbols[i] == "ADC")
		{
			if (i == symbols.size() - 1 || symbols[i + 1] == "#a_reg")
			{
				machineCode += "21";
				++address;
			}
			else if (i < symbols.size() - 1 && symbols[i + 1] == "#stack")
			{
				machineCode += "41";
				++address;
			}
			else if (i < symbols.size() && symbols[i + 1][0] == '!')
			{
				machineCode += "31" + to_immediate(symbols[i + 1]);
				address += 2;
			}
			else if (i < symbols.size() - 1 && isInteger(symbols[i + 1]))
			{
				machineCode += "11" + to_address(symbols[i + 1]);
				address += 3;
			}
		}
		else if (symbols[i] == "SUB")
		{
			if (i == symbols.size() - 1 || symbols[i + 1] == "#a_reg")
			{
				machineCode += "22";
				++address;
			}
			else if (i < symbols.size() - 1 && symbols[i + 1] == "#stack")
			{
				machineCode += "42";
				++address;
			}
			else if (i < symbols.size() && symbols[i + 1][0] == '!')
			{
				machineCode += "32" + to_immediate(symbols[i + 1]);
				address += 2;
			}
			else if (i < symbols.size() - 1 && isInteger(symbols[i + 1]))
			{
				machineCode += "12" + to_address(symbols[i + 1]);
				address += 3;
			}
		}
		else if (symbols[i] == "SBB")
		{
			if (i == symbols.size() - 1 || symbols[i + 1] == "#a_reg")
			{
				machineCode += "23";
				++address;
			}
			else if (i < symbols.size() - 1 && symbols[i + 1] == "#stack")
			{
				machineCode += "43";
				++address;
			}
			else if (i < symbols.size() && symbols[i + 1][0] == '!')
			{
				machineCode += "33" + to_immediate(symbols[i + 1]);
				address += 2;
			}
			else if (i < symbols.size() - 1 && isInteger(symbols[i + 1]))
			{
				machineCode += "13" + to_address(symbols[i + 1]);
				address += 3;
			}
		}
		else if (symbols[i] == "ONC")
		{
			if (i == symbols.size() - 1 || symbols[i + 1] == "#a_reg")
			{
				machineCode += "24";
				++address;
			}
			else if (i < symbols.size() - 1 && symbols[i + 1] == "#stack")
			{
				machineCode += "44";
				++address;
			}
			else if (i < symbols.size() && symbols[i + 1][0] == '!')
			{
				machineCode += "34" + to_immediate(symbols[i + 1]);
				address += 2;
			}
			else if (i < symbols.size() - 1 && isInteger(symbols[i + 1]))
			{
				machineCode += "14" + to_address(symbols[i + 1]);
				address += 3;
			}
		}
		else if (symbols[i] == "TWC")
		{
			if (i == symbols.size() - 1 || symbols[i + 1] == "#a_reg")
			{
				machineCode += "25";
				++address;
			}
			else if (i < symbols.size() - 1 && symbols[i + 1] == "#stack")
			{
				machineCode += "45";
				++address;
			}
			else if (i < symbols.size() && symbols[i + 1][0] == '!')
			{
				machineCode += "35" + to_immediate(symbols[i + 1]);
				address += 2;
			}
			else if (i < symbols.size() - 1 && isInteger(symbols[i + 1]))
			{
				machineCode += "15" + to_address(symbols[i + 1]);
				address += 3;
			}
		}
		else if (symbols[i] == "AND")
		{
			if (i == symbols.size() - 1 || symbols[i + 1] == "#a_reg")
			{
				machineCode += "26";
				++address;
			}
			else if (i < symbols.size() - 1 && symbols[i + 1] == "#stack")
			{
				machineCode += "46";
				++address;
			}
			else if (i < symbols.size() && symbols[i + 1][0] == '!')
			{
				machineCode += "36" + to_immediate(symbols[i + 1]);
				address += 2;
			}
			else if (i < symbols.size() - 1 && isInteger(symbols[i + 1]))
			{
				machineCode += "16" + to_address(symbols[i + 1]);
				address += 3;
			}
		}
		else if (symbols[i] == "OR")
		{
			if (i == symbols.size() - 1 || symbols[i + 1] == "#a_reg")
			{
				machineCode += "27";
				++address;
			}
			else if (i < symbols.size() - 1 && symbols[i + 1] == "#stack")
			{
				machineCode += "47";
				++address;
			}
			else if (i < symbols.size() && symbols[i + 1][0] == '!')
			{
				machineCode += "37" + to_immediate(symbols[i + 1]);
				address += 2;
			}
			else if (i < symbols.size() - 1 && isInteger(symbols[i + 1]))
			{
				machineCode += "17" + to_address(symbols[i + 1]);
				address += 3;
			}
		}
		else if (symbols[i] == "XOR")
		{
			if (i == symbols.size() - 1 || symbols[i + 1] == "#a_reg")
			{
				machineCode += "28";
				++address;
			}
			else if (i < symbols.size() - 1 && symbols[i + 1] == "#stack")
			{
				machineCode += "48";
				++address;
			}
			else if (i < symbols.size() && symbols[i + 1][0] == '!')
			{
				machineCode += "38" + to_immediate(symbols[i + 1]);
				address += 2;
			}
			else if (i < symbols.size() - 1 && isInteger(symbols[i + 1]))
			{
				machineCode += "18" + to_address(symbols[i + 1]);
				address += 3;
			}
		}
		else if (symbols[i] == "LSL")
		{
			if (i == symbols.size() - 1 || symbols[i + 1] == "#a_reg")
			{
				machineCode += "29";
				++address;
			}
			else if (i < symbols.size() - 1 && symbols[i + 1] == "#stack")
			{
				machineCode += "49";
				++address;
			}
			else if (i < symbols.size() && symbols[i + 1][0] == '!')
			{
				machineCode += "39" + to_immediate(symbols[i + 1]);
				address += 2;
			}
			else if (i < symbols.size() - 1 && isInteger(symbols[i + 1]))
			{
				machineCode += "19" + to_address(symbols[i + 1]);
				address += 3;
			}
		}
		else if (symbols[i] == "LSR")
		{
			if (i == symbols.size() - 1 || symbols[i + 1] == "#a_reg")
			{
				machineCode += "2a";
				++address;
			}
			else if (i < symbols.size() - 1 && symbols[i + 1] == "#stack")
			{
				machineCode += "4a";
				++address;
			}
			else if (i < symbols.size() && symbols[i + 1][0] == '!')
			{
				machineCode += "3a" + to_immediate(symbols[i + 1]);
				address += 2;
			}
			else if (i < symbols.size() - 1 && isInteger(symbols[i + 1]))
			{
				machineCode += "1a" + to_address(symbols[i + 1]);
				address += 3;
			}
		}
		else if (symbols[i] == "ROL")
		{
			if (i == symbols.size() - 1 || symbols[i + 1] == "#a_reg")
			{
				machineCode += "2b";
				++address;
			}
			else if (i < symbols.size() - 1 && symbols[i + 1] == "#stack")
			{
				machineCode += "4b";
				++address;
			}
			else if (i < symbols.size() && symbols[i + 1][0] == '!')
			{
				machineCode += "3b" + to_immediate(symbols[i + 1]);
				address += 2;
			}
			else if (i < symbols.size() - 1 && isInteger(symbols[i + 1]))
			{
				machineCode += "1b" + to_address(symbols[i + 1]);
				address += 3;
			}
		}
		else if (symbols[i] == "ROR")
		{
			if (i == symbols.size() - 1 || symbols[i + 1] == "#a_reg")
			{
				machineCode += "2c";
				++address;
			}
			else if (i < symbols.size() - 1 && symbols[i + 1] == "#stack")
			{
				machineCode += "4c";
				++address;
			}
			else if (i < symbols.size() && symbols[i + 1][0] == '!')
			{
				machineCode += "3c" + to_immediate(symbols[i + 1]);
				address += 2;
			}
			else if (i < symbols.size() - 1 && isInteger(symbols[i + 1]))
			{
				machineCode += "1c" + to_address(symbols[i + 1]);
				address += 3;
			}
		}
		else if (symbols[i] == "RCL")
		{
			if (i == symbols.size() - 1 || symbols[i + 1] == "#a_reg")
			{
				machineCode += "2d";
				++address;
			}
			else if (i < symbols.size() - 1 && symbols[i + 1] == "#stack")
			{
				machineCode += "4d";
				++address;
			}
			else if (i < symbols.size() && symbols[i + 1][0] == '!')
			{
				machineCode += "3d" + to_immediate(symbols[i + 1]);
				address += 2;
			}
			else if (i < symbols.size() - 1 && isInteger(symbols[i + 1]))
			{
				machineCode += "1d" + to_address(symbols[i + 1]);
				address += 3;
			}
		}
		else if (symbols[i] == "ROR")
		{
			if (i == symbols.size() - 1 || symbols[i + 1] == "#a_reg")
			{
				machineCode += "2f";
				++address;
			}
			else if (i < symbols.size() - 1 && symbols[i + 1] == "#stack")
			{
				machineCode += "4f";
				++address;
			}
			else if (i < symbols.size() && symbols[i + 1][0] == '!')
			{
				machineCode += "3f" + to_immediate(symbols[i + 1]);
				address += 2;
			}
			else if (i < symbols.size() - 1 && isInteger(symbols[i + 1]))
			{
				machineCode += "1f" + to_address(symbols[i + 1]);
				address += 3;
			}
		}
		else if (symbols[i] == "LOD")
		{
			if (i < symbols.size() - 1 && symbols[i + 1] == "#stack")
			{
				machineCode += "90";
				++address;
			}
			else if (i < symbols.size() && symbols[i + 1][0] == '!')
			{
				machineCode += "60" + to_immediate(symbols[i + 1]);
				address += 2;
			}
			else if (i < symbols.size() - 1 && isInteger(symbols[i + 1]))
			{
				machineCode += "50" + to_address(symbols[i + 1]);
				address += 3;
			}
		}
		else if (symbols[i] == "STO")
		{
			if (i < symbols.size() - 1 && symbols[i + 1] == "#stack")
			{
				machineCode += "80";
				++address;
			}
			else if (i < symbols.size() - 1 && isInteger(symbols[i + 1]))
			{
				machineCode += "70" + to_address(symbols[i + 1]);
				address += 3;
			}
		}
		else if (symbols[i] == "PSH")
		{
			machineCode += "80";
			++address;
		}
		else if (symbols[i] == "POP")
		{
			machineCode += "90";
			++address;
		}
		else if (symbols[i] == "JMP" && i < symbols.size() - 1)
		{
			machineCode += "a0" + to_address(symbols[i + 1]);
			address += 3;
		}
		else if (symbols[i] == "BRC" && i < symbols.size() - 1)
		{
			machineCode += "b1" + to_address(symbols[i + 1]);
			address += 3;
		}
		else if (symbols[i] == "BRZ" && i < symbols.size() - 1)
		{
			machineCode += "b2" + to_address(symbols[i + 1]);
			address += 3;
		}
		else if (symbols[i] == "BRN" && i < symbols.size() - 1)
		{
			machineCode += "b4" + to_address(symbols[i + 1]);
			address += 3;
		}
		else if (symbols[i] == "BRV" && i < symbols.size() - 1)
		{
			machineCode += "b8" + to_address(symbols[i + 1]);
			address += 3;
		}
		else if (symbols[i] == "BNC" && i < symbols.size() - 1)
		{
			machineCode += "c1" + to_address(symbols[i + 1]);
			address += 3;
		}
		else if (symbols[i] == "BNZ" && i < symbols.size() - 1)
		{
			machineCode += "c2" + to_address(symbols[i + 1]);
			address += 3;
		}
		else if (symbols[i] == "BNN" && i < symbols.size() - 1)
		{
			machineCode += "c4" + to_address(symbols[i + 1]);
			address += 3;
		}
		else if (symbols[i] == "BNV" && i < symbols.size() - 1)
		{
			machineCode += "c8" + to_address(symbols[i + 1]);
			address += 3;
		}
		else if (symbols[i] == "JSR" && i < symbols.size() - 1)
		{
			machineCode += "d0" + to_address(symbols[i + 1]);
			address += 3;
		}
		else if (symbols[i] == "RSR")
		{
			machineCode += "e0";
			++address;
		}
		else if (symbols[i] == "HLT")
		{
			machineCode += "ff";
			++address;
		}

		// memory directives
		else if (symbols[i] == ".reserve" && i < symbols.size() - 1 && isInteger(symbols[i + 1]))
		{
			int numBytes = integer(symbols[i + 1]);
			for (int i = 0; i < numBytes; ++i)
			{
				machineCode += "00";
				++address;
			}
		}
		else if (symbols[i] == ".org" && i < symbols.size() - 1 && isInteger(symbols[i + 1]))
		{
			int newAddress = integer(symbols[i + 1]);
			if (newAddress < address)
			{
				cout << "ERROR: .org " << symbols[i + 1] << " is invalid because it would overwrite previous data!" << endl;
				return "";
			}
			while (address != newAddress)
			{
				machineCode += "00";
				++address;
			}
		}
		else if (symbols[i] == ".byte" && i < symbols.size() - 1 && isInteger(symbols[i + 1]))
		{
			machineCode += to_immediate(symbols[i + 1]);
			++address;
		}
		else if (symbols[i] == ".string" && i < symbols.size() - 1 && symbols[i + 1][0] == '"' && symbols[i + 1].back() == '"')
		{
			string str = symbols[i + 1];
			str.erase(0, 1);
			str.pop_back();

			for (int i = 0; i < str.length(); ++i)
			{
				if (str[i] == '\\' && i < str.length() - 1)
				{
					str.erase(i, 1);
					if (str[i] == 'a') str[i] = '\a';
					if (str[i] == 'b') str[i] = '\b';
					if (str[i] == 'f') str[i] = '\f';
					if (str[i] == 'n') str[i] = '\n';
					if (str[i] == 'r') str[i] = '\r';
					if (str[i] == 't') str[i] = '\t';
					if (str[i] == 'v') str[i] = '\v';
				}
				machineCode += to_immediate(to_string(str[i]));
				++address;
			}
			machineCode += "00"; // null terminator
			++address;
		}
		else if (symbols[i] == ".data" && i < symbols.size() - 1 && isInteger(symbols[i + 1]))
		{
			string hexData = to_hex(symbols[i + 1]);
			if (hexData.size() % 2) hexData = "0" + hexData;
			machineCode += hexData;
			address += hexData.size() / 2;
		}
	}
	return machineCode;
}

// function to write machine code from a string to a binary file
void writeFile(string filename, string machineCode)
{
	ofstream fout(filename, ios::binary);

	for (int i = 0; i < machineCode.length() - 1; i += 2)
	{
		string byteString = "0x";
		byteString.push_back(machineCode[i]);
		byteString.push_back(machineCode[i + 1]);
		unsigned char byte = integer(byteString);
		fout << byte;
	}

	fout.close();
}

int main()
{
	// get the source filename
	string sourceFilename;
	cout << "source filename: ";
	getline(cin, sourceFilename);

	// get the destination filename
	string destFilename;
	cout << "destination filename: ";
	getline(cin, destFilename);

	// load the file
	string asmCode = loadFile(sourceFilename);

	// assemble into binary machine code
	string machineCode = assemble(asmCode);
	if (machineCode.empty()) return 0;

	// cout the hex code so we can paste it into logisim if desired

	cout << "HEX DUMP: " << machineCode.length() / 2 << " bytes" << endl << endl;
	for (int i = 0; i < machineCode.size() - 1; i += 2)
		cout << (char)machineCode[i] << (char)machineCode[i + 1] << " ";
	cout << endl << endl;

	// write the resulting machine code to a file
	writeFile(destFilename, machineCode);

	// end program
	return 0;
}
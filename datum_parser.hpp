#ifndef DATUM_PARSER_H
#define DATUM_PARSER_H

#include <vector>
#include <variant>
#include <iomanip>
#include <sstream>
#include <iostream>
#include <string>
#include <functional>
#include "date.h"

namespace datum
{
#define RESET "\033[0m"
#define RED "\033[31m"

// ERROR HANDLING

enum ErrorSeverity
{
	Silence,
	Logging,
	Throw,
	Terminate
};

extern ErrorSeverity __global_error_severity;

void set_error_severity(ErrorSeverity const &e);
void handle_error(std::string const &msg);
void handle_error(ErrorSeverity const &e, std::string const &msg);

typedef std::variant<std::string, long double, long long, unsigned long> Entry;
typedef std::string Pattern;

enum DataType
{
	Currency,
	Date,
	String,
	Float,
	Integer,
	Percentage
};

class Datum
{
public:
	Datum(std::string e, DataType t): entry(e), type(t) {}
	Datum(long double e, DataType t): entry(e), type(t) {}
	Datum(long long e, DataType t): entry(e), type(t) {}
	Datum(unsigned long e, DataType t): entry(e), type(t) {}

	template <typename T>
	void visit(std::function<void(T)> visitor)
	{
		try
		{
			visitor(std::get<T>(entry));
		}
		catch(...)
		{
			handle_error("Invalid type argument to visit");
		}
	}

	void visit(std::function<void(Entry, DataType)> visitor)
	{
		visitor(entry, type);
	};

private:
	Entry entry;
	DataType type;
};

struct ParseInstruction
{
	DataType type;
	Pattern pattern;
	ParseInstruction(DataType t) : type(t), pattern("") {}
	ParseInstruction(DataType t, Pattern p) : type(t), pattern(p) {}
};

/* PARSER: Datum =============== */

Datum parse(std::string const &raw_data);
Datum parse(std::string const &raw_data, ParseInstruction const &parse_instruction);
ParseInstruction get_parse_instruction(std::string const &raw_data);


/* PARSER: Numerical =============== */

long long parse_integer(std::string const &str_num);
long double parse_float(std::string const &str_num);
bool is_numeric(const std::string &s);
bool is_integer(const std::string &s);


/* PARSER: Date =============== */

unsigned long parse_date(std::string const &date_string);
unsigned long parse_date(std::string const &date_string, Pattern const &pattern);
bool is_date(std::string const &date_string);
Pattern get_date_pattern(std::string const &date_string);


} // namespace datum

#endif /* DATUM_PARSER_H */

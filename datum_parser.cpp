#include "datum_parser.hpp"

namespace datum
{

// remove ascii characters that do not match the predicate
std::string filter_chars(std::string const &s, std::function<bool(unsigned char)> predicate);

/* Error handling =============== */

ErrorSeverity __global_error_severity = ErrorSeverity::Throw;

void set_error_severity(ErrorSeverity const &e)
{
	__global_error_severity = e;
}

void handle_error(std::string const &msg)
{
	handle_error(__global_error_severity, msg);
}

void handle_error(ErrorSeverity const &e, std::string const &msg)
{
	switch (e)
	{
	case ErrorSeverity::Throw:
		throw std::invalid_argument(msg);
	case ErrorSeverity::Terminate:
		std::cerr << RED << msg << RESET << " exit 1" << '\n';
		exit(1);
	case ErrorSeverity::Logging:
		std::cerr << RED << msg << RESET << '\n';

	case ErrorSeverity::Silence:
		break;

	default:
		break;
	}
}

/* PARSER: Datum  ===============  */

Datum parse(std::string const &raw_data)
{
	return parse(raw_data, get_parse_instruction(raw_data));
}

ParseInstruction get_parse_instruction(std::string const &raw_data)
{
	if (is_integer(raw_data))
		return ParseInstruction(DataType::Integer);

	if (is_numeric(raw_data))
		return ParseInstruction(DataType::Float);

	std::string pattern;
	if ((pattern = get_date_pattern(raw_data)) != "invalid")
		return ParseInstruction(DataType::Date, pattern);

	return ParseInstruction(DataType::String);
}

Datum parse(std::string const &raw_data, ParseInstruction const &parse_instruction)
{
	switch (parse_instruction.type)
	{
	case DataType::Currency:
	case DataType::Float:
	case DataType::Percentage:
		return Datum(parse_float(raw_data), parse_instruction.type);

	case DataType::Date:
		if (parse_instruction.pattern == "")
			handle_error("Attempting to parse date with empty pattern");
		return Datum(parse_date(raw_data, parse_instruction.pattern), parse_instruction.type);

	case DataType::Integer:
		return Datum(parse_integer(raw_data), parse_instruction.type);

	case DataType::String:
		return Datum(raw_data, parse_instruction.type);

	default:
		return Datum("", DataType::String);
	}
}

/* PARSER: Numerical =============== */

long long parse_integer(std::string const &str_num)
{
	try
	{
		return std::stoll(filter_chars(str_num, [](unsigned char c) -> bool {
			return c == '.' || (c >= 48 && c <= 57);
		}));
	}
	catch (const std::exception &e)
	{
		handle_error("Failed to convert: " + str_num + " to long long");
		return 0;
	}
}

long double parse_float(std::string const &str_num)
{
	try
	{
		return std::stold(filter_chars(str_num, [](unsigned char c) -> bool {
			return c == '.' || (c >= 48 && c <= 57);
		}));
	}
	catch (const std::exception &e)
	{
		handle_error("Failed to convert: " + str_num + " to long double");
		return 0;
	}
}

bool is_numeric(const std::string &s)
{
	if (s.empty())
		return false;

	/* only commas, digits are legal and decimals */
	std::string illegal_chars = filter_chars(s, [](unsigned char c) -> bool {
		return !((c >= 48 && c <= 57) || c == ',' || c == '.' || c == '%' || c == '$' || c >= 156);
	});

	if (illegal_chars.size() > 0)
		return false;

	try
	{
		/* remove commas, and any exotic characters, percentage symbols etc... */
		std::stold(filter_chars(s, [](unsigned char c) -> bool {
			return c < 127 && c != ',';
		}));
		return true;
	}
	catch (...)
	{
		return false;
	}

	return false;
}

bool is_integer(const std::string &s)
{
	auto predicate = [](unsigned char c) -> bool {
		return !std::isdigit(c);
	};

	return !s.empty() && std::find_if(s.begin(), s.end(), predicate) == s.end();
}

/* PARSER: Date =============== */

const std::vector<std::string> date_patterns = {
	"%a, %d %b %Y %T %z",
	"%Y-%m-%dT%H:%M:%SZ",
	"%Y-%m-%dT%H:%M:%S",
	"%Y-%m-%d %H:%M:%S",
	"%Y/%m/%dT%H:%M:%SZ",
	"%Y/%m/%dT%H:%M:%S",
	"%Y/%m/%d %H:%M:%S",
	"%d/%m/%Y %H:%M:%S",
	"%d/%m/%Y %H:%M",
	"%d/%m/%Y %I:%M %p",
	"%m/%d/%Y %I:%M%p",
	"%d-%m-%Y %H:%M:%S",
	"%d-%m-%Y %H:%M",
	"%d-%m-%Y %I:%M %p",
	"%m-%d-%Y %I:%M%p",
	"%b %e, %Y %I:%M %p",
	"%Y/%m/%d",
	"%d/%m/%y",
	"%d/%m/%Y",
	"%m/%d/%Y",
	"%Y/%m",
	"%d/%b/%y",
	"%Y-%m-%d",
	"%d-%b-%y",
	"%d-%m-%y",
	"%d-%m-%Y",
	"%m-%d-%Y",
	"%d-%b-%y",
	"%Y-%m",
	"%d %b %Y",
	"%b %d, %Y",
};

unsigned long parse_date(std::string const &date_string)
{
	std::string pattern = get_date_pattern(date_string);
	if (pattern == "invalid")
		return 0;

	return parse_date(date_string, pattern);
}

unsigned long parse_date(std::string const &date_string, Pattern const &pattern)
{
	std::istringstream in(date_string);
	date::sys_time<std::chrono::milliseconds> tp;
	in >> date::parse(pattern, tp);

	if (in.fail())
	{
		handle_error("Failed to convert: '" + date_string + "' with pattern: '" + (pattern.empty() ? "empty pattern" : pattern) + "' to milliseconds");
		return 0;
	}

	auto tp_days = date::floor<date::days>(tp);

	return std::chrono::duration_cast<std::chrono::milliseconds>(tp_days.time_since_epoch()).count();
}

bool is_date(std::string const &date_string)
{
	return get_date_pattern(date_string) != "invalid";
}

Pattern get_date_pattern(std::string const &date_string)
{
	date::sys_time<std::chrono::milliseconds> tp;

	for (auto opt : date_patterns)
	{
		std::istringstream in(date_string);
		in >> date::parse(opt, tp);

		if (in.fail())
			continue;
		else
			return opt;
	}

	handle_error("Could not find date pattern for: " + date_string);
	return "invalid";
}

// util

std::string filter_chars(std::string const &s, std::function<bool(unsigned char)> predicate)
{
	std::string new_str;

	for (auto c : s)
		if (predicate(c))
			new_str += c;

	return new_str;
}
} // namespace datum
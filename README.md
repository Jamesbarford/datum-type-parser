# Simple type parsing

The aim is to provide simple parsing of dates, currencies and percentages to a common object `Datum`

## Setup
Download 'date.h' `https://github.com/HowardHinnant/date`

Or:

```sh
sh ./install_dependencies.sh
```

## Error handling
Default setting is to `throw` a `std::invalid_argument`.

```cpp
enum ErrorSeverity
{
	Silence, // no errors! 😄, absolutely recommend against this
	Logging, // keep ticking over, logs error
	Throw, // throws `std::invalid_argument`
	Terminate // logs error and exits
};
```

## `DataType`

```cpp
enum DataType
{
	Date, // -> unsigned long -> milliseconds since epoch-> "2020/09/23" -> 1600819200000 -> 0 if invalid
	String, // -> string -> "hello world" -> "hello world"
	Float, // -> long double -> 1,000,000.95 -> 1000000.95000 -> 0 if invalid
	Currency, // -> long double -> $1,350,00.96 -> 135000.96000 -> 0 if invalid
	Percentage, // -> long double -> 99.99% -> 99.99000 -> 0 if invalid
	Integer // -> long long ->  1,000,000.95 -> 1000000 -> 0 if invalid
};
```

## Parsers:

### datum parser - top level

The top level parser takes a `string` and `ParseInstruction`. `ParseInstruction` requires a `DataType` and an optional pattern to parse the string. The pattern is mandatory for a date. See `https://en.cppreference.com/w/cpp/chrono/parse` for a list of patterns.

## N.B

If the error setting is `Logging` or `Silence` then an invalid parse of a `Numeric` datatype or a `Date` will result in `0`.

```cpp
Datum parse(std::string const &raw_data, ParseInstruction const &parse_instruction);
Datum parse(std::string const &raw_data);
ParseInstruction get_parse_instruction(std::string const &raw_data);
```

```cpp
typedef std::string Pattern;

struct ParseInstruction
{
	DataType type;
	Pattern pattern;
	ParseInstruction(DataType t) : type(t), pattern("") {}
	ParseInstruction(DataType t, Pattern p) : type(t), pattern(p) {}
};
```

This will return a `Datum`. the value can be visited by providing a callback to `visit` with the correct type. One of `std::string, long double, long long, unsigned long`

```cpp
class Datum
{
public:
	Datum(std::string e, DataType t);
	Datum(long double e, DataType t);
	Datum(long long e, DataType t);
	Datum(unsigned long e, DataType t);
	template <typename T>
	void visit(std::function<void(T)> visitor);
	void visit(std::function<void(Entry, DataType)> visitor);
};
```

`Entry` can be instantiated with `std::string`, `long double`, `long long`, `unsigned long`


### Date parser

Parses a date string to milliseconds since epoch or 0 if invalid `"2020/09/23" -> 1600819200000`.

Can make an unstable guess at parsing the date string or can be provided with a pattern see `https://en.cppreference.com/w/cpp/chrono/parse` for a list of patterns.

Internally uses: `https://github.com/HowardHinnant/date`

```cpp
unsigned long parse_date(std::string const &date_string);
unsigned long parse_date(std::string const &date_string, Pattern const &pattern);
bool is_date(std::string const &date_string);
Pattern get_date_pattern(std::string const &date_string);
```

### Numerical parser

Will remove all characters except `0-9` and `.` will parse to a `long double` if `Currency`, `Float` or `Percentage` otherwise will be parsed as a `long double`.

```cpp
long long parse_integer(std::string const &str_num);
long double parse_float(std::string const &str_num);
bool is_numeric(const std::string &s);
bool is_integer(const std::string &s);
```

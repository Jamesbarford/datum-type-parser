#include <iostream>
#include "datum_parser.hpp"

int main(void)
{
	datum::set_error_severity(datum::ErrorSeverity::Logging);
	datum::Datum d = datum::parse("2020/09/dog09", datum::ParseInstruction(datum::DataType::Date));

	d.visit<unsigned long>([](auto e) { std::cout << e << '\n'; });
}
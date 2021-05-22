#include "numbers.h"

nano::uint128_union::uint128_union (std::string const & string_a)
{
	auto error (decode_hex (string_a));
}

nano::uint128_union::uint128_union (uint64_t value_a)
{
	*this = uint128_t (value_a);
}

nano::uint128_union::uint128_union (uint128_t const & number_a)
{
	bytes.fill (0);
	boost::multiprecision::export_bits (number_a, bytes.rbegin (), 8, false);
}

bool nano::uint128_union::operator== (uint128_union const & other_a) const
{
	return qwords[0] == other_a.qwords[0] && qwords[1] == other_a.qwords[1];
}

bool nano::uint128_union::operator!= (uint128_union const & other_a) const
{
	return !(*this == other_a);
}

bool nano::uint128_union::operator< (uint128_union const & other_a) const
{
	return std::memcmp (bytes.data (), other_a.bytes.data (), 16) < 0;
}

bool nano::uint128_union::operator> (uint128_union const & other_a) const
{
	return std::memcmp (bytes.data (), other_a.bytes.data (), 16) > 0;
}

uint128_t nano::uint128_union::number () const
{
	uint128_t result;
	boost::multiprecision::import_bits (result, bytes.begin (), bytes.end ());
	return result;
}

void nano::uint128_union::encode_hex (std::string & text) const
{
	std::stringstream stream;
	stream << std::hex << std::uppercase << std::noshowbase << std::setw (32) << std::setfill ('0');
	stream << number ();
	text = stream.str ();
}

bool nano::uint128_union::decode_hex (std::string const & text)
{
	auto error (text.size () > 32);
	if (!error)
	{
		std::stringstream stream (text);
		stream << std::hex << std::noshowbase;
		uint128_t number_l;
		try
		{
			stream >> number_l;
			*this = number_l;
			if (!stream.eof ())
			{
				error = true;
			}
		}
		catch (std::runtime_error &)
		{
			error = true;
		}
	}
	return error;
}

void nano::uint128_union::encode_dec (std::string & text) const
{
	std::stringstream stream;
	stream << std::dec << std::noshowbase;
	stream << number ();
	text = stream.str ();
}

bool nano::uint128_union::decode_dec (std::string const & text, bool decimal)
{
	auto error (text.size () > 39 || (text.size () > 1 && text.front () == '0' && !decimal) || (!text.empty () && text.front () == '-'));
	if (!error)
	{
		std::stringstream stream (text);
		stream << std::dec << std::noshowbase;
		boost::multiprecision::checked_uint128_t number_l;
		try
		{
			stream >> number_l;
			uint128_t unchecked (number_l);
			*this = unchecked;
			if (!stream.eof ())
			{
				error = true;
			}
		}
		catch (std::runtime_error &)
		{
			error = true;
		}
	}
	return error;
}

bool nano::uint128_union::decode_dec (std::string const & text, uint128_t scale)
{
	bool error (text.size () > 40 || (!text.empty () && text.front () == '-'));
	if (!error)
	{
		auto delimiter_position (text.find (".")); // Dot delimiter hardcoded until decision for supporting other locales
		if (delimiter_position == std::string::npos)
		{
			uint128_union integer;
			error = integer.decode_dec (text);
			if (!error)
			{
				// Overflow check
				try
				{
					auto result (boost::multiprecision::checked_uint128_t (integer.number ()) * boost::multiprecision::checked_uint128_t (scale));
					error = (result > std::numeric_limits<uint128_t>::max ());
					if (!error)
					{
						*this = uint128_t (result);
					}
				}
				catch (std::overflow_error &)
				{
					error = true;
				}
			}
		}
		else
		{
			uint128_union integer_part;
			std::string integer_text (text.substr (0, delimiter_position));
			error = (integer_text.empty () || integer_part.decode_dec (integer_text));
			if (!error)
			{
				// Overflow check
				try
				{
					error = ((boost::multiprecision::checked_uint128_t (integer_part.number ()) * boost::multiprecision::checked_uint128_t (scale)) > std::numeric_limits<uint128_t>::max ());
				}
				catch (std::overflow_error &)
				{
					error = true;
				}
				if (!error)
				{
					uint128_union decimal_part;
					std::string decimal_text (text.substr (delimiter_position + 1, text.length ()));
					error = (decimal_text.empty () || decimal_part.decode_dec (decimal_text, true));
					if (!error)
					{
						// Overflow check
						auto scale_length (scale.convert_to<std::string> ().length ());
						error = (scale_length <= decimal_text.length ());
						if (!error)
						{
							auto base10 = boost::multiprecision::cpp_int (10);
							auto pow10 = boost::multiprecision::pow (base10, static_cast<unsigned> (scale_length - decimal_text.length () - 1));
							auto decimal_part_num = decimal_part.number ();
							auto integer_part_scaled = integer_part.number () * scale;
							auto decimal_part_mult_pow = decimal_part_num * pow10;
							auto result = integer_part_scaled + decimal_part_mult_pow;

							// Overflow check
							error = (result > std::numeric_limits<uint128_t>::max ());
							if (!error)
							{
								*this = uint128_t (result);
							}
						}
					}
				}
			}
		}
	}
	return error;
}

std::string nano::uint128_union::to_string () const
{
	std::string result;
	encode_hex (result);
	return result;
}

std::string nano::uint128_union::to_string_dec () const
{
	std::string result;
	encode_dec (result);
	return result;
}

void format_frac (std::ostringstream & stream, uint128_t value, uint128_t scale, int precision)
{
	auto reduce = scale;
	auto rem = value;
	while (reduce > 1 && rem > 0 && precision > 0)
	{
		reduce /= 10;
		auto val = rem / reduce;
		rem -= val * reduce;
		stream << val;
		precision--;
	}
}

void format_dec (std::ostringstream & stream, uint128_t value, char group_sep, const std::string & groupings)
{
	auto largestPow10 = uint256_t (1);
	int dec_count = 1;
	while (1)
	{
		auto next = largestPow10 * 10;
		if (next > value)
		{
			break;
		}
		largestPow10 = next;
		dec_count++;
	}

	if (dec_count > 39)
	{
		// Impossible.
		return;
	}

	// This could be cached per-locale.
	bool emit_group[39];
	if (group_sep != 0)
	{
		int group_index = 0;
		int group_count = 0;
		for (int i = 0; i < dec_count; i++)
		{
			group_count++;
			if (group_count > groupings[group_index])
			{
				group_index = std::min (group_index + 1, (int)groupings.length () - 1);
				group_count = 1;
				emit_group[i] = true;
			}
			else
			{
				emit_group[i] = false;
			}
		}
	}

	auto reduce = uint128_t (largestPow10);
	uint128_t rem = value;
	while (reduce > 0)
	{
		auto val = rem / reduce;
		rem -= val * reduce;
		stream << val;
		dec_count--;
		if (group_sep != 0 && emit_group[dec_count] && reduce > 1)
		{
			stream << group_sep;
		}
		reduce /= 10;
	}
}


std::string format_balance (uint128_t balance, uint128_t scale, int precision, bool group_digits, char thousands_sep, char decimal_point, std::string & grouping)
{
	std::ostringstream stream;
	auto int_part = balance / scale;
	auto frac_part = balance % scale;
	auto prec_scale = scale;
	for (int i = 0; i < precision; i++)
	{
		prec_scale /= 10;
	}
	if (int_part == 0 && frac_part > 0 && frac_part / prec_scale == 0)
	{
		// Display e.g. "< 0.01" rather than 0.
		stream << "< ";
		if (precision > 0)
		{
			stream << "0";
			stream << decimal_point;
			for (int i = 0; i < precision - 1; i++)
			{
				stream << "0";
			}
		}
		stream << "1";
	}
	else
	{
		format_dec (stream, int_part, group_digits && grouping.length () > 0 ? thousands_sep : 0, grouping);
		if (precision > 0 && frac_part > 0)
		{
			stream << decimal_point;
			format_frac (stream, frac_part, scale, precision);
		}
	}
	return stream.str ();
}

std::string nano::uint128_union::format_balance (uint128_t scale, int precision, bool group_digits) const
{
	auto thousands_sep = std::use_facet<std::numpunct<char>> (std::locale ()).thousands_sep ();
	auto decimal_point = std::use_facet<std::numpunct<char>> (std::locale ()).decimal_point ();
	std::string grouping = "\3";
	return ::format_balance (number (), scale, precision, group_digits, thousands_sep, decimal_point, grouping);
}

std::string nano::uint128_union::format_balance (uint128_t scale, int precision, bool group_digits, const std::locale & locale) const
{
	auto thousands_sep = std::use_facet<std::moneypunct<char>> (locale).thousands_sep ();
	auto decimal_point = std::use_facet<std::moneypunct<char>> (locale).decimal_point ();
	std::string grouping = std::use_facet<std::moneypunct<char>> (locale).grouping ();
	return ::format_balance (number (), scale, precision, group_digits, thousands_sep, decimal_point, grouping);
}

void nano::uint128_union::clear ()
{
	qwords.fill (0);
}

bool nano::uint128_union::is_zero () const
{
	return qwords[0] == 0 && qwords[1] == 0;
}

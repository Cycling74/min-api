//	Copyright 2013 - Cycling '74
//	Timothy Place, tim@cycling74.com	

#pragma once

namespace c74 {
namespace max {


// Mock implementations for basic atom accessors

inline long atom_gettype(const t_atom *a)			{ return a->a_type; }

inline t_atom_float atom_getfloat(const t_atom *a)	{ return a->a_w.w_float; }
inline t_atom_long atom_getlong(const t_atom *a)	{ return a->a_w.w_long; }
inline t_symbol *atom_getsym(const t_atom *a)		{ return a->a_w.w_sym; }

inline t_max_err atom_setfloat(t_atom *a, double v)		{a->a_w.w_float = v; a->a_type = A_FLOAT;return 0;}
inline t_max_err atom_setlong(t_atom *a, t_atom_long v)	{a->a_w.w_long = v; a->a_type = A_LONG;return 0;}
inline t_max_err atom_setsym(t_atom *a, t_symbol *s)	{a->a_w.w_sym = s; a->a_type = A_SYM;return 0;}


// Special stuff for the mocked testing environment

/**	A vector of atoms.	*/
typedef std::vector<t_atom>	t_atom_vector;

/** A sequence of atom vectors.	 
	Used to log inlet and outlet activity in the mock environment.	
	We can use logging of inlet and outlet sequences for BDD.
	We can also do more traditional state-based testing.
	Or mix-n-match as we see fit...

	@remark		should sequences have time-stamps?
 */
typedef std::vector<t_atom_vector>	t_sequence;


/** Expose t_atom for use in std output streams. 
	@remark		Would be nice to have the functionality of atoms_totext(), but that's also pretty complex!
 */
template <class charT, class traits>
std::basic_ostream <charT, traits>& operator<< (std::basic_ostream <charT, traits>& stream, const t_atom& a)
{
	char str[4096]; // TODO: can I really alloc this on the stack? the stream probably copies it, but not sure!
	
	if (a.a_type == A_LONG)
#ifdef _LP64
		snprintf(str, 4096, "%lld", a.a_w.w_long);
#else
		snprintf(str, 4096, "%ld", a.a_w.w_long);
#endif
	else if (a.a_type == A_FLOAT)
		snprintf(str, 4096, "%f", a.a_w.w_float);
	else if (a.a_type == A_SYM)
		snprintf(str, 4096, "%s", a.a_w.w_sym->s_name);
	else
		snprintf(str, 4096, "%s", "<nonsense>");
		
	return stream << str;
}
	

// comparison functions for atoms to make it easier (and less rsi/typing!) to check results in atoms.

/** epsilon for comparing floats. 
	see http://www.cygnus-software.com/papers/comparingfloats/comparingfloats.htm
 */
static const double k_epsilon = 0.000001;


/** Compare floats despite quantization error.
	This function lifted from oscar.
 */
t_bool testequals_equivalent(double a, double b)
{
	t_bool	result;
	double	a_abs = fabs(a);
	double	b_abs = fabs(b);
	double	absolute_or_relative = (1.0f > a_abs ? 1.0f : a_abs);
	
	absolute_or_relative = (absolute_or_relative > b_abs ? absolute_or_relative : b_abs);
	if (fabs(a - b) <= k_epsilon * absolute_or_relative)
		result = true;
	else
		result = false;
	
	return result;
}


inline bool operator==(const t_atom& a, const int& i)
{
	return (a.a_type == A_LONG && a.a_w.w_long == i);
}


inline bool operator==(const t_atom& a, const double& f)
{
	return (a.a_type == A_FLOAT) && testequals_equivalent(a.a_w.w_float, f);
	//return (a.a_type == A_FLOAT && a.a_w.w_float == f);
}


inline bool operator==(const t_atom& a, const char* s)
{
	return (a.a_type == A_SYM && a.a_w.w_sym == gensym(s));
}

}} // namespace c74::max


#include "ac_cache_trace.H"

ac_cache_trace::ac_cache_trace(std::ostream &o) : out(o) 
{
	out << std::hex;
}
ac_cache_trace::~ac_cache_trace() { out.flush(); }

void ac_cache_trace::add(trace_operation o, unsigned a, unsigned l)
{
	if (o == trace_read) {
		out << "r ";
	}
	else {
		out << "w ";
	}
	out << a << " " << l << '\n';
}


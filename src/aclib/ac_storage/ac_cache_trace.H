#ifndef _AC_TRACE_H_INCLUDED_
#define _AC_TRACE_H_INCLUDED_

#include <ostream>

enum trace_operation { trace_read, trace_write };

class ac_cache_trace {
	std::ostream &out;
	public:
	ac_cache_trace(std::ostream &o);
	~ac_cache_trace();
	void add(trace_operation o, unsigned a, unsigned l);
};

#endif /* _AC_TRACE_H_INCLUDED_ */


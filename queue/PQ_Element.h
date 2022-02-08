#pragma once

#include <math.h>
#include <phmap_utils.h>

class PQ_Element {
public:
	explicit PQ_Element(double t) : time(t) {}
	virtual ~PQ_Element()	{ }

	double Time() const	{ return time; }
	void MinimizeTime()	{ time = -HUGE_VAL; }

	int Offset() const	{ return offset; }
	void SetOffset(int off)	{ offset = off; }

	bool operator<(const PQ_Element& p) const {
		return time < p.Time();
		}
	bool operator==(const PQ_Element& p) const {
		return time == p.Time();
		}

	double time = 0;
	int offset = -1;

protected:

	PQ_Element() = default;
};

namespace std
{
    // inject specialization of std::hash for Person into namespace std
    // An alternative is to provide a hash_value() friend function (see hash_value.h)
    // ------------------------------------------------------------------------------
    template<> struct hash<PQ_Element>
    {
        std::size_t operator()(PQ_Element const &p) const
        {
        return phmap::HashState().combine(0, p.Time());
        }
    };
}

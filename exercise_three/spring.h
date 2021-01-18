#include <vector>
#include "particle.h"

struct spring {
    const unsigned i, j;
	const double k;
	const double l;

	spring(unsigned _i, unsigned _j, double _k, double _l);
	void draw(std::vector<particle>& particles) const;
	void update(std::vector<particle>& particles, const particle& sphere) const;
};
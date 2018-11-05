#include "als/als_math.h"

struct ConvexHull;

void quickHull(Vec3* pointSoup, uint32 pointSoupCount, ConvexHull* hull_out, bool shouldDeduplicate=true);

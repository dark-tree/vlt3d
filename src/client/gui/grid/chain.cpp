
#include "chain.hpp"

Chain Chain::AFTER {1, 0, 0, 0};
Chain Chain::BEFORE {0, 0, -1, 0};
Chain Chain::ABOVE {0, -1, 0, 0};
Chain Chain::BELOW {0, 0, 0, 1};
Chain Chain::OVER {0, 0, 0, 0};
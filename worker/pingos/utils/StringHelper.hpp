#pragma once

#include <string>
#include <vector>

namespace pingos
{
    std::vector<std::string> SplitOneOf(const std::string& str, const std::string delims, const size_t maxSplits = 0);
} // namespace pingos

#include "Master/Filter.hpp"

namespace Master {

Filter* Filter::Next()
{
    return m_next;
}

void Filter::SetNext(Filter *filter)
{
    m_next = filter;
}

}

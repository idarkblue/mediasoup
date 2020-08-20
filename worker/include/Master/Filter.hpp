#ifndef _FILTER_H_INCLUDE_
#define _FILTER_H_INCLUDE_

#include "Defines.hpp"

namespace Master {

class Filter;

class Filter {
public:
    virtual int OnPlay(RtcSession *s, PlayValue *val) = 0;
    virtual int OnPublish(RtcSession *s, PublishValue *val) = 0;
    virtual int OnCloseStream(RtcSession *s, CloseStreamValue *val) = 0;

    Filter* Next();
    void SetNext(Filter *filter);

private:
    Filter *m_next { nullptr };
};

}

#endif

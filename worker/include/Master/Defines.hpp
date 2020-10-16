#pragma once

#include <string>
#include <json.hpp>

#include "MediaSoupErrors.hpp"

using json = nlohmann::json;

#ifndef MAX_PATH
#define MAX_PATH 65535
#endif

#define JSON_READ_VALUE_ASSERT(_json_object_, _key_, _type_, _val_)             \
    if ((_json_object_).find(_key_) == (_json_object_).end()) {                     \
        PMS_ERROR("Cannot find {}", _key_);                                     \
        return -1;                                                              \
    } else {                                                                    \
        _val_ = (_json_object_)[_key_].get<_type_>();                             \
    }

#define JSON_READ_VALUE_DEFAULT(_json_object_, _key_, _type_, _val_, _default_) \
    if ((_json_object_).find(_key_) == (_json_object_).end()) {                     \
        PMS_WARN("Cannot find {}, set default {}", _key_, _default_);           \
        _val_ = _default_;                                                      \
    } else {                                                                    \
        _val_ = (_json_object_)[_key_].get<_type_>();                             \
    }

#define JSON_READ_VALUE_THROW(_json_object_, _key_, _type_, _val_)              \
    if ((_json_object_).find(_key_) == (_json_object_).end()) {                     \
        MS_THROW_ERROR("missing %s", std::string(_key_).c_str());               \
    } else {                                                                    \
        _val_ = (_json_object_)[_key_].get<_type_>();                             \
    }

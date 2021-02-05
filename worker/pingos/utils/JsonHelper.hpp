#pragma once

#include "json.hpp"
#include "MediaSoupErrors.hpp"

using json = nlohmann::json;

namespace pingos
{
    #define JSON_THROW_READ_VALUE(_ROOT_, _KEY_, _TYPE_, _JSON_TYPE_, _VALUE_)                       \
        if (_ROOT_.find((_KEY_)) == _ROOT_.end() || !_ROOT_.find((_KEY_))->is_##_JSON_TYPE_()) {     \
            MS_ERROR("missing key [%s]", std::string(_KEY_).c_str());                                \
            MS_THROW_ERROR("missing key [%s]", std::string(_KEY_).c_str());                          \
        }                                                                                            \
        _VALUE_ = _ROOT_.find((_KEY_))->get<_TYPE_>();

    #define JSON_THROW_READ_OBJECT(_ROOT_, _KEY_, _IT_)                                              \
        if (_ROOT_.find((_KEY_)) == _ROOT_.end() || !_ROOT_.find((_KEY_))->is_object()) {             \
            MS_ERROR("missing object [%s]", std::string(_KEY_).c_str());                             \
            MS_THROW_ERROR("missing object [%s]", std::string(_KEY_).c_str());                       \
        }                                                                                            \
        _IT_ = *_ROOT_.find((_KEY_));

    #define JSON_THROW_READ_ARRAY(_ROOT_, _KEY_, _IT_)                                               \
        if (_ROOT_.find((_KEY_)) == _ROOT_.end() || !_ROOT_.find((_KEY_))->is_array()) {             \
            MS_ERROR("missing object [%s]", std::string(_KEY_).c_str());                             \
            MS_THROW_ERROR("missing object [%s]", std::string(_KEY_).c_str());                       \
        }                                                                                            \
        _IT_ = *_ROOT_.find((_KEY_))

    #define JSON_DEFAULT_READ_VALUE(_ROOT_, _KEY_, _TYPE_, _JSON_TYPE_, _VALUE_, _DEFAULT_)          \
        if (_ROOT_.find((_KEY_)) != _ROOT_.end() && _ROOT_.find((_KEY_))->is_##_JSON_TYPE_()) {      \
            _VALUE_ = _ROOT_.find((_KEY_))->get<_TYPE_>();                                           \
        } else {                                                                                     \
            _VALUE_ = _DEFAULT_;                                                                     \
        }

    #define JSON_READ_VALUE_ASSERT(_json_object_, _key_, _type_, _val_)             \
        if ((_json_object_).find(_key_) == (_json_object_).end()) {                 \
            MS_ERROR("Cannot find %s", (_key_).c_str());                            \
            return -1;                                                              \
        } else {                                                                    \
            _val_ = (_json_object_)[_key_].get<_type_>();                           \
        }

    #define JSON_READ_VALUE_DEFAULT(_json_object_, _key_, _type_, _val_, _default_) \
        if ((_json_object_).find(_key_) == (_json_object_).end()) {                 \
            _val_ = (_default_);                                                    \
        } else {                                                                    \
            _val_ = (_json_object_)[_key_].get<_type_>();                           \
        }

    #define JSON_READ_VALUE_THROW(_json_object_, _key_, _type_, _val_)              \
        if ((_json_object_).find(_key_) == (_json_object_).end()) {                 \
            MS_THROW_ERROR("missing %s", std::string(_key_).c_str());               \
        } else {                                                                    \
            _val_ = (_json_object_)[_key_].get<_type_>();                           \
        }

} // namespace pingos


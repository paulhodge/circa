// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#pragma once

namespace circa {
namespace set_t {

    bool contains(List* list, TaggedValue* value);
    void add(List* list, TaggedValue* value);
    void setup_type(Type* type);

} // namespace set_t
} // namespace circa

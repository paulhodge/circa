// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#pragma once

namespace circa {

bool is_hashtable(caValue* value);
void set_hashtable(caValue* value);
void set_mutable_hashtable(caValue* value);

caValue* hashtable_get(caValue* table, caValue* key);
caValue* hashtable_get(caValue* table, const char* keyStr);
caValue* hashtable_insert(caValue* table, caValue* key, bool consumeKey);
caValue* hashtable_insert(caValue* table, caValue* key);

caValue* hashtable_get_int_key(caValue* table, int key);
caValue* hashtable_insert_int_key(caValue* table, int key);
void hashtable_remove_int_key(caValue* table, int key);

void hashtable_remove(caValue* table, caValue* key);
bool hashtable_is_empty(caValue* table);
void hashtable_get_keys(caValue* table, caValue* keysOut);

int hashtable_count(caValue* table);
int hashtable_slot_count(caValue* table);
caValue* hashtable_key_by_index(caValue* table, int index);
caValue* hashtable_value_by_index(caValue* table, int index);

void hashtable_setup_type(Type* type);

struct HashtableIterator
{
    caValue* table;
    int index;

    HashtableIterator(caValue* table);

    caValue* currentKey();
    caValue* current();
    void advance();
    bool finished();

    operator bool() { return !finished(); }
    void operator++() { advance(); }

    void _advanceWhileInvalid();
};

} // namespace circa

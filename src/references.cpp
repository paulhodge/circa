// Copyright 2008 Andrew Fischer

#include "circa.h"

namespace circa {

void Ref::set(Term* target)
{
    if (t == target)
        return;

    Term* previousTarget = t;

    t = target;

    if (t != NULL)
        t->refCount++;

    if (previousTarget != NULL) {
        previousTarget->refCount--;
        if (previousTarget->refCount == 0)
            delete_term(previousTarget);
    }
}

void Ref::remap_pointers(Term* term, ReferenceMap const& map)
{
    deref(term) = map.getRemapped(deref(term));
}

void delete_term(Term* term)
{
    assert_good_pointer(term);

    if (term->boolPropOptional("owned-value", true))
        dealloc_value(term);

    assert(term->refCount == 0);

    term->inputs.clear();
    term->type = NULL;
    term->function = NULL;

    unregister_good_pointer(term);

#if !DEBUG_NEVER_DELETE_TERMS
    delete term;
#endif
}

void remap_pointers(Term* term, ReferenceMap const& map)
{
    assert_good_pointer(term);

    // make sure this map doesn't try to remap NULL, because such a thing
    // would almost definitely lead to errors.
    assert(!map.contains(NULL));

    term->inputs.remapPointers(map);
    term->function = map.getRemapped(term->function);

    // TODO, call changeType if our type is changed
    Term* newType = map.getRemapped(term->type);
    if (term->type != newType)
        change_type(term, newType);

    // Remap on value
    if ((term->value != NULL)
            && term->type != NULL
            && (as_type(term->type).remapPointers != NULL)) {

        as_type(term->type).remapPointers(term, map);
    }

    // Remap inside properties
    for (int i=0; i < term->properties.length(); i++)
        remap_pointers(term->properties[i], map);
}

void remap_pointers(Term* term, Term* original, Term* replacement)
{
    assert_good_pointer(term);
    assert_good_pointer(original);
    assert(original != NULL);

    ReferenceMap map;
    map[original] = replacement;
    remap_pointers(term, map);
}

void remap_pointers(Branch& branch, Term* original, Term* replacement)
{
    ReferenceMap map;
    map[original] = replacement;

    for (int i=0; i < branch.length(); i++) {
        remap_pointers(branch[i], map);
    }
}

} // namespace circa

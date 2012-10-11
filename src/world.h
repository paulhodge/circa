// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#pragma once

namespace circa {

struct LoadedDll;

struct World {

    Branch* root;

    circa::Value actorList;
    caStack* actorStack;

    NativeModuleWorld* nativeModuleWorld;

protected:
    // Disallow C++ construction
    World();
    ~World();
};

World* create_world();
void world_initialize(World* world);

void actor_send_message(ListData* actor, caValue* message);
void actor_run_message(caStack* stack, ListData* actor, caValue* message);
ListData* find_actor(World* world, const char* name);

void refresh_all_modules(caWorld* world);

} // namespace circa

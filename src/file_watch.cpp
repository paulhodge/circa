// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "common_headers.h"

#include "block.h"
#include "debug.h"
#include "file.h"
#include "kernel.h"
#include "list.h"
#include "modules.h"
#include "names.h"
#include "native_patch.h"
#include "string_type.h"
#include "tagged_value.h"
#include "world.h"

#define FILE_WATCH_DEBUG_LOGGING 0

namespace circa {

struct FileWatch
{
    Value filename;
    Value onChangeActions;
    int lastObservedVersion;
};

struct FileWatchWorld
{
    std::map<std::string, FileWatch*> watches;
};

FileWatchWorld* alloc_file_watch_world()
{
    FileWatchWorld* world = new FileWatchWorld();
    return world;
}

void free_file_watch_world(FileWatchWorld* world)
{
    std::map<std::string, FileWatch*>::const_iterator it;
    for (it = world->watches.begin();
         it != world->watches.end();
         ++it) {
        FileWatch* watch = it->second;
        delete watch;
    }
    delete world;
}

FileWatch* find_file_watch(World* world, const char* filename)
{
    std::map<std::string, FileWatch*>::const_iterator it =
        world->fileWatchWorld->watches.find(filename);

    if (it != world->fileWatchWorld->watches.end())
        return it->second;

    return NULL;
}

FileWatch* add_file_watch(World* world, const char* filename)
{
    // Return existing watch if it exists.
    FileWatch* existing = find_file_watch(world, filename);
    if (existing != NULL)
        return existing;

#if FILE_WATCH_DEBUG_LOGGING
    printf("add_file_watch: %s\n", filename);
#endif

    FileWatch* newWatch = new FileWatch();
    set_string(&newWatch->filename, filename);
    set_list(&newWatch->onChangeActions, 0);
    newWatch->lastObservedVersion = 0;

    world->fileWatchWorld->watches[filename] = newWatch;
    return newWatch;
}

FileWatch* add_file_watch_action(World* world, const char* filename, Value* action)
{
    // Fetch the FileWatch entry.
    FileWatch* watch = add_file_watch(world, filename);

    // Check if this exact action already exists, if so do nothing.
    if (list_contains(&watch->onChangeActions, action))
        return watch;

    // Add action
    copy(action, list_append(&watch->onChangeActions));

    return watch;
}

static bool file_watch_check_for_update(FileWatch* watch)
{
    int latestVersion = circa_file_get_version(global_world(), as_cstring(&watch->filename));
    if (latestVersion != watch->lastObservedVersion) {
        watch->lastObservedVersion = latestVersion;
        return true;
    }

    return false;
}

void file_watch_trigger_actions(World* world, FileWatch* watch)
{
#if FILE_WATCH_DEBUG_LOGGING
    printf("file_watch_trigger_actions: %s\n", as_cstring(&watch->filename));
#endif

    // Walk through each action and execute it.
    for (int i = 0; i < list_length(&watch->onChangeActions); i++) {
        Value* action = list_get(&watch->onChangeActions, i);

        Symbol label = first_symbol(action);
        ca_assert(label != s_none);

        switch (label) {
        case s_NativePatch: {
            Value* moduleName = list_get(action, 1);

            NativePatch* nativeModule = insert_native_patch(world, moduleName);
            native_patch_load_from_file(nativeModule, as_cstring(&watch->filename));
            circa_finish_native_patch(nativeModule);
            break;
        }
        case s_RecompileModule: {
            printf("recompiling changed module: %s\n", as_cstring(&watch->filename));

            // Reload this code block.
            Block* block = alloc_block(world);
            load_script(block, as_cstring(&watch->filename));


#if 0
            if (has_static_errors(block)) {
                // Temp solution, log to stdout.

                Value msg;
                set_string(&msg, "Static error(s) after reloading file: ");
                string_append(&msg, &watch->filename);
                write_log(world, as_cstring(&msg));
                print_static_errors_formatted(block, &msg);
                write_log(world, as_cstring(&msg));
            } else
#endif
            {
                module_install_replacement(world, &watch->filename, block);
            }
            break;
        }
        default:
            internal_error("unrecognized file watch action");
        }
    }
}

void file_watch_trigger_actions(World* world, const char* filename)
{
    FileWatch* watch = find_file_watch(world, filename);

    // No-op if there is no watch.
    if (watch == NULL)
        return;

    file_watch_trigger_actions(world, watch);
}

void file_watch_check_now(World* world, FileWatch* watch)
{
    if (file_watch_check_for_update(watch))
        file_watch_trigger_actions(world, watch);
}

void file_watch_ignore_latest_change(FileWatch* watch)
{
    file_watch_check_for_update(watch);
}

void file_watch_check_all(World* world)
{
    std::map<std::string, FileWatch*>::const_iterator it;

    for (it = world->fileWatchWorld->watches.begin();
         it != world->fileWatchWorld->watches.end();
         ++it) {
        FileWatch* watch = it->second;
        file_watch_check_now(world, watch);
    }
}

FileWatch* add_file_watch_recompile_module(World* world, const char* filename)
{
    circa::Value action;
    set_list(&action, 1);
    set_symbol(list_get(&action, 0), s_RecompileModule);
    return add_file_watch_action(world, filename, &action);
}

FileWatch* add_file_watch_native_patch(World* world, const char* filename, const char* moduleName)
{
    circa::Value action;
    set_list(&action, 2);
    set_symbol(list_get(&action, 0), s_NativePatch);
    set_string(list_get(&action, 1), moduleName);
    return add_file_watch_action(world, filename, &action);
}

CIRCA_EXPORT void circa_update_changed_files(caWorld* world)
{
    return file_watch_check_all(world);
}

} // namespace circa

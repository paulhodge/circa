
## First-Class Change Events ##

Plan written on 8/28/2013

OVERVIEW

In the new scheme of live code changes, every change will be stored as a first class
value. These events might come from an editor or a remote source. When we receive
an event, we immediately update all relevant pieces of data. This starts with modifying
the code itself, and continues to all derived pieces of data: live execution stacks and
runtime state.

Change events should support distributed development, where several clients can work together
on a shared piece of code by exchanging change messages.

Change events can also be used for an undo system.

MOTIVATION

The change-event system is something we knew we needed, in order to have sane live updates.
Any live updating system where changes aren't captured as first-class values is probably
going to be unsound, especially w/r/t the problem of migrating state.

The old system Circa had was not sufficient. In the old system, Blocks could be modified
in-place, each Block had a monotonically-increasing version number, and any references to a
Block (such as the references inside Stack) would store the version. This way, any users
of a Block would know whether they had used an old version. If the version number changed, the
user could update their derived data (such as adding/deleting registers in a stack's Frame).

The main drawback was not having enough context when migrating data across a change event.
The Block's users had no context for how the current block's state related to the old Block,
all they knew was that their data was old. This approach would fall flat when trying to
preserve data across complicated changes in a sound way.

In the new event, each change event is a first-class value. When handling a change, we
immediately update/migrate all existing data according to the change.

The new system can be called 'structured' changes, where the existing system was unstructured.

UNSTRUCTURED MIGRATION

Circa will still support the migration of state across two possibly-unrelated branches. This
is the feature that allows for arbitrary editing of source files as text files. This feature
does not use change events.

CHANGE EVENT DETAILS

A ChangeEvent contains the following fields:

 - Change type
 - Change details
 - Code target (Block or Term)
  - Possibly included inside 'change details' because the format of the target may vary per change type.
 - Optional metadata depending on use case:
   - Timestamp
   - Client id (in a multi-user case)

There's a variety of change event types. As time goes on we may add more. Here are a few example change types:

  - Rename a single term
  - Replace a block's content using a string expression.
  - Append a string expression to the end of a block
  - A compound list of change events
    - The compound list case introduces some tricky issues, such as how do we validate the Nth event without committing the (N-1)th event, and if an event fails to validate, how do we cleanly rollback the previous events? (needs more spec)

Each change event has a 'target' (such as a Term or a Block), which may be a direct pointer, or it may be a name.

OPERATIONAL TRANSFORM

Change events support operational transform (OT), which is useful for a few use cases,
including distributed development.

A basic outline of operational transform: Say we have two change events A and B which are
made against the same version. We want to apply them both, but they may interact with each
other. We first pick one (say A) and apply it as normal. Then we use A to transform B into a
similar change event called B', where B' is valid to apply to the result of applying A.

Implementation-wise, the OT process is a double dispatch, first dispatching on the type
of change event A, and then (probably) dispatching on the type of change event B.

The OT process may fail if there is a conflict between A and B, so any process relying
on OT should be able to handle failure.

APPLYING A CHANGE EVENT

Applying a change event is a two step process:

 1) Validate:
   (world, changeEvent) -> (bool success, validationSummary)

 2) Commit:
   (world, changeEvent, validationSummary, bool generateInverse) -> (modified world, maybe inverseChangeEvent)

When invoking the validate/commit process, the caller must follow these rules:

 1) No changes to code data may occur after Validate and before Commit. Ideally the Commit step occurs immediately after.

 2) The caller may call Validate and then decide never to Commit (even if it passed validation).

The Validate handler is dispatched by the change event type. In this step, the change event is
examined against the current code, to determine if it can actually be performed. This step
may fail if the change was invalid, or if the block was changed after the change was
created.

The Validate step may examine Blocks but it may not examine Stacks. This is a deliberate
restriction: we don't want validation to be influenced by the current runtime Stack
data. Some reasons for this restriction are:

 1) In a client-server model, the server is responsible for vetting incoming changes,
    and sending a stream of guaranteed-valid changes to each client. However the server
    might not actually be running the code, so it has no Stack to validate against.

 2) In general, to have a sane & predictable system, it's good if the change submission
    process can't error as a result of what stacks may happen to be in progress.
    Creating a Stack should be a private concern.

To deal with the restriction of no-validate-against-stacks, the validator should reason
about existing Stacks using static knowledge from the code, erring on the side of being
conservative.

The Validate step returns success, and it also returns a 'validationSummary' value. This summary
value is private to the change event type (and it is optional). The summary is passed along
to the Commit step. The motivation is that the Commit step can use the summary to avoid
performing duplicate work.

If Validate doesn't fail, Commit immediately follows. The Commit step isn't allowed to fail.

The Commit step is also dispatched by the change event type. It consists of these steps:

 1. Modify the relevant Blocks and Terms
   - Also perform cascading changes (such as regenerating bytecode)
 2. Iterate on every allocated Stack to migrate across the change (see 'updating stack data')

Additionally, Commit receives a generateInverse flag. If enabled, the Commit action
should output a new change event which is the inverse of the change just performed.

 Q: Will it be a problem for every change event type to know how to generate an inverse?

 A: Probably not; in the easy case, the inverse can simply be a snapshot of affected block(s).

UPDATING STACK DATA

When committing a change event, we must update every live allocated Stack. The following
changes must be made.

  - Update 'state'
    - Remove state elements for deleted terms
    - Open question: do we instantiate state elements for new state terms? (Alternative is
      to wait and allow the missing state to be instantiated at runtime)
  - Update 'registers'
    - Resize list if needed
    - Move around existing values if there are any term moves
    - Update PC*
    - Evaluate new expressions that occur before PC*
    - Reevaluate changed expressions that occur before PC*
      - And reevaluate expressions that depend on them, etc.
  - Possibly alter the frames list*
    - For larger changes that involve adding/deleting blocks
    - Remove frames using deleted blocks
  - Update memoized frames
    - Delete memoized data for any blocks that were modified.

(*) Items with a star are only required once we support the migration of in-progress stacks.

The first implementation will not support in-progress migration, only migration of finished
stacks, so we have the following guarantees:

 - Stacks don't have a PC
 - Each Stack only has the one top-level frame
 - Stacks *may* have a tree of saved 'state' frames (for inline state and memoized frames)

DISTRIBUTED CHANGE EVENTS

A rundown of how we might use change events to enable a distributed development environment.
In this setup, we have one trusted server and multiple clients, each with their own copy of
the codebase. Additionally, the clients probably have running Stacks that use the code, but
this isn't a requirement.

Model 1 - No client-side prediction

In this model, all change events are passed through the trusted server. Clients don't use
any change events that don't come directly from the server.

Pseudocode:

0. Server has a monotonically-increasing version which all clients know.
1. Client creates change event, and validates it locally.
2. Client uploads change event to server, with version number attached.
3. Server receives change event
4. Server possibly transforms event, if it is made against an old version.
    - Server should maintain a recent history of change events in order to OT changes made
      against older versions.
    - History doesn't need to be very long, the primary purpose is to support changes that
      are received at roughly the same time.
    - The transform step may fail.
5. Server validates change event
6. If the event fails during OT step or validation step, server informs client that the
    event is rejected. Client should handle gracefully.
7. If accepted, submit event to server's copy of the world, and broadcast the event to all
    listeners.
8. Clients receive an event stream directly from server, and apply change events as they
    are received. These events are (hopefully) guaranteed to always be valid, because the
    server has already vetted them. A validation error here should probably be handled by
    a disconnect/reconnect.

Model 2 - Client-side prediction

In this model, client immediately submits the change event when it is generated, in order
to see the effect immediately. It is then submitted to the server as in model 1. The client might need to rollback if the server rejects the event. Additionally, some sort of transform will need to take place if the client receives other events before their event is confirmed. (needs spec)

UNDO/REDO

Through change events, we can also build an undo/redo system suitable for an editor.
Here is the outline:

 1) When submitting an event, enable the generateInverse flag, and save the inverse
    event in the undo-list.

 2) When the user hits Undo, pick the event from the top of the undo-list, submit it,
    and save its inverse in the redo-list.

In a more ambitious case, we might be able to have undo/redo in combination with an external
source of events, such as a remote server. Pseudocode:

 1. Client does a few actions, which generates an undo list, U = [C1, C2, C3]
 2. Client receives a foreign change event from server
   Q: How does client tell which events came from themselves?
 3. Client inserts the foreign event F1 into its history: [F1, C1, C2, C3]
   (As in, we have one external event at the top of the stack, and then three locally-
    generated events).
 4. Client hits undo key. We now want to apply C1 to the world.
 5. Use OT on C1 to bring it across F1, then submit the transformed event.

(Needs more spec)

Notes:

 - In distributed world, each client needs an id so that they can recognize their own events.
 - Each server-submitted event should also have a client-unique id?
 

### API ###

    // Create specific change events:
    void change_event_make_rename(Value* event, Term* target, const char* newName);
    void change_event_make_append(Value* event, Block* target, const char* str);

    bool change_event_validate(World* world, Value* event, Value* summaryOut);
    void change_event_commit(World* world, Value* event, Value* summary, Value* inverseOut);


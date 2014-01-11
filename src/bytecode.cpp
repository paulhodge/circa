// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "common_headers.h"

#include "blob.h"
#include "building.h"
#include "bytecode.h"
#include "code_iterators.h"
#include "control_flow.h"
#include "kernel.h"
#include "if_block.h"
#include "inspection.h"
#include "interpreter.h"
#include "list.h"
#include "loops.h"
#include "names.h"
#include "stateful_code.h"
#include "string_type.h"
#include "tagged_value.h"
#include "term.h"
#include "term_list.h"

namespace circa {


struct Writer {
    Stack* stack;
    caValue* bytecode;
    bool skipEffects;
};
    
    
static void bytecode_write_input_instructions(Writer* writer, Term* caller);
static void bytecode_write_output_instructions(Writer* writer, Term* caller, Block* block);
static void bytecode_write_local_reference(Writer* writer, Block* callingBlock, Term* term);

static void bc_append_local_reference_string(caValue* string, const char* bc, int* pc)
{
    string_append(string, "frame:");
    string_append(string, blob_read_u16(bc, pc));
    string_append(string, " reg:");
    string_append(string, blob_read_u16(bc, pc));
}

void bytecode_op_to_string(const char* bc, int* pc, caValue* string)
{
    char op = blob_read_char(bc, pc); 

    switch (op) {
    case bc_End:
        set_string(string, "end");
        break;
    case bc_Pause:
        set_string(string, "pause");
        break;
    case bc_SetNull:
        set_string(string, "set_null ");
        string_append(string, blob_read_u32(bc, pc));
        break;
    case bc_InlineCopy:
        set_string(string, "inline_copy ");
        string_append(string, blob_read_u32(bc, pc));
        bc_append_local_reference_string(string, bc, pc);
        break;
    case bc_LocalCopy:
        set_string(string, "local_copy ");
        string_append(string, blob_read_u32(bc, pc));
        string_append(string, " ");
        string_append(string, blob_read_u32(bc, pc));
        break;
    case bc_NoOp:
        set_string(string, "noop");
        break;
    case bc_EnterFrame:
        set_string(string, "enter_frame");
        break;
    case bc_PopFrame:
        set_string(string, "pop_frame");
        break;
    case bc_PopFrameAndPause:
        set_string(string, "pop_frame_and_pause");
        break;
    case bc_PushFunction:
        set_string(string, "push_function termIndex:");
        string_append(string, blob_read_u32(bc, pc));
        string_append(string, " bc:");
        string_append(string, blob_read_u32(bc, pc));
        break;
    case bc_PushDynamicMethod:
        set_string(string, "push_dyn_method ");
        string_append(string, blob_read_u32(bc, pc));

        for (int i=0; i < c_methodCacheCount; i++) {
            MethodCallSiteCacheLine* line = (MethodCallSiteCacheLine*) &bc[*pc];
            *pc += sizeof(*line);
            string_append(string, "\n  ");
            string_append(string, line->typeId);
            string_append(string, " -> ");
            string_append(string, line->blockIndex);
        }
        break;
    case bc_PushFuncCall:
        set_string(string, "push_func_call ");
        string_append(string, blob_read_u32(bc, pc));
        break;
    case bc_PushFuncApply:
        set_string(string, "push_func_apply ");
        string_append(string, blob_read_u32(bc, pc));
        break;
    case bc_FireNative:
        set_string(string, "fire_native");
        break;
    case bc_PushCase:
        set_string(string, "push_case termIndex:");
        string_append(string, blob_read_u32(bc, pc));
        string_append(string, " blockIndex:");
        string_append(string, blob_read_u32(bc, pc));
        break;
    case bc_PushLoop:
        set_string(string, "push_loop termIndex:");
        string_append(string, blob_read_u32(bc, pc));
        string_append(string, " blockIndex:");
        string_append(string, blob_read_u32(bc, pc));
        string_append(string, " zeroBlockIndex:");
        string_append(string, blob_read_u32(bc, pc));
        if (blob_read_char(bc, pc))
            string_append(string, " :with_output");
        else
            string_append(string, " :no_output");
        break;
    case bc_PushWhile:
        set_string(string, "push_while ");
        string_append(string, blob_read_u32(bc, pc));
        break;
    case bc_PushRequire:
        set_string(string, "push_require ");
        string_append(string, blob_read_u32(bc, pc));
        break;
    case bc_ExitPoint:
        set_string(string, "exit_point");
        break;
    case bc_Return:
        set_string(string, "return ");
        string_append(string, blob_read_u32(bc, pc));
        break;
    case bc_Continue:
        set_string(string, "continue");
        string_append(string, blob_read_char(bc, pc) ? " :loop_output": "");
        string_append(string, " ");
        string_append(string, blob_read_u32(bc, pc));
        break;
    case bc_Break:
        set_string(string, "break");
        string_append(string, blob_read_char(bc, pc) ? " :loop_output": "");
        string_append(string, " ");
        string_append(string, blob_read_u32(bc, pc));
        break;
    case bc_Discard:
        set_string(string, "discard");
        string_append(string, blob_read_char(bc, pc) ? " :loop_output": "");
        string_append(string, " ");
        string_append(string, blob_read_u32(bc, pc));
        break;
    case bc_CaseConditionBool:
        set_string(string, "case_condition_bool input:");
        string_append(string, blob_read_u16(bc, pc));
        string_append(string, "/");
        string_append(string, blob_read_u16(bc, pc));
        string_append(string, " nextBlockBc:");
        string_append(string, blob_read_u32(bc, pc));
        break;
    case bc_LoopConditionBool:
        set_string(string, "loop_condition_bool input:");
        string_append(string, blob_read_u16(bc, pc));
        string_append(string, "/");
        string_append(string, blob_read_u16(bc, pc));
        string_append(string, " bc:");
        string_append(string, blob_read_u32(bc, pc));
        break;
    case bc_Loop:
        set_string(string, "loop");
        break;
    case bc_FinishIteration:
        set_string(string, "finish_iteration ");
        if (blob_read_char(bc, pc))
            string_append(string, " :with_output");
        else
            string_append(string, " :no_output");
        break;
    case bc_FinishBlock:
        set_string(string, "finish_block");
        break;
    case bc_FinishDemandFrame:
        set_string(string, "finish_demand_frame");
        break;
    case bc_DynamicTermEval:
        set_string(string, "dynamic_term_eval");
        break;
    case bc_PopRequire:
        set_string(string, "pop_require");
        break;
    case bc_ErrorNotEnoughInputs:
        set_string(string, "error_not_enough_inputs");
        break;
    case bc_ErrorTooManyInputs:
        set_string(string, "error_too_many_inputs");
        break;
    case bc_InputFromStack:
        set_string(string, "input_from_stack ");
        string_append(string, blob_read_u16(bc, pc));
        string_append(string, " ");
        string_append(string, blob_read_u16(bc, pc));
        break;
    case bc_InputNull:
        set_string(string, "input_null");
        break;
    case bc_InputFromValue:
        set_string(string, "input_from_value ");
        string_append(string, blob_read_u32(bc, pc));
        break;
    case bc_PushNonlocalInput:
        set_string(string, "push_nonlocal_input ");
        string_append(string, blob_read_u32(bc, pc));
        break;
    case bc_InputFromBlockRef:
        set_string(string, "input_from_block_ref ");
        string_append(string, blob_read_u32(bc, pc));
        string_append(string, " ");
        string_append(string, blob_read_u32(bc, pc));
        break;
    case bc_PushExplicitState:
        set_string(string, "push_explicit_state ");
        string_append(string, blob_read_u32(bc, pc));
        break;
    case bc_PopOutput:
        set_string(string, "pop_output ");
        string_append(string, blob_read_u32(bc, pc));
        string_append(string, " ");
        string_append(string, blob_read_u32(bc, pc));
        break;
    case bc_PopOutputNull:
        set_string(string, "pop_output_null ");
        string_append(string, blob_read_u32(bc, pc));
        break;
    case bc_PopOutputsDynamic:
        set_string(string, "pop_outputs_dynamic");
        break;
    case bc_PopExplicitState:
        set_string(string, "pop_explicit_state ");
        string_append(string, blob_read_u32(bc, pc));
        break;
    case bc_SetFrameOutput:
        set_string(string, "set_frame_output ");
        string_append(string, blob_read_u32(bc, pc));
        break;
    case bc_MemoizeCheck:
        set_string(string, "memoize_check");
        break;
    case bc_MemoizeSave:
        set_string(string, "memoize_save");
        break;
    case bc_SetInt:
        set_string(string, "set_int ");
        string_append(string, blob_read_u32(bc, pc));
        string_append(string, " ");
        string_append(string, blob_read_u32(bc, pc));
        break;
    case bc_SetFloat:
        set_string(string, "set_float ");
        string_append(string, blob_read_u32(bc, pc));
        string_append(string, " ");
        string_append_f(string, blob_read_float(bc, pc));
        break;
    case bc_SetTermValue:
        set_string(string, "set_value ");
        string_append(string, blob_read_u32(bc, pc));
        break;

    #define INLINE_MATH_CASE(op, str) \
        case op: \
            set_string(string, str " "); \
            string_append(string, blob_read_u32(bc, pc)); \
            break;

    INLINE_MATH_CASE(bc_Addf, "add_f");
    INLINE_MATH_CASE(bc_Addi, "add_i");
    INLINE_MATH_CASE(bc_Subi, "sub_i");
    INLINE_MATH_CASE(bc_Subf, "sub_f");
    INLINE_MATH_CASE(bc_Multi, "mult_i");
    INLINE_MATH_CASE(bc_Multf, "mult_f");
    INLINE_MATH_CASE(bc_Divi, "div_i");
    INLINE_MATH_CASE(bc_Divf, "div_f");
    INLINE_MATH_CASE(bc_Eqf, "eq_f");
    INLINE_MATH_CASE(bc_Neqf, "neq_f");
    INLINE_MATH_CASE(bc_EqShallow, "eq_shallow");
    INLINE_MATH_CASE(bc_NeqShallow, "neq_shallow");

    case bc_PackState:
        set_string(string, "pack_state ");
        for (int i=0; i < 4; i++) {
            if (i > 0)
                string_append(string, " ");
            string_append(string, blob_read_u16(bc, pc));
        }
        break;
    default:
        set_string(string, "*unrecognized op: ");
        string_append(string, int(op));
    }
}

int bytecode_op_to_term_index(const char* bc, int pc)
{
    char op = blob_read_char(bc, &pc);

    switch (op) {
    case bc_Pause:
        return -1;
    case bc_SetNull:
        return blob_read_u32(bc, &pc);
    case bc_InlineCopy:
        return blob_read_u32(bc, &pc);
    case bc_LocalCopy:
        return -1;
    case bc_NoOp:
        return -1;
    case bc_EnterFrame:
        return -1;
    case bc_PopFrame:
        return -1;
    case bc_PushFunction:
        return blob_read_u32(bc, &pc);
    case bc_PushDynamicMethod:
        return blob_read_u32(bc, &pc);
    case bc_PushFuncCall:
        return blob_read_u32(bc, &pc);
    case bc_PushFuncApply:
        return blob_read_u32(bc, &pc);
    case bc_FireNative:
        return -1;
    case bc_PushCase:
        return blob_read_u32(bc, &pc);
    case bc_PushLoop:
        return blob_read_u32(bc, &pc);
    case bc_PushWhile:
        return blob_read_u32(bc, &pc);
    case bc_PushRequire:
        return blob_read_u32(bc, &pc);
    case bc_ExitPoint:
    case bc_Return:
    case bc_Continue:
    case bc_Break:
    case bc_Discard:
    case bc_CaseConditionBool:
    case bc_LoopConditionBool:
    case bc_Loop:
    case bc_FinishIteration:
    case bc_InputFromStack:
    case bc_InputNull:
    case bc_InputFromValue:
    case bc_PushNonlocalInput:
    case bc_InputFromBlockRef:
    case bc_PushExplicitState:
    case bc_ErrorNotEnoughInputs:
    case bc_ErrorTooManyInputs:
    case bc_PopOutput:
    case bc_PopOutputNull:
    case bc_PopOutputsDynamic:
    case bc_PopExplicitState:
    case bc_MemoizeCheck:
    case bc_MemoizeSave:
    case bc_PackState:
    default:
        return -1;
    }
}

void bytecode_to_string(caValue* bytecode, caValue* string)
{
    std::stringstream strm;

    const char* bc = as_blob(bytecode);
    int pos = 0;

    while (true) {

        int prevPos = pos;

        circa::Value line;
        bytecode_op_to_string(bc, &pos, &line);

        strm << as_cstring(&line) << std::endl;

        switch (blob_read_char(bc, &prevPos)) {
            case bc_End:
                set_string(string, strm.str().c_str());
                return;
        }

        if (pos >= blob_size(bytecode)) {
            strm << "*error: passed end of buffer" << std::endl;
            set_string(string, strm.str().c_str());
            return;
        }
    }
}

void bytecode_to_string_lines(caValue* bytecode, caValue* lines)
{
    int pos = 0;

    set_list(lines, 0);

    while (pos < blob_size(bytecode)) {

        caValue* line = list_append(lines);
        set_string(line, "[");
        string_append(line, pos);
        string_append(line, "] ");

        circa::Value op;
        bytecode_op_to_string(as_blob(bytecode), &pos, &op);

        string_append(line, &op);
    }
}

void bytecode_dump_val(caValue* bytecode)
{
    circa::Value lines;
    bytecode_to_string_lines(bytecode, &lines);

    for (int i=0; i < list_length(&lines); i++) {
        std::cout << as_cstring(list_get(&lines, i)) << std::endl;
    }
}

void bytecode_dump(char* data)
{
    int pc = 0;
    while (data[pc] != bc_End)
        bytecode_dump_next_op(data, &pc);
}

void bytecode_dump_next_op(const char* bc, int *pc)
{
    int originalPc = *pc;
    circa::Value line;
    bytecode_op_to_string(bc, pc, &line);
    printf("[%d] %s\n", originalPc, as_cstring(&line));
}

bool block_contains_memoize(Block* block)
{
    return find_term_with_function(block, FUNCS.memoize) != NULL;
}

static bool term_has_two_int_inputs(Term* term)
{
    return term->numInputs() == 2
        && term->input(0) != NULL
        && term->input(0)->type == TYPES.int_type
        && term->input(1) != NULL
        && term->input(1)->type == TYPES.int_type;
}

static bool type_is_int_or_float(Type* type)
{
    return type == TYPES.float_type || type == TYPES.int_type;
}

static bool term_has_two_floaty_inputs(Term* term)
{
    return term->numInputs() == 2
        && term->input(0) != NULL
        && type_is_int_or_float(term->input(0)->type)
        && term->input(1) != NULL
        && type_is_int_or_float(term->input(1)->type);
}

static char inline_op_match_number_type(Term* term, int whenInts, int whenFloats)
{
    if (term_has_two_int_inputs(term))
        return whenInts;
    if (term_has_two_floaty_inputs(term))
        return whenFloats;
    else
        return 0;
}

static char get_inline_bc_for_term(Term* term)
{
    if (term->function == FUNCS.add)
        return inline_op_match_number_type(term, bc_Addi, bc_Addf);
    else if (term->function == FUNCS.add_i)
        return inline_op_match_number_type(term, bc_Addi, 0);
    else if (term->function == FUNCS.add_f)
        return inline_op_match_number_type(term, bc_Addf, bc_Addf);
    else if (term->function == FUNCS.sub)
        return inline_op_match_number_type(term, bc_Subi, bc_Subf);
    else if (term->function == FUNCS.sub_i)
        return inline_op_match_number_type(term, bc_Subi, 0);
    else if (term->function == FUNCS.sub_f)
        return inline_op_match_number_type(term, bc_Subf, bc_Subf);
    else if (term->function == FUNCS.mult)
        return inline_op_match_number_type(term, bc_Multi, bc_Multf);
#if 0
    else if (term->function == FUNCS.mult_i)
        return inline_op_match_number_type(term, bc_Multi, 0);
    else if (term->function == FUNCS.mult_f)
        return inline_op_match_number_type(term, bc_Multf, bc_Multf);
#endif
    else if (term->function == FUNCS.div)
        return inline_op_match_number_type(term, bc_Divf, bc_Divf);
    else if (term->function == FUNCS.div_i)
        return inline_op_match_number_type(term, bc_Divi, 0);
    else if (term->function == FUNCS.div_f)
        return inline_op_match_number_type(term, bc_Divf, bc_Divf);
    else if (term->function == FUNCS.equals)
        return inline_op_match_number_type(term, bc_EqShallow, bc_Eqf);
    else if (term->function == FUNCS.not_equals)
        return inline_op_match_number_type(term, bc_NeqShallow, bc_Neqf);

    return 0;
}

Block* case_condition_get_next_case_block(Block* currentCase)
{
    Block* ifBlock = get_parent_block(currentCase);
    int caseIndex = case_block_get_index(currentCase) + 1;
    return if_block_get_case(ifBlock, caseIndex);
}

void write_term_call(Writer* writer, Term* term)
{
    INCREMENT_STAT(WriteTermBytecode);

    if (term->function == FUNCS.output) {

        if (term->input(0) == NULL) {
            blob_append_char(writer->bytecode, bc_SetNull);
            blob_append_u32(writer->bytecode, term->index);
            return;
        } else {
            blob_append_char(writer->bytecode, bc_InlineCopy);
            blob_append_u32(writer->bytecode, term->index);
            bytecode_write_local_reference(writer, term->owningBlock,
                term->input(0));
            return;
        }
    }

    else if (term->function == FUNCS.nonlocal) {
        blob_append_char(writer->bytecode, bc_PushNonlocalInput);
        blob_append_u32(writer->bytecode, term->index);
        return;
    }

    else if (term->function == FUNCS.case_condition_bool) {
        blob_append_char(writer->bytecode, bc_CaseConditionBool);
        bytecode_write_local_reference(writer, term->owningBlock, term->input(0));

        Block* nextCase = case_condition_get_next_case_block(term->owningBlock);
        int nextBlockBcIndex = stack_bytecode_create_entry(writer->stack, nextCase);
        blob_append_u32(writer->bytecode, nextBlockBcIndex);
        return;
    }

    else if (term->function == FUNCS.loop_condition_bool) {
        blob_append_char(writer->bytecode, bc_LoopConditionBool);
        bytecode_write_local_reference(writer, term->owningBlock, term->input(0));
        return;
    }

    else if (term->function == FUNCS.dynamic_term_eval) {
        blob_append_char(writer->bytecode, bc_DynamicTermEval);
        bytecode_write_local_reference(writer, term->owningBlock, term->input(0));
        return;
    }

    if (is_exit_point(term)) {
        if (term->function == FUNCS.return_func) {
            blob_append_char(writer->bytecode, bc_Return);
            blob_append_u32(writer->bytecode, term->index);
        } else if (term->function == FUNCS.break_func) {
            blob_append_char(writer->bytecode, bc_Break);
            blob_append_char(writer->bytecode, enclosing_loop_produces_output_value(term));
            blob_append_u32(writer->bytecode, term->index);
        } else if (term->function == FUNCS.continue_func) {
            blob_append_char(writer->bytecode, bc_Continue);
            blob_append_char(writer->bytecode, enclosing_loop_produces_output_value(term));
            blob_append_u32(writer->bytecode, term->index);
        } else if (term->function == FUNCS.discard) {
            blob_append_char(writer->bytecode, bc_Discard);
            blob_append_char(writer->bytecode, enclosing_loop_produces_output_value(term));
            blob_append_u32(writer->bytecode, term->index);
        } else {
            internal_error("unrecognized exit point function");
        }

        return;
    }

    if (is_value(term))
        return;

    char inlineOp = get_inline_bc_for_term(term);
    if (inlineOp != 0) {
        blob_append_char(writer->bytecode, inlineOp);
        blob_append_u32(writer->bytecode, term->index);
        bytecode_write_input_instructions(writer, term);
        return;
    }

    if (term->function == FUNCS.lambda
            || term->function == FUNCS.block_unevaluated) {
        // These funcs have a nestedContents, but shouldn't be evaluated.
        return;
    }

    // 'referenceTargetBlock' is a block that describes the expected inputs & outputs.
    // It might be the exact block that will actually be pushed (such as for PushNested),
    // or it might just be a block that resembles what will be pushed at runtime (such as
    // for PushCase). It also might not be known at all, in the case of dynamic dispatch
    // (Closure calls and dynamic_method), in which case it will remain NULL.
    Block* referenceTargetBlock = NULL;

    if (term->function == FUNCS.func_call) {
        referenceTargetBlock = NULL;
        blob_append_char(writer->bytecode, bc_PushFuncCall);
        blob_append_u32(writer->bytecode, term->index);
    }

    else if (is_dynamic_func_call(term)) {
        referenceTargetBlock = NULL;
        blob_append_char(writer->bytecode, bc_PushFuncCall);
        blob_append_u32(writer->bytecode, term->index);
    }

    else if (term->function == FUNCS.func_apply) {
        referenceTargetBlock = NULL;
        blob_append_char(writer->bytecode, bc_PushFuncApply);
        blob_append_u32(writer->bytecode, term->index);
    }
    
    else if (term->function == FUNCS.dynamic_method) {
        referenceTargetBlock = NULL;
        blob_append_char(writer->bytecode, bc_PushDynamicMethod);
        blob_append_u32(writer->bytecode, term->index);
        blob_append_space(writer->bytecode, c_methodCacheSize);
    }

    else if (term->function == FUNCS.if_block) {
        referenceTargetBlock = term->nestedContents;
        blob_append_char(writer->bytecode, bc_PushCase);
        blob_append_u32(writer->bytecode, term->index);
        Block* firstCaseBlock = if_block_get_case(term->nestedContents, 0);
        u32 blockIndex = stack_bytecode_create_entry(writer->stack, firstCaseBlock);
        blob_append_u32(writer->bytecode, blockIndex);
    }

    else if (term->function == FUNCS.for_func) {
        referenceTargetBlock = term->nestedContents;
        blob_append_char(writer->bytecode, bc_PushLoop);
        blob_append_u32(writer->bytecode, term->index);
        blob_append_u32(writer->bytecode, stack_bytecode_create_entry(writer->stack, term->nestedContents));
        blob_append_u32(writer->bytecode, stack_bytecode_create_entry(writer->stack,
            for_loop_get_zero_block(term->nestedContents)));
        blob_append_char(writer->bytecode, loop_produces_output_value(term) ? 0x1 : 0x0);
    }

    else if (term->function == FUNCS.while_loop) {
        referenceTargetBlock = term->nestedContents;
        blob_append_char(writer->bytecode, bc_PushWhile);
        blob_append_u32(writer->bytecode, term->index);
    }
    
    else if (term->function == FUNCS.closure_block || term->function == FUNCS.function_decl) {
        // Call the function, not nested contents.
        referenceTargetBlock = function_contents(term->function);
        blob_append_char(writer->bytecode, bc_PushFunction);
        blob_append_u32(writer->bytecode, term->index);
        blob_append_u32(writer->bytecode, 0xffffffff);
    }

    else if (term->nestedContents != NULL) {

        internal_error("bytecode_write_term_call: function has nestedContents but isn't recognized as special case");

    } else {

        // If no other case applies, use the term's function.
        referenceTargetBlock = function_contents(term->function);

        if (block_is_evaluation_empty(referenceTargetBlock))
            return;

        blob_append_char(writer->bytecode, bc_PushFunction);
        blob_append_u32(writer->bytecode, term->index);
        blob_append_u32(writer->bytecode, 0xffffffff);
    }

    bytecode_write_input_instructions(writer, term);

    blob_append_char(writer->bytecode, bc_EnterFrame);
    bytecode_write_output_instructions(writer, term, referenceTargetBlock);
    blob_append_char(writer->bytecode, bc_PopFrame);
}

void bytecode_write_input_instruction_block_ref(Writer* writer, Term* input)
{
    blob_append_char(writer->bytecode, bc_InputFromBlockRef);
    blob_append_u32(writer->bytecode, stack_bytecode_create_empty_entry(writer->stack, input->owningBlock));
    blob_append_u32(writer->bytecode, input->index);
}

static void bytecode_write_input_instructions(Writer* writer, Term* caller)
{
    if (is_dynamic_func_call(caller))
        bytecode_write_input_instruction_block_ref(writer, caller->function);

    for (int i=0; i < caller->numInputs(); i++) {
        Term* input = caller->input(i);
        if (input == NULL) {
            blob_append_char(writer->bytecode, bc_InputNull);
        } else if (is_value(input) || input->owningBlock == global_builtins_block()) {
            blob_append_char(writer->bytecode, bc_InputFromValue);
            blob_append_u32(writer->bytecode, i);
        } else {
            bytecode_write_input_instruction_block_ref(writer, input);
            //blob_append_char(bytecode, bc_InputFromStack);
            //bytecode_write_local_reference(bytecode, caller->owningBlock, input);
        }
    }
}

static void bytecode_write_output_instructions(Writer* writer, Term* caller, Block* block)
{
    if (block == NULL) {
        blob_append_char(writer->bytecode, bc_PopOutputsDynamic);
        return;
    }

    int placeholderIndex = 0;

    for (int outputIndex=0;; outputIndex++) {
        Term* output = get_output_term(caller, outputIndex);
        if (output == NULL)
            break;

        // Normal output.

        Term* placeholder = get_output_placeholder(block, placeholderIndex);
        if (placeholder == NULL) {
            blob_append_char(writer->bytecode, bc_PopOutputNull);
            blob_append_u32(writer->bytecode, outputIndex);
        } else {
            blob_append_char(writer->bytecode, bc_PopOutput);
            blob_append_u32(writer->bytecode, placeholderIndex);
            blob_append_u32(writer->bytecode, outputIndex);
        }

        placeholderIndex++;
    }
}

static int get_expected_stack_distance(Block* from, Block* to)
{
    int distance = 0;

    while (from != to) {
        // The one case where the block distance doesn't match the stack frame distance
        // is with an if-block. The if-block has a 'parent' block (that contains each
        // condition) which itself does not get a stack frame.
        if (!is_if_block(from))
            distance++;

        from = get_parent_block(from);
        ca_assert(from != NULL);
    }
    return distance;
}

static void bytecode_write_local_reference(Writer* writer, Block* callingBlock, Term* term)
{
    blob_append_u16(writer->bytecode, get_expected_stack_distance(callingBlock, term->owningBlock));
    blob_append_u16(writer->bytecode, term->index);
}

static void write_pre_exit_pack_state(Writer* writer, Block* block, Term* exitPoint)
{
    // Add PackState ops for each minor block.
    bool anyPackState = false;

    UpwardIterator2 packStateSearch;

    if (exitPoint != NULL) {
        packStateSearch = UpwardIterator2(exitPoint);
        packStateSearch.stopAt(find_block_that_exit_point_will_reach(exitPoint));
    } else {
        packStateSearch = UpwardIterator2(block);
        packStateSearch.stopAt(block);
    }

    for (; packStateSearch; ++packStateSearch) {
        Term* term = packStateSearch.current();

        if (term->function == FUNCS.declared_state) {
            Term* stateResult = NULL;
            if (exitPoint != NULL)
                stateResult = find_name_at(exitPoint, &term->nameValue);
            else
                stateResult = find_name(block, &term->nameValue);

            blob_append_char(writer->bytecode, bc_PackState);
            bytecode_write_local_reference(writer, block, term);
            bytecode_write_local_reference(writer, block, stateResult);
            anyPackState = true;
        }
    }
}

static void write_loop_finish(Writer* writer, Block* block)
{
    // Copy values for looped_inputs.
    for (int i=0;; i++) {
        Term* term = block->get(i);
        if (term->function != FUNCS.looped_input)
            break;

        blob_append_char(writer->bytecode, bc_LocalCopy);
        blob_append_u32(writer->bytecode, term->input(1)->index);
        blob_append_u32(writer->bytecode, term->index);
    }
}

static void write_block_pre_exit(Writer* writer, Block* block, Term* exitPoint)
{
    write_pre_exit_pack_state(writer, block, exitPoint);

    if (block_contains_memoize(block))
        blob_append_char(writer->bytecode, bc_MemoizeSave);
}

void write_block(Writer* writer, Block* block)
{
    bool exitAdded = false;

    if (block_contains_memoize(block))
        blob_append_char(writer->bytecode, bc_MemoizeCheck);

    // Check to just trigger a C override.
    if (get_override_for_block(block) != NULL) {
        blob_append_char(writer->bytecode, bc_FireNative);

    } else {
        
        for (int i=0; i < block->length(); i++) {
            Term* term = block->get(i);
            if (term == NULL)
                continue;

            if (is_exit_point(term) && !is_conditional_exit_point(term)) {
                write_block_pre_exit(writer, block, term);
                write_term_call(writer, term);
                exitAdded = true;
                break;
            }

            if (is_conditional_exit_point(term))
                write_pre_exit_pack_state(writer, block, term);

            write_term_call(writer, term);
        }
    }

    if (!exitAdded)
        write_block_pre_exit(writer, block, NULL);

    if (is_while_loop(block)) {
        write_loop_finish(writer, block);
        blob_append_char(writer->bytecode, bc_Loop);
    }

    if (is_for_loop(block)) {
        blob_append_char(writer->bytecode, bc_FinishIteration);
        blob_append_char(writer->bytecode,
            loop_produces_output_value(block->owningTerm) ? 0x1 : 0x0);
    } else {
        blob_append_char(writer->bytecode, bc_FinishBlock);
    }

    blob_append_char(writer->bytecode, bc_End);
}

void write_on_demand_block(Writer* writer, Term* term, bool thenStop)
{
#if 0
    Block* block = term->owningBlock;

    bool* involvedTerms = (bool*) malloc(sizeof(bool) * block->length());
    memset(involvedTerms, 0, sizeof(bool) * block->length());

    involvedTerms[term->index] = true;

    for (int searchIndex=term->index; searchIndex >= 0; searchIndex--) {

        if (!involvedTerms[searchIndex])
            continue;

        Term* involvedTerm = block->get(searchIndex);

        for (int i=0; i < involvedTerm->numInputs(); i++) {
            Term* input = involvedTerm->input(i);
            if (input == NULL || input->owningBlock != block || is_value(input))
                continue;

            involvedTerms[input->index] = true;
        }
    }

    for (int i=0; i <= term->index; i++) {
        if (involvedTerms[i])
            write_term_call(writer, block->get(i));
    }

    free(involvedTerms);

    blob_append_char(writer->bytecode, bc_SaveInModuleFrames);

    if (thenStop) {
        blob_append_char(writer->bytecode, bc_PopFrameAndPause);
        blob_append_char(writer->bytecode, bc_End);
    } else {

        blob_append_char(writer->bytecode, bc_SetFrameOutput);
        blob_append_u32(writer->bytecode, term->index);

        blob_append_char(writer->bytecode, bc_FinishDemandFrame);
        blob_append_char(writer->bytecode, bc_End);
    }
#endif
}

void writer_setup_from_stack(Writer* writer, Stack* stack)
{
    writer->stack = stack;
}

void bytecode_write_term_call(Stack* stack, caValue* bytecode, Term* term)
{
    Writer writer;
    writer.bytecode = bytecode;
    writer_setup_from_stack(&writer, stack);
    set_blob(bytecode, 0);
    write_term_call(&writer, term);
}

void bytecode_write_block(Stack* stack, caValue* bytecode, Block* block)
{
    Writer writer;
    writer.bytecode = bytecode;
    writer_setup_from_stack(&writer, stack);
    set_blob(bytecode, 0);
    write_block(&writer, block);
}
void bytecode_write_on_demand_block(Stack* stack, caValue* bytecode, Term* term, bool thenStop)
{
    Writer writer;
    writer.bytecode = bytecode;
    writer_setup_from_stack(&writer, stack);
    set_blob(bytecode, 0);
    write_on_demand_block(&writer, term, thenStop);
}

} // namespace circa

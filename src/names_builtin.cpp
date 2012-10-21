// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

// This file was generated using name_tool.py

#include "common_headers.h"
#include "names_builtin.h"

namespace circa {

const char* builtin_name_to_string(int name)
{
    if (name >= name_LastBuiltinName)
        return NULL;

    switch (name) {
    case name_None: return "None";
    case name_Invalid: return "Invalid";
    case name_File: return "File";
    case name_Newline: return "Newline";
    case name_Out: return "Out";
    case name_Unknown: return "Unknown";
    case name_Repeat: return "Repeat";
    case name_Success: return "Success";
    case name_Failure: return "Failure";
    case name_FileNotFound: return "FileNotFound";
    case name_NotEnoughInputs: return "NotEnoughInputs";
    case name_TooManyInputs: return "TooManyInputs";
    case name_ExtraOutputNotFound: return "ExtraOutputNotFound";
    case name_Default: return "Default";
    case name_ByDemand: return "ByDemand";
    case name_Unevaluated: return "Unevaluated";
    case name_InProgress: return "InProgress";
    case name_Lazy: return "Lazy";
    case name_Consumed: return "Consumed";
    case name_Return: return "Return";
    case name_Continue: return "Continue";
    case name_Break: return "Break";
    case name_Discard: return "Discard";
    case name_InfixOperator: return "InfixOperator";
    case name_FunctionName: return "FunctionName";
    case name_TypeName: return "TypeName";
    case name_TermName: return "TermName";
    case name_Keyword: return "Keyword";
    case name_Whitespace: return "Whitespace";
    case name_UnknownIdentifier: return "UnknownIdentifier";
    case name_LookupAny: return "LookupAny";
    case name_LookupType: return "LookupType";
    case name_LookupFunction: return "LookupFunction";
    case name_LookupModule: return "LookupModule";
    case name_Untyped: return "Untyped";
    case name_UniformListType: return "UniformListType";
    case name_AnonStructType: return "AnonStructType";
    case name_StructType: return "StructType";
    case name_NativeModule: return "NativeModule";
    case name_Branch: return "Branch";
    case tok_Identifier: return "tok_Identifier";
    case tok_Name: return "tok_Name";
    case tok_Integer: return "tok_Integer";
    case tok_HexInteger: return "tok_HexInteger";
    case tok_Float: return "tok_Float";
    case tok_String: return "tok_String";
    case tok_Color: return "tok_Color";
    case tok_Bool: return "tok_Bool";
    case tok_LParen: return "tok_LParen";
    case tok_RParen: return "tok_RParen";
    case tok_LBrace: return "tok_LBrace";
    case tok_RBrace: return "tok_RBrace";
    case tok_LBracket: return "tok_LBracket";
    case tok_RBracket: return "tok_RBracket";
    case tok_Comma: return "tok_Comma";
    case tok_At: return "tok_At";
    case tok_Dot: return "tok_Dot";
    case tok_DotAt: return "tok_DotAt";
    case tok_Star: return "tok_Star";
    case tok_Question: return "tok_Question";
    case tok_Slash: return "tok_Slash";
    case tok_DoubleSlash: return "tok_DoubleSlash";
    case tok_Plus: return "tok_Plus";
    case tok_Minus: return "tok_Minus";
    case tok_LThan: return "tok_LThan";
    case tok_LThanEq: return "tok_LThanEq";
    case tok_GThan: return "tok_GThan";
    case tok_GThanEq: return "tok_GThanEq";
    case tok_Percent: return "tok_Percent";
    case tok_Colon: return "tok_Colon";
    case tok_DoubleColon: return "tok_DoubleColon";
    case tok_DoubleEquals: return "tok_DoubleEquals";
    case tok_NotEquals: return "tok_NotEquals";
    case tok_Equals: return "tok_Equals";
    case tok_PlusEquals: return "tok_PlusEquals";
    case tok_MinusEquals: return "tok_MinusEquals";
    case tok_StarEquals: return "tok_StarEquals";
    case tok_SlashEquals: return "tok_SlashEquals";
    case tok_ColonEquals: return "tok_ColonEquals";
    case tok_RightArrow: return "tok_RightArrow";
    case tok_LeftArrow: return "tok_LeftArrow";
    case tok_Ampersand: return "tok_Ampersand";
    case tok_DoubleAmpersand: return "tok_DoubleAmpersand";
    case tok_DoubleVerticalBar: return "tok_DoubleVerticalBar";
    case tok_Semicolon: return "tok_Semicolon";
    case tok_TwoDots: return "tok_TwoDots";
    case tok_Ellipsis: return "tok_Ellipsis";
    case tok_TripleLThan: return "tok_TripleLThan";
    case tok_TripleGThan: return "tok_TripleGThan";
    case tok_Pound: return "tok_Pound";
    case tok_Def: return "tok_Def";
    case tok_Type: return "tok_Type";
    case tok_Begin: return "tok_Begin";
    case tok_Do: return "tok_Do";
    case tok_End: return "tok_End";
    case tok_If: return "tok_If";
    case tok_Else: return "tok_Else";
    case tok_Elif: return "tok_Elif";
    case tok_For: return "tok_For";
    case tok_State: return "tok_State";
    case tok_Return: return "tok_Return";
    case tok_In: return "tok_In";
    case tok_True: return "tok_True";
    case tok_False: return "tok_False";
    case tok_Namespace: return "tok_Namespace";
    case tok_Include: return "tok_Include";
    case tok_And: return "tok_And";
    case tok_Or: return "tok_Or";
    case tok_Not: return "tok_Not";
    case tok_Discard: return "tok_Discard";
    case tok_Null: return "tok_Null";
    case tok_Break: return "tok_Break";
    case tok_Continue: return "tok_Continue";
    case tok_Switch: return "tok_Switch";
    case tok_Case: return "tok_Case";
    case tok_While: return "tok_While";
    case tok_Whitespace: return "tok_Whitespace";
    case tok_Newline: return "tok_Newline";
    case tok_Comment: return "tok_Comment";
    case tok_Eof: return "tok_Eof";
    case tok_Unrecognized: return "tok_Unrecognized";
    case op_NoOp: return "op_NoOp";
    case op_Pause: return "op_Pause";
    case op_SetNull: return "op_SetNull";
    case op_InlineCopy: return "op_InlineCopy";
    case op_CallBranch: return "op_CallBranch";
    case op_DynamicCall: return "op_DynamicCall";
    case op_FireNative: return "op_FireNative";
    case op_CaseBlock: return "op_CaseBlock";
    case op_ForLoop: return "op_ForLoop";
    case op_ExitPoint: return "op_ExitPoint";
    case op_FinishFrame: return "op_FinishFrame";
    case op_FinishLoop: return "op_FinishLoop";
    case op_ErrorNotEnoughInputs: return "op_ErrorNotEnoughInputs";
    case op_ErrorTooManyInputs: return "op_ErrorTooManyInputs";
    case name_LoopProduceOutput: return "LoopProduceOutput";
    case name_FlatOutputs: return "FlatOutputs";
    case name_OutputsToList: return "OutputsToList";
    case name_Multiple: return "Multiple";
    case name_Cast: return "Cast";
    case name_DynamicMethodOutput: return "DynamicMethodOutput";
    case name_FirstStatIndex: return "FirstStatIndex";
    case stat_TermsCreated: return "stat_TermsCreated";
    case stat_TermPropAdded: return "stat_TermPropAdded";
    case stat_TermPropAccess: return "stat_TermPropAccess";
    case stat_InternedNameLookup: return "stat_InternedNameLookup";
    case stat_InternedNameCreate: return "stat_InternedNameCreate";
    case stat_Copy_PushedInputNewFrame: return "stat_Copy_PushedInputNewFrame";
    case stat_Copy_PushedInputMultiNewFrame: return "stat_Copy_PushedInputMultiNewFrame";
    case stat_Copy_PushFrameWithInputs: return "stat_Copy_PushFrameWithInputs";
    case stat_Copy_ListDuplicate: return "stat_Copy_ListDuplicate";
    case stat_Copy_LoopCopyRebound: return "stat_Copy_LoopCopyRebound";
    case stat_Cast_ListCastElement: return "stat_Cast_ListCastElement";
    case stat_Cast_PushFrameWithInputs: return "stat_Cast_PushFrameWithInputs";
    case stat_Cast_FinishFrame: return "stat_Cast_FinishFrame";
    case stat_Touch_ListCast: return "stat_Touch_ListCast";
    case stat_ValueCreates: return "stat_ValueCreates";
    case stat_ValueCopies: return "stat_ValueCopies";
    case stat_ValueCast: return "stat_ValueCast";
    case stat_ValueCastDispatched: return "stat_ValueCastDispatched";
    case stat_ValueTouch: return "stat_ValueTouch";
    case stat_ListsCreated: return "stat_ListsCreated";
    case stat_ListsGrown: return "stat_ListsGrown";
    case stat_ListSoftCopy: return "stat_ListSoftCopy";
    case stat_ListHardCopy: return "stat_ListHardCopy";
    case stat_DictHardCopy: return "stat_DictHardCopy";
    case stat_StringCreate: return "stat_StringCreate";
    case stat_StringDuplicate: return "stat_StringDuplicate";
    case stat_StringResizeInPlace: return "stat_StringResizeInPlace";
    case stat_StringResizeCreate: return "stat_StringResizeCreate";
    case stat_StringSoftCopy: return "stat_StringSoftCopy";
    case stat_StringToStd: return "stat_StringToStd";
    case stat_StepInterpreter: return "stat_StepInterpreter";
    case stat_InterpreterCastOutputFromFinishedFrame: return "stat_InterpreterCastOutputFromFinishedFrame";
    case stat_BranchNameLookups: return "stat_BranchNameLookups";
    case stat_PushFrame: return "stat_PushFrame";
    case stat_LoopFinishIteration: return "stat_LoopFinishIteration";
    case stat_LoopWriteOutput: return "stat_LoopWriteOutput";
    case stat_WriteTermBytecode: return "stat_WriteTermBytecode";
    case stat_DynamicCall: return "stat_DynamicCall";
    case stat_FinishDynamicCall: return "stat_FinishDynamicCall";
    case stat_DynamicMethodCall: return "stat_DynamicMethodCall";
    case stat_SetIndex: return "stat_SetIndex";
    case stat_SetField: return "stat_SetField";
    case name_LastStatIndex: return "LastStatIndex";
    case name_LastBuiltinName: return "LastBuiltinName";
    default: return NULL;
    }
}

} // namespace circa

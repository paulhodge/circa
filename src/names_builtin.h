// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

// This file was generated using name_tool.py

#pragma once

namespace circa {

const int name_None = 0;
const int name_Invalid = 1;
const int name_File = 2;
const int name_Newline = 3;
const int name_Out = 4;
const int name_Unknown = 5;
const int name_Repeat = 6;
const int name_Success = 7;
const int name_Failure = 8;
const int name_FileNotFound = 9;
const int name_NotEnoughInputs = 10;
const int name_TooManyInputs = 11;
const int name_ExtraOutputNotFound = 12;
const int name_Default = 13;
const int name_ByDemand = 14;
const int name_Unevaluated = 15;
const int name_InProgress = 16;
const int name_Lazy = 17;
const int name_Consumed = 18;
const int name_Return = 19;
const int name_Continue = 20;
const int name_Break = 21;
const int name_Discard = 22;
const int name_InfixOperator = 23;
const int name_FunctionName = 24;
const int name_TypeName = 25;
const int name_TermName = 26;
const int name_Keyword = 27;
const int name_Whitespace = 28;
const int name_UnknownIdentifier = 29;
const int name_LookupAny = 30;
const int name_LookupType = 31;
const int name_LookupFunction = 32;
const int name_LookupModule = 33;
const int name_Untyped = 34;
const int name_UniformListType = 35;
const int name_AnonStructType = 36;
const int name_StructType = 37;
const int name_NativeModule = 38;
const int name_Branch = 39;
const int tok_Identifier = 40;
const int tok_Name = 41;
const int tok_Integer = 42;
const int tok_HexInteger = 43;
const int tok_Float = 44;
const int tok_String = 45;
const int tok_Color = 46;
const int tok_Bool = 47;
const int tok_LParen = 48;
const int tok_RParen = 49;
const int tok_LBrace = 50;
const int tok_RBrace = 51;
const int tok_LBracket = 52;
const int tok_RBracket = 53;
const int tok_Comma = 54;
const int tok_At = 55;
const int tok_Dot = 56;
const int tok_DotAt = 57;
const int tok_Star = 58;
const int tok_Question = 59;
const int tok_Slash = 60;
const int tok_DoubleSlash = 61;
const int tok_Plus = 62;
const int tok_Minus = 63;
const int tok_LThan = 64;
const int tok_LThanEq = 65;
const int tok_GThan = 66;
const int tok_GThanEq = 67;
const int tok_Percent = 68;
const int tok_Colon = 69;
const int tok_DoubleColon = 70;
const int tok_DoubleEquals = 71;
const int tok_NotEquals = 72;
const int tok_Equals = 73;
const int tok_PlusEquals = 74;
const int tok_MinusEquals = 75;
const int tok_StarEquals = 76;
const int tok_SlashEquals = 77;
const int tok_ColonEquals = 78;
const int tok_RightArrow = 79;
const int tok_LeftArrow = 80;
const int tok_Ampersand = 81;
const int tok_DoubleAmpersand = 82;
const int tok_DoubleVerticalBar = 83;
const int tok_Semicolon = 84;
const int tok_TwoDots = 85;
const int tok_Ellipsis = 86;
const int tok_TripleLThan = 87;
const int tok_TripleGThan = 88;
const int tok_Pound = 89;
const int tok_Def = 90;
const int tok_Type = 91;
const int tok_Begin = 92;
const int tok_Do = 93;
const int tok_End = 94;
const int tok_If = 95;
const int tok_Else = 96;
const int tok_Elif = 97;
const int tok_For = 98;
const int tok_State = 99;
const int tok_Return = 100;
const int tok_In = 101;
const int tok_True = 102;
const int tok_False = 103;
const int tok_Namespace = 104;
const int tok_Include = 105;
const int tok_And = 106;
const int tok_Or = 107;
const int tok_Not = 108;
const int tok_Discard = 109;
const int tok_Null = 110;
const int tok_Break = 111;
const int tok_Continue = 112;
const int tok_Switch = 113;
const int tok_Case = 114;
const int tok_While = 115;
const int tok_Whitespace = 116;
const int tok_Newline = 117;
const int tok_Comment = 118;
const int tok_Eof = 119;
const int tok_Unrecognized = 120;
const int op_NoOp = 121;
const int op_Pause = 122;
const int op_SetNull = 123;
const int op_InlineCopy = 124;
const int op_CallBranch = 125;
const int op_DynamicCall = 126;
const int op_FireNative = 127;
const int op_CaseBlock = 128;
const int op_ForLoop = 129;
const int op_ExitPoint = 130;
const int op_FinishFrame = 131;
const int op_FinishLoop = 132;
const int op_ErrorNotEnoughInputs = 133;
const int op_ErrorTooManyInputs = 134;
const int name_LoopProduceOutput = 135;
const int name_FlatOutputs = 136;
const int name_OutputsToList = 137;
const int name_Multiple = 138;
const int name_Cast = 139;
const int name_DynamicMethodOutput = 140;
const int name_FirstStatIndex = 141;
const int stat_TermsCreated = 142;
const int stat_TermPropAdded = 143;
const int stat_TermPropAccess = 144;
const int stat_InternedNameLookup = 145;
const int stat_InternedNameCreate = 146;
const int stat_Copy_PushedInputNewFrame = 147;
const int stat_Copy_PushedInputMultiNewFrame = 148;
const int stat_Copy_PushFrameWithInputs = 149;
const int stat_Copy_ListDuplicate = 150;
const int stat_Copy_LoopCopyRebound = 151;
const int stat_Cast_ListCastElement = 152;
const int stat_Cast_PushFrameWithInputs = 153;
const int stat_Cast_FinishFrame = 154;
const int stat_Touch_ListCast = 155;
const int stat_ValueCreates = 156;
const int stat_ValueCopies = 157;
const int stat_ValueCast = 158;
const int stat_ValueCastDispatched = 159;
const int stat_ValueTouch = 160;
const int stat_ListsCreated = 161;
const int stat_ListsGrown = 162;
const int stat_ListSoftCopy = 163;
const int stat_ListHardCopy = 164;
const int stat_DictHardCopy = 165;
const int stat_StringCreate = 166;
const int stat_StringDuplicate = 167;
const int stat_StringResizeInPlace = 168;
const int stat_StringResizeCreate = 169;
const int stat_StringSoftCopy = 170;
const int stat_StringToStd = 171;
const int stat_StepInterpreter = 172;
const int stat_InterpreterCastOutputFromFinishedFrame = 173;
const int stat_BranchNameLookups = 174;
const int stat_PushFrame = 175;
const int stat_LoopFinishIteration = 176;
const int stat_LoopWriteOutput = 177;
const int stat_WriteTermBytecode = 178;
const int stat_DynamicCall = 179;
const int stat_FinishDynamicCall = 180;
const int stat_DynamicMethodCall = 181;
const int stat_SetIndex = 182;
const int stat_SetField = 183;
const int name_LastStatIndex = 184;
const int name_LastBuiltinName = 185;

const char* builtin_name_to_string(int name);

} // namespace circa

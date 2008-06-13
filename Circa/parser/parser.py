#
# parser.py
#
# High-level parser, turns a string into a Circa code object.
#
# Depends on expression.py for expression parsing

from Circa.core import ca_codeunit
from Circa.parser import ast
import expression as _expression_module
import tokens as _tokens_module
from token_definitions import *
import parse_errors

def parseFile(filename, compilationCU=None):
    """
    Parse the given file.
    Returns tuple: (errors, CodeUnit)
       errors is a list of ParseErrors, or an empty list (if none occured)
       codeUnit is an instance of CodeUnit, or None if errors occured
    """

    fileReader = open(filename, 'r')
    fileContents = fileReader.read()
    fileReader.close()

    tokens = _tokens_module.tokenize(fileContents)

    # Scan the token stream for an instance of UNRECOGNIZED token
    # If found, immediately return with this error
    while not tokens.finished():
        token = tokens.consume()
        if token.match == UNRECOGNIZED:
            return ([parse_errors.UnrecognizedToken(token)], None)

    # Reset token stream
    tokens.reset()

    pstate = ParserState(tokens, ca_codeunit.CodeUnit(),
            compilationCU=compilationCU)

    compilation_unit(pstate)

    return (pstate.errors, pstate.codeUnit)


class ParserState(object):
    def __init__(self, tokenSource, resultCodeUnit, compilationCU=None):
        self.tokens = tokenSource
        self.errors = []
        self.codeUnit = resultCodeUnit

        if compilationCU is None:
            self.compilationCU = resultCodeUnit
        else:
            self.compilationCU = compilationCU


def compilation_unit(pstate):

    try:
        resultAst = _expression_module.statement_list(pstate.tokens)
        resultAst.createTerms(
            ast.CompilationContext(pstate.codeUnit, pstate.compilationCU))

        pstate.codeUnit.ast = resultAst

    except parse_errors.ParseError, e:
        pstate.errors.append(e)


def statement(pstate):
    # Dispatch based on next token
    next_token = pstate.tokens.next()

    if next_token.match == TYPE:
        type_decl(pstate)
    elif next_token.match == IF:
        if_statement(pstate)
    # Function decls are now parsed as expressions
    # elif next_token.match == FUNCTION:
    #   function_decl(pstate)
    elif next_token.match == RETURN:
        return_statement(pstate)
    elif next_token.match == RBRACKET:
        close_block(pstate)
    elif next_token.match == NEWLINE:
        new_line(pstate)
    elif next_token.match == COMMENT_LINE:
        comment_line(pstate)
    else:
        expression_statement(pstate)

def expression_statement(pstate):
    try:
        mark = pstate.tokens.markLocation()
        resultAst = _expression_module.parseExpression(pstate.tokens)
        term = resultAst.createTerms(
            ast.CompilationContext(pstate.codeUnit, pstate.compilationCU))

        pstate.codeUnit.statementAsts.append(resultAst)

    except _expression_module.MatchFailed, e:
        pstate.tokens.restoreMark(mark)
        raise parse_errors.ExpectedExpression(pstate.tokens.next())

def new_line(pstate):
    # Ignore NEWLINE
    pstate.tokens.consume(NEWLINE)

    # pstate.codeUnit.statementAsts.append(ast.BlankLine())

def comment_line(pstate):
    comment = pstate.tokens.consume(COMMENT_LINE)
    pstate.codeUnit.statementAsts.append(ast.IgnoredSyntax(comment))


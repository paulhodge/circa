#
# All pieces of data in Circa are wrapped in Terms.
#
# This class should be as minimal as possible.
# 
# When possible, code for manimulating or comparing terms should be placed elsewhere,
# such as in CodeUnit

import pdb

import builtins, ca_function, ca_type
from Circa.common import debug

nextGlobalID = 1

class Term(object):
    def __init__(self, branch):
        "Don't use this constructor directly, use CodeUnit.createTerm instead"

        debug._assert(branch is not None)

        self.owningBranch = branch
   
        # Inputs, a list of terms
        self.inputs = []

        # Function
        self.functionTerm = None

        # A set of terms that are using us. This includes terms that
        # have us as an input, and terms that have us as the function.
        self.users = set()

        # The output value of this term.
        self.cachedValue = None

        # True if 'cachedValue' is out of date
        self.needsUpdate = True

        # True if this term has been evaluated and may be accessed
        self.outputReady = False

        # Persistent state.
        self.state = None

        # Inner branches; this can be considered part of the term's state
        self.branches = []

        # Pointers to other terms; this can be considered part of the
        # term's state
        self.termPointers = []

        # The CodeUnit object that owns us
        self.codeUnit = None
        
        # This structure gives hints on how to produce source code for this term
        self.termSyntaxHints = TermSyntaxHints()

        self.givenName = None
        self.debugName = None

        self.executionContext = TermExecutionContext(self)
   
        global nextGlobalID
        self.globalID = nextGlobalID
        nextGlobalID += 1

    def numInputs(self):
        return len(self.inputs)

    def getInput(self, inputIndex):
        "Return an input term"
        return self.inputs[inputIndex]
  
    def getType(self):
        "Returns this term's output type"
        return ca_function.outputType(self.functionTerm)

    def getIdentifier(self):
        return self.codeUnit.getIdentifier(self)

    name = getIdentifier

    def getNames(self):
        return self.codeUnit.getNames(self)

    def value(self):
        return self.cachedValue

    def isConstant(self):
        "Returns true if this term is a constant term"
        return self.functionTerm.functionTerm is builtins.CONST_GENERATOR

    def isVariable(self):
        "Returns true if this term is a variable term"
        return self.functionTerm.functionTerm is builtins.VARIABLE_GENERATOR

    def toShortString(self):
        return ca_type.toShortString(self.getType())(self)

    def iterateInnerTerms(self):
        return ca_type.iterateInnerTerms(self.getType())(self)

    def execute(self):
        "Calls our function and updates our output value"
        self.needsUpdate = False

        for input in self.inputs:
            input.requestUpdate()
        self.functionTerm.requestUpdate()
        
        ca_function.evaluateFunc(self.functionTerm)(self.executionContext)
        self.outputReady = True

    def execute_trace(self):
        """
        Calls our function using the Python debugger (pdb). Useful for
        debugging.
        """
        pdb.runcall(ca_function.evaluateFunc(self.functionTerm), self.executionContext)
        self.needsUpdate = False

    def requestUpdate(self):
        """
        Make sure 'term' is up-to-date. If it is not, we make sure that
        term's inputs are all up-to-date, and then update 'term'. This
        request may propogate upward.
        """
        if self.needsUpdate:

            self.execute()
            self.needsUpdate = False

    def invalidate(self):
        """
        Mark this term as 'needsUpdate'. This call propogates downward
        to all the users of this term.
        """

        # If needsUpdate is already true, don't do anything.
        if self.needsUpdate:
            return

        debug._assert(not self in self.users)

        self.needsUpdate = True

        for user in self.users:
            user.invalidate()
  
    # value accessors
    def __int__(self):
        return int(self.cachedValue)
  
    def __float__(self):
        return float(self.cachedValue)
  
    def __str__(self):
        return str(self.cachedValue)

class TermSyntaxHints(object):
    __slots__ = ['functionName', 'infix', 'rightArrow', 'namedInputs',
            'preInputWhitespace', 'postInputWhitespace',
            'multilineString']

    def __init__(self):
        self.functionName = None
        self.infix = False
        self.rightArrow = False
        self.namedInputs = {}
        self.preInputWhitespace = []
        self.postInputWhitespace = []
        self.multilineString = False

class TermExecutionContext(object):
    """
    This object is provided to evaluate functions. The purpose is to be
    able to track when these calls occur, and hope to provide replacements
    for them.
    """

    def __init__(self, term):
        self.targetTerm = term
    def inputTerm(self, index):
        return self.targetTerm.getInput(index)
    def input(self, index):
        return self.targetTerm.getInput(index).cachedValue
    def inputs(self):
        return [self.input(n) for n in range(self.numInputs())]
    def state(self):
        return self.targetTerm.state
    def caller(self):
        return self.targetTerm
    def numInputs(self):
        return len(self.targetTerm.inputs)
    def codeUnit(self):
        return self.targetTerm.codeUnit
    def result(self):
        return self.targetTerm.cachedValue
    def setResult(self, value):
        self.targetTerm.cachedValue = value

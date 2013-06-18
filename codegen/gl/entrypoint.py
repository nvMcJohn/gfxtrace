
kInOutValues = ( "in", "out" )
kParamTypeValues = ( "array", "value", "reference" )

kCallingConvention = "APIENTRY"

# -------------------------------------------------------------------------------------------------
# -------------------------------------------------------------------------------------------------
# -------------------------------------------------------------------------------------------------
def tupleAsCDataDecl(tpl):
    assert(len(tpl) >= 2)

    name = tpl[0]
    # If we have an array of stuff, need to extract that.
    if "[" in tpl[1]:
        firstBracket = tpl[1].find("[")
        dataType = tpl[1][:firstBracket]
        arrayCount = tpl[1][firstBracket:]
        return "%s mData_%s%s;" % (dataType, name, arrayCount)
    else:
        dataType = tpl[1]
        return "%s mData_%s;" % (dataType, name)
    assert(0)

# -------------------------------------------------------------------------------------------------
# -------------------------------------------------------------------------------------------------
# -------------------------------------------------------------------------------------------------
class Parameter(object):
    def __init__(self, args):
        self.name = args[0]
        self.abstractType = args[1]
        self.inout = args[2]
        self.paramType = args[3]

        if self.paramType == 'array':
            self.compSize = args[4]
            assert(self.compSize[0] == '[' and self.compSize[-1] == ']')
        else:
            self.compSize = None

        self.realType = None
        
        # Sanity checking types
        assert(self.inout in kInOutValues)
        assert(self.paramType in kParamTypeValues)

    def __str__(self):
        return "%s %s" % (self.realType, self.name)
    __repr__ = __str__

    def ResolveType(self, typemap):
        assert(self.abstractType in typemap)
        self.realType = typemap[self.abstractType]

    # Codegen things here.
    # ---------------------------------------------------------------------------------------------
    # ---------------------------------------------------------------------------------------------
    def asCtype(self):
        if self.paramType == "value":
            return self.realType
        return "%s*" % self.realType

    def asDeclaration(self):
        return "%s %s" % (self.asCtype(), self.name)

# -------------------------------------------------------------------------------------------------
# -------------------------------------------------------------------------------------------------
# -------------------------------------------------------------------------------------------------
class EntryPoint(object):
    def __init__(self, name, args):
        self.name = name
        self.args = args

        self.returnType = None
        self.params = {}

        self.requiresPublicReal = False
        self.requiresStaticResolution = False

        self.singleState = None
        self.multiState = None

    def parse_param(self, args):
        p = Parameter(args)
        assert(p.name in self.args)
        self.params[p.name] = p

    def parse_return(self, args):
        assert(len(args) == 1)
        # Make a fake parameter.
        self.returnType = Parameter(["return", args[0], "out", "value"])

    def ResolveTypes(self, typemap):
        # First, hit the return value
        self.returnType.ResolveType(typemap)

        # Then, hit the arguments
        for param in self.params.itervalues():
            param.ResolveType(typemap)
        
    def SetMultiState(self, controllerAbstractType, allowableValuesAndTypes):
        self.multiState = MultiStateController(controllerAbstractType, allowableValuesAndTypes)

    # Codegen things here.
    # ---------------------------------------------------------------------------------------------
    # ---------------------------------------------------------------------------------------------
    def asData(self, isDeclaration):
        # Unlike other as functions, this returns a list of strings
        if isDeclaration:
            retList = []
            if self.multiState:
                for vt in self.multiState.valuesAndTypes:
                    retList.append(tupleAsCDataDecl(vt))
            elif self.singleState:
                assert(0)
            else:
                assert(0)
            return retList
        else:
            retList = []
            if self.multiState:
                for vt in self.multiState.valuesAndTypes:
                    retList.append(vt)
            elif self.singleState:
                assert(0)
            else:
                assert(0)
            return retList
        assert(0)

    def asGlName(self, prefix=None):
        if prefix:
            return "%s_gl%s" % (prefix, self.name)
        return "gl%s" % self.name

    def asFunction(self, isDeclaration):
        if isDeclaration:
            return self.__funcDeclaration("")
        assert(0)

    def asHookFunction(self, isDeclaration):
        if isDeclaration:
            return self.__funcDeclaration("hook_", kCallingConvention)
        assert(0)

    def asFunctionPointer(self, qualifier=None, callingConvention=None, prefix=None, initialValue=None):
        qualifier = "" if qualifier is None else ("%s " % qualifier)
        callingConvention = "" if callingConvention is None else ("%s " % callingConvention)
        prefix = "" if prefix is None else ("%s_" % prefix)
        funcName = "%s%s" % (prefix, self.asGlName())
        initialValue = "" if initialValue is None else (" = %s" % initialValue)

        argsStr = ", ".join([self.params[arg].asDeclaration() for arg in self.args])
        
        return "%s%s (%s*%s)(%s)%s;" % (qualifier, self.returnType.asCtype(), callingConvention, funcName, argsStr, initialValue)

    def asPointerToFunction(self, isDeclaration):
        qualifier = "" if self.requiresPublicReal else "static "
        initialVal = "NULL"
        if self.requiresStaticResolution:
            initialVal = self.asGlName()
        
        argsStr = ", ".join([self.params[arg].asDeclaration() for arg in self.args])
        if isDeclaration:
            return "%s%s (%s *%s)(%s) = %s;" % (qualifier, self.returnType.asCtype(), kCallingConvention, self.asRealPointerName(), argsStr, initialVal)
        assert(0)

    def asRealPointerName(self):
        return "gReal_%s" % self.asGlName()

    def __funcDeclaration(self, funcPrefix, callConvention=None):
        argsStr = ", ".join([self.params[arg].asDeclaration() for arg in self.args])
            
        if callConvention:
            return "%s %s %s%s(%s);" % (self.returnType.asCtype(), callConvention, funcPrefix, self.asGlName(), argsStr)
        return "%s %s%s(%s);" % (self.returnType.asCtype(), funcPrefix, self.asGlName(), argsStr)

    def __str__(self):
        return "%s(%s)" % (self.name, ",".join(self.args))
    __repr__ = __str__

# -------------------------------------------------------------------------------------------------
class MultiStateController(object):
    def __init__(self, abstractParamName, valuesAndTypes):
        self.abstractParamName = abstractParamName
        self.valuesAndTypes = valuesAndTypes

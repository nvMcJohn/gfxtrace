
import apiobjects
import spec
import entrypoints

# -------------------------------------------------------------------------------------------------
def GenerateCode(opts=None):
    fullSpec = spec.getSpecData(True)

    allEntryPoints, functionForwardTable = entrypoints.annotate(fullSpec["spec"], fullSpec["typemap"])
    return __generateCode(allEntryPoints, functionForwardTable, fullSpec["typemap"])

# -------------------------------------------------------------------------------------------------
# -------------------------------------------------------------------------------------------------
# -------------------------------------------------------------------------------------------------
def __generateCode(allEntryPoints, functionForwardingTable, typemap):
    retVal = {}
    retVal["hooks"] = __generateHooks(allEntryPoints, functionForwardingTable, typemap)
    retVal["serializers"] = __generateSerializers(allEntryPoints, functionForwardingTable, typemap)
    retVal["pointerHelpers"] = __generatePointerHelpers(allEntryPoints, functionForwardingTable, typemap)
    retVal["objects"] = __generateObjects(allEntryPoints, functionForwardingTable, typemap)

    print "\n".join(retVal["objects"]["decl"])
    print "\n".join(retVal["objects"]["defn"])

    return retVal

# -------------------------------------------------------------------------------------------------
def __generateHooks(allEntryPoints, functionForwardingTable, typemap):
    return {}

# -------------------------------------------------------------------------------------------------
def __generateSerializers(allEntryPoints, functionForwardingTable, typemap):
    return {}

# -------------------------------------------------------------------------------------------------
def __generatePointerHelpers(allEntryPoints, functionForwardingTable, typemap):
    return {}

# -------------------------------------------------------------------------------------------------
def __generateObjects(allEntryPoints, functionForwardingTable, typemap):
    retDict = {}
    retDict["decl"] = []
    retDict["defn"] = []

    for object in apiobjects.AllAPIObjects():
        retDict["decl"].append("class %s {" % object.__name__)
        retDict["decl"].append("public:")

        allData = []
        for stateFunc in object.StateFunctions:
            allData.extend(allEntryPoints[stateFunc].asData(True))
        allData = sorted(set(allData))

        # First, do the declarations.
        if object.HasType:
            retDict["decl"].append("\t%s(GLenum _type=GL_NONE)" % object.__name__)
        else:
            retDict["decl"].append("\t%s();" % object.__name__)
        retDict["decl"].append("\t~%s();" % object.__name__)
        retDict["decl"].append("")
        
        if object.HasType:
            retDict["decl"].append("\tvoid CheckOrSetType(GLenum _type);")
        retDict["decl"].append("\tvoid Write(FileLike* _out) const;")
        retDict["decl"].append("\tvoid Read(FileLike* _in);")
        retDict["decl"].append("\tGLuint Create(const GLTrace* _trace) const;")
        retDict["decl"].append("")

        if hasattr(object, "BindFunction"):
            retDict["decl"].append("\t// Bind Function")
            retDict["decl"].append("\t%s" % allEntryPoints[object.BindFunction].asFunction(True))
            retDict["decl"].append("")

        retDict["decl"].append("\t// State Function(s)")
        for stateFunc in object.StateFunctions:
            retDict["decl"].append("\t%s" % allEntryPoints[stateFunc].asFunction(True))
        retDict["decl"].append("")

        retDict["decl"].append("\t// Data Function(s)")
        for dataFunc in object.DataFunctions:
            retDict["decl"].append("\t%s" % allEntryPoints[dataFunc].asFunction(True))
        retDict["decl"].append("")

        retDict["decl"].append("private:")
        retDict["decl"].append("\t// Data")
        if object.HasType:
            retDict["decl"].append("\tGLenum mType;")
        for datum in allData:
            retDict["decl"].append("\t%s" % datum)
        retDict["decl"].append("")
        retDict["decl"].append("\tManualConstruct();")
        retDict["decl"].append("\tManualDestruct();")

        retDict["decl"].append("};")

        allData = []
        for stateFunc in object.StateFunctions:
            allData.extend(allEntryPoints[stateFunc].asData(False))
        allData = sorted(set(allData), key=lambda x: "%s %s" % (x[1], x[0]))

        # Now do the definitions.
        firstUsedData = -1
        if object.HasType:
            retDict["defn"].append("%s::%s(GLenum _type)" % (object.__name__, object.__name__))
        else:
            retDict["defn"].append("%s::%s()" % (object.__name__, object.__name__))
        lineStartChar = ":"
        if object.HasType:
            retDict["defn"].append("%s %s(_type)" % (lineStartChar, "mType"))
        elif len(allData):
            for i, datum in enumerate(allData):
                if len(datum) >= 3:
                    firstUsedData = i
                    break
            retDict["defn"].append("%s mData_%s(%s)" % (lineStartChar, allData[firstUsedData][0], allData[firstUsedData][2]))
        lineStartChar = ","

        for datum in allData[firstUsedData + 1:]:
            if len(datum) >= 3:
                retDict["defn"].append("%s mData_%s(%s)" % (lineStartChar, datum[0], datum[2]))
        retDict["defn"].append("{")
        retDict["defn"].append("\tManualConstruct();")
        retDict["defn"].append("}")

        
    return retDict
        




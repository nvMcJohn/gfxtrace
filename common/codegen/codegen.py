# Copyright (c) 2013, NVIDIA CORPORATION. All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#  * Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
#  * Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
#  * Neither the name of NVIDIA CORPORATION nor the names of its
#    contributors may be used to endorse or promote products derived
#    from this software without specific prior written permission.
# 
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ``AS IS'' AND ANY
# EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
# PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
# CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
# EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
# PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
# PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
# OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

import inspect
import re
import string

kCallingConvention = "APIENTRY"
kDeserializePrefix = "deserialize_"
kDeterminePointerLengthPrefix = "determinePointerLength_"
kHookedPrefix = "hooked_"
kGetDynamicProcAddress = "wglGetProcAddress"
kGetStaticProcAddress = "GetProcAddress"
kGlobalState = "GlobalState"
kRealPrefix = "gReal_"
kDataPacketStructName = "SSerializeDataPacket"

# -------------------------------------------------------------------------------------------------
# -------------------------------------------------------------------------------------------------
# -------------------------------------------------------------------------------------------------
class Argument:
    def __init__(self, ctype, name, isPointer, isPointerOrOffset):
        self.ctype = ctype
        self.name = name
        self.isPointer = isPointer
        self.isPointerOrOffset = isPointerOrOffset

        if "*" not in self.ctype:
            self.ctypeUnderlyingType = self.ctype
        else:
            self.ctypeUnderlyingType = self.ctype.replace("*", "", 1)

    @classmethod 
    def FromPythonFunctionArg(cls, funcArg, pointerOrOffset):
        argPieces = funcArg.split("_")
        name = argPieces[-1]
        ctype = " ".join(argPieces[:-1])
        ctype = ctype.replace(" ptr", "*")
        
        return cls(ctype, name, "*" in ctype, pointerOrOffset == name)

    def __str__(self):
        return "%s %s" % (self.ctype, self.name)

    def asDeterminePointerLengthFunc(self, functionName):
        return "%s%s_%s" % (kDeterminePointerLengthPrefix, functionName, self.name)

    @property
    def asDataDeclaration(self):
        return "%s %s" % (self.ctype, self.name)

    @property
    def isConst(self):
        return "const " in self.ctype

    @property
    def lvaluetype(self):
        return self.ctype.replace("const ", "")

    @property
    def pointerOrOffsetName(self):
        return "isPointer_%s" % self.name


gImmediateFuncRE = re.compile(r'gl(Color|Normal|TexCoord|Vertex)(\d+)(b|d|f|i|s|ub|ui|us)v')
gRectFuncRE = re.compile(r'glRect(d|f|i|s)v')
gRasterPosFuncRE = re.compile(r'glRasterPos(\d+)(d|f|i|s)v')
gSingleValueFuncRE = re.compile(r'gl(EdgeFlag|Index)(d|f|i|s|ub)?v')
gMatrixFuncRE = re.compile(r'gl(Load|Mult)Matrix(d|f)')
gVertexDataFuncRE = re.compile(r'gl(Color|EdgeFlag|Normal|TexCoord|Vertex|VertexAttrib)Pointer')
gIndexDataFuncRE = re.compile(r'gl(Index)Pointer')
gEvalCoordFuncRE = re.compile(r'gl(EvalCoord)(\d+)(d|f)v')
gGenFuncRE = re.compile(r'glGen(\w+)')
gFogFuncRE = re.compile(r'glFog(f|i)v')
gGetFuncRE = re.compile(r'glGet(Boolean|Double|Float|Integer)v')
gGetLightOrMaterialFuncRE = re.compile(r'glGet(Light|Material)(f|i)v')
# -------------------------------------------------------------------------------------------------
# -------------------------------------------------------------------------------------------------
# -------------------------------------------------------------------------------------------------
class GLFunction:
    def __init__(self, returnType, callConvention, name, args, isState, needsManualState, needsManualDetour, needsStaticHook, needsPublicReal, alias, multiState, supported, needsManualRestore, needsManualReplay):
        self.returnType = returnType
        self.callConvention = callConvention
        self.name = name
        self.args = args
        self.isState = isState
        self.needsManualState = needsManualState
        self.needsManualDetour = needsManualDetour
        self.needsManualReplay = needsManualReplay
        self.needsManualRestore = needsManualRestore
        self.needsStaticHook = needsStaticHook
        self.needsPublicReal = needsPublicReal
        self.alias = alias
        self.multiState = multiState
        self.supported = supported
       
    def asDataStructFunctionArgs(self, varName):
        return ", ".join(["%s.%s.%s" % (varName, self.asDataStructMemberName, arg.name) for arg in self.args])

    def asDeterminePointerLengthBody(self, argNum):
        if self.args[0].ctype == "GLsizei" and self.args[0].name == "n" and "void" not in self.args[argNum].ctype:
            return "return (size_t)(%s * sizeof(%s));" % (self.args[0].name, self.args[argNum].ctypeUnderlyingType)
        m = gImmediateFuncRE.match(self.name)
        if m:
            return "return (size_t)(%s * sizeof(%s));" % (m.group(2), self.args[argNum].ctypeUnderlyingType)
        m = gRectFuncRE.match(self.name)
        if m:
            return "return (size_t)(2 * sizeof(%s));" % (self.args[argNum].ctypeUnderlyingType)
        m = gRasterPosFuncRE.match(self.name)
        if m:
            return "return (size_t)(%s * sizeof(%s));" % (m.group(1), self.args[argNum].ctypeUnderlyingType)
        m = gSingleValueFuncRE.match(self.name)
        if m:
            return "return (size_t)(1 * sizeof(%s));" % (self.args[argNum].ctypeUnderlyingType)
        m = gMatrixFuncRE.match(self.name)
        if m:
            return "return (size_t)(16 * sizeof(%s));" % (self.args[argNum].ctypeUnderlyingType)
        m = gVertexDataFuncRE.match(self.name)
        m2 = gIndexDataFuncRE.match(self.name)
        if m or m2:
            raise NotImplementedError("Vertex and Index size functions need manual bodies")
            
        m = gEvalCoordFuncRE.match(self.name)
        if m:
            return "return (size_t)(%s * sizeof(%s));" % (m.group(2), self.args[argNum].ctypeUnderlyingType)
        m = gGenFuncRE.match(self.name)
        if m and self.args[0].ctype == "GLsizei":
            return "return (size_t)(%s * sizeof(%s));" % (self.args[0].name, self.args[argNum].ctypeUnderlyingType)
        m = gFogFuncRE.match(self.name)
        if m:
            return "return (size_t)((%s == GL_FOG_COLOR ? 4 : 1) * sizeof(%s));" % (self.args[0].name, self.args[argNum].ctypeUnderlyingType)
        m = gGetFuncRE.match(self.name)
        if m:
            return "return (size_t)(GLenumToParameterCount(%s) * sizeof(%s));" % (self.args[0].name, self.args[argNum].ctypeUnderlyingType)
        m = gGetLightOrMaterialFuncRE.match(self.name)
        if m:
            return "return (size_t)(GLenumToParameterCount(%s) * sizeof(%s));" % (self.args[1].name, self.args[argNum].ctypeUnderlyingType)

        raise NotImplementedError("couldn't auto determine length for this parameter")

    def asDetouredFunction(self, isDefinition):
        ''' Get the detoured function. If isDefinition is true, will get the whole body, otherwise will just get a declaration. '''
        if isDefinition:
            callName = self.name
            if self.alias is not None:
                callName = self.alias
            # Note with callName: Currently always calls the aliased real function, but then sends the aliased calls to 
            # the function we're aliasing.
            lines = []
            lines.append("%s %s %s(%s)" % (self.returnType, self.callConvention, self.asDetouredName, self.argsAsStr))
            lines.append("{")
            if self.returnType == "void":
                lines.append("\t%s(%s);" % (self.asRealPointerName, self.argsForPassingAsStr))
            else:
                lines.append("\tauto retVal = %s(%s);" % (self.asRealPointerName, self.argsForPassingAsStr))
            lines.append("\tif (!gContextState->CheckOwnerThreadId())")
            if self.returnType == "void":
                lines.append("\t\treturn;")
            else:
                lines.append("\t\treturn retVal;")
            if self.alias is not None:
                lines.append("\t// NOTE: Calling aliased function, see functionhooks.py for alias define!")
            if self.supported:
                lines.append("\tif (gIsRecording)")
                lines.append("\t\t%s::%s(%s).Write(&FileLike(gMessageStream));" % (kDataPacketStructName, callName, self.argsForPassingAsStr))
                if self.isState:
                    lines.append("\tgContextState->%s(%s);" % (callName, self.argsForPassingAsStateStr))
                if self.returnType != "void":
                    lines.append("\treturn retVal;")
            else:
                lines.append("\t// Unsupported function, Error once.")
                lines.append('\tOnce(TraceError("%s was called, but is unsupported by glTrace--please update the trace tool."));' % callName)
                if self.returnType != "void":
                    lines.append("\treturn retVal;")

            lines.append("}")

            return "\n".join(lines)

        return "%s %s %s(%s);" % (self.returnType, self.callConvention, self.asDetouredName, self.argsAsStr)

    def asRealPointerData(self, isDefinition):
        ''' Get the data declaration for the real function pointer. '''
        if self.needsStaticHook:
            pointsTo = self.name
        else:
            pointsTo = "NULL"
        
        if isDefinition:
            if self.needsManualDetour or self.needsPublicReal:
                visibility = ""
            else:
                visibility = "static "
            return "%s%s (%s * %s)(%s) = %s;" % (visibility, self.returnType, self.callConvention, self.asRealPointerName, self.argsAsStr, pointsTo)

        return "extern %s (%s * %s)(%s);" % (self.returnType, self.callConvention, self.asRealPointerName, self.argsAsStr)

    def asSerializeStruct(self, isDefinition):
        ''' Get the structure that defines the data necessary for a single command of this type. '''
        if isDefinition:
            retStr = "struct { %s } %s" % ( self.argsAsDataStruct, self.asDataStructMemberName )
        else:
            assert(False and "TODO")
        return retStr

    def asStateStruct(self, isDefinition):
        ''' Get the structure that defines the data. Output onto a single line. '''
        if isDefinition:
            dataStructStr = ""
            if self.multiState is not None:
                dataStructStr = self.multiStateAsDataStruct
            else:
                dataStructStr = self.argsAsDataStruct
            retStr = "struct { %s } %s" % ( dataStructStr, self.asDataStructMemberName )
        else:
            assert(False and "TODO")
        return retStr


    def canAutoDeterminePointerLength(self, argNum):
        try:
            self.asDeterminePointerLengthBody(argNum)
        except NotImplementedError:
            return False
        return True

    @property
    def asDataName(self):
        return "EST%sData" % self.name

    @property
    def asDataStructMemberName(self):
        return "mData_%s" % self.name

    @property
    def asDetouredName(self):
        return "%s%s" % (kHookedPrefix, self.name)

    @property
    def asRealPointerCast(self):
        return "%s (%s *)(%s)" % (self.returnType, self.callConvention, self.argsAsStr)

    @property
    def asRealPointerName(self):
        ''' Get the name of the real pointer, useful for calling. '''
        return "%s%s" % (kRealPrefix, self.name)

    @property
    def argsAsStr(self):
        return ", ".join([str(arg) for arg in self.args])

    @property
    def argsAsStateStr(self):
        if self.needsManualState and self.returnType != "void":
            return ", ".join(["%s _retVal" % self.returnType] + [str(arg) for arg in self.args])
        return self.argsAsStr

    @property
    def argsAsDataStruct(self):
        if self.hasAnyPointerOrOffsets:
            declList = []
            for arg in self.args:
                if arg.isPointerOrOffset:
                    declList.append("bool %s;" % arg.pointerOrOffsetName)
                declList.append("%s;" % arg.asDataDeclaration)
        else:
            declList = ["%s;" % arg.asDataDeclaration for arg in self.args]
        return " ".join(declList)

    @property
    def argsForPassingAsStr(self):
        return ", ".join([arg.name for arg in self.args])

    @property
    def argsForPassingAsStateStr(self):
        if self.needsManualState and self.returnType != "void":
            return ", ".join(["retVal"] + [arg.name for arg in self.args])
        return self.argsForPassingAsStr

    @property
    def hasAnyPointers(self):
        anyPointers = False
        for arg in self.args:
            if arg.isPointer:
                anyPointers = True
        return anyPointers

    @property
    def hasAnyPointerOrOffsets(self):
        anyPointerOrOffsets = False
        for arg in self.args:
            if arg.isPointerOrOffset:
                anyPointerOrOffsets = True
        return anyPointerOrOffsets

    @property
    def multiStateAsDataStruct(self):
        assert(self.multiState)
        tmpList = []
        for e in self.multiState[1]:
            if isinstance(e, (str, unicode)):
                tmpList.append("%s data_%s;" % (self.multiState[2], e))
            else:
                tmpList.append("%s data_%s;" % (e[1], e[0]))
        return " ".join(tmpList)

    def __str__(self):
        return "%s %s %s(%s)" % (self.returnType, self.callConvention, self.name, self.argsAsStr)

    @classmethod 
    def FromPythonFunction(cls, func):
        (name, funcGuts) = func
        returnType = getattr(funcGuts, 'returntype', 'void')
        returnType = returnType.replace("_", " ")
        returnType = returnType.replace(" ptr", "*")

        isState = getattr(funcGuts, 'isState', False)
        needsManualState = getattr(funcGuts, 'manual_state', False)
        needsManualRestore = getattr(funcGuts, 'manual_restore', False)
        needsManualReplay = getattr(funcGuts, 'manual_replay', False)
        needsManualDetour = getattr(funcGuts, 'manual_detour', False)
        needsStaticHook = getattr(funcGuts, 'static_hook', False)
        needsPublicReal = getattr(funcGuts, 'public_real', False)
        alias = getattr(funcGuts, 'alias', None)
        multiState = getattr(funcGuts, 'multi_state', None)
        if alias is not None:
            try:
                # Assume they gave us a function object
                alias = alias.__name__
            except:
                # But a string is fine, too.
                pass
        supported = not getattr(funcGuts, 'unsupported', False)
        callConvention = kCallingConvention

        pointerOrOffset = getattr(funcGuts, 'pointer_or_offset', None)

        args = [Argument.FromPythonFunctionArg(arg, pointerOrOffset) for arg in inspect.getargspec(funcGuts)[0]]
        return cls(returnType, callConvention, name, args, isState, needsManualState, needsManualDetour, needsStaticHook, needsPublicReal, alias, multiState, supported, needsManualRestore, needsManualReplay)

class GLData:
    def __init__(self, name, ctype):
        self.name = name
        self.ctype = ctype

    @property
    def asDeclaration(self):
        if "[" in self.ctype:
            # Arrays handled slightly differently. Dimensionality doesn't matter here.
            (ctype_simple, arrayDims) = self.ctype.split("[", 1)
            return "%s mData_%s[%s" % (ctype_simple, self.name, arrayDims)
        return "%s mData_%s" % (self.ctype, self.name)
    
    @classmethod
    def FromPythonData(cls, pyData):
        return cls(pyData["name"], pyData["ctype"])

class GLClass:
    def __init__(self, cname, members, data):
        self.cname = cname
        self.members = members
        self.data = data

    @classmethod
    def FromPythonClass(cls, pyClass):
        (cname, guts) = pyClass

        # Members
        pyMembers = []
        for (name, item) in inspect.getmembers(guts):
            if name.startswith('__'):
                continue
            if inspect.ismethod(item):
                item = item.im_func
                pyMembers.append((name, item))
        cmembers = [GLFunction.FromPythonFunction(m) for m in pyMembers]

        # Data
        cData = []
        if hasattr(guts, "Data"):
            for dataDecl in guts.Data:
                cData.append(GLData.FromPythonData(dataDecl))
        # TODO: Turn members that are not 'manual_state' into data packets here, too.

        return cls(cname, cmembers, cData)

def generateHeader(allMembers, allClasses, cmdLine):
    lines = []
    lines.append("// This file was automatically generated, do not modify. To regenerate, run:")
    lines.append("// %s" % cmdLine)
    lines.append("")
    lines.append("#pragma once")
    lines.append('#include <map>')
    lines.append('#include "functionhooks.manual.h"')
    lines.append("")

    lines.append("extern bool gIsRecording;")
    lines.append("extern class ContextState* gContextState;")
    for member in allMembers:
        if member.needsManualDetour:
            lines.append(member.asRealPointerData(False))
        elif member.needsPublicReal:
            lines.append(member.asRealPointerData(False))


    lines.append("")
    lines.append("void Generated_ResolveDynamics();")
    lines.append("void Generated_AttachStaticHooks();")
    lines.append("void Generated_AttachDynamicHooks();")
    lines.append("void Generated_DetachAllHooks();")
    lines.append("size_t GLenumToParameterCount(GLenum pname);")
    lines.append("")

    # Generate core detour declarations
    for member in allMembers:
        lines.append(member.asDetouredFunction(False))
    lines.append("")

    # Generate serialization enumeration
    lines.append("// Serialization Enumeration")
    lines.append("enum ESerializeTypes {")
    for member in allMembers:
        if member.alias is not None:
            continue
        if not member.supported:
            continue
        lines.append("\t%s," % member.asDataName)
    lines.append("\tEST_Message,")
    lines.append("\tEST_Sentinel,")
    lines.append("\n\tEST_ForceSize = 0x7FFFFFFF")

    lines.append("};")

    lines.append("")

    # Generate structure for serialization.
    lines.append("struct %s" % kDataPacketStructName)
    lines.append("{")
    lines.append("\tvoid Read(FileLike* _in);")
    lines.append("\tvoid Write(FileLike* _out) const;")
    lines.append("\tvoid Play() const;")
    lines.append("")
    lines.append("\tESerializeTypes mDataType;")
    lines.append("\tsize_t mPacketId;")
    lines.append("\tunion {")
    for member in allMembers:
        if member.alias is not None:
            continue
        if not member.supported:
            continue
        lines.append("\t\t%s;" % member.asSerializeStruct(True))
    lines.append("\t\tstruct { int level; char* messageBody; } mData_Message;")
    lines.append("\t};")
    lines.append("")
    for member in allMembers:
        if member.alias is not None:
            continue
        if not member.supported:
            continue
        lines.append("\tstatic %s %s(%s);" % (kDataPacketStructName, member.name, member.argsAsStr))
    lines.append("")
    lines.append("};")
    lines.append("")

    for member in allMembers:
        if member.needsManualReplay:
            lines.append("void ManualPlay_%s(%s);" % (member.name, member.argsAsStr))
    lines.append("")

    # For pointers, generate the declaration of the parameter to determine pointer size.
    lines.append("// determining pointer length for parameters")
    for member in allMembers:
        for i, arg in enumerate(member.args):
            if arg.isPointer:
                if member.canAutoDeterminePointerLength(i):
                    lines.append("inline size_t %s(%s) { if (!%s) return 0; %s }" % (arg.asDeterminePointerLengthFunc(member.name), member.argsAsStr, arg.name, member.asDeterminePointerLengthBody(i) ))
                else:
                    lines.append("       size_t %s(const ContextState* _ctxState, %s);" % (arg.asDeterminePointerLengthFunc(member.name), member.argsAsStr))
    lines.append("")

    for stateClass in allClasses:
        lines.append("class %s" % stateClass.cname)
        lines.append("{")
        lines.append("public:")
        lines.append("\t%s();" % stateClass.cname)
        lines.append("\t~%s();" % stateClass.cname)
        lines.append("\tvoid Read(FileLike* _in);")
        lines.append("\tvoid Write(FileLike* _out) const;")
        lines.append("\tvoid OnCaptureStart();")
        lines.append("\tvoid Restore();")
        # TODO: need a way to specify C functions on the class, rather than here.
        lines.append("\tvoid SetOwnerThreadId(DWORD _threadId);")
        lines.append("\tbool CheckOwnerThreadId() const;")

        lines.append("")
        for member in stateClass.members:
            if member.alias is not None:
                continue
            if not member.supported:
                continue
            if member.needsManualState:
                lines.append("\t%s %s(%s);" % (member.returnType, member.name, member.argsAsStateStr))
            else:
                lines.append("\tvoid %s(%s);" % (member.name, member.argsAsStateStr))
        lines.append("")
        for entry in stateClass.data:
            lines.append("\tinline const %s Get%s() const { return mData_%s; }" % (entry.ctype, entry.name, entry.name))

        lines.append("")
        lines.append("private:")
        if len(stateClass.members) > 0:
            lines.append("\tvoid ManualConstruct(); // Construct any manual data members")
            lines.append("\tvoid ManualDestruct(); // Destroy any manual data members")
        lines.append("\tvoid ManualWrite(FileLike* _out) const;")
        lines.append("\tvoid ManualRead(FileLike* _in);")
        lines.append("\tvoid ManualPreRestore();")
        lines.append("\tvoid ManualRestore();")
        lines.append("")
        for member in stateClass.members:
            if member.isState and not member.needsManualState and member.alias is None and member.supported:
                lines.append("\t%s;" % member.asStateStruct(True))
        for member in stateClass.members:
            if member.isState and not member.needsManualState and member.alias is None and member.supported:
                lines.append("\tbool mHasSet_%s;" % (member.name))
        lines.append("")
        for data in stateClass.data:
            lines.append("\t%s;" % (data.asDeclaration))
        lines.append("\t// For data reconstruction")
        lines.append("\tfriend class GLTrace;")
        lines.append("};")
        lines.append("")

    # GCC whinges if the last line doesn't end with "\n"
    lines.append("\n")

    return "\n".join(lines)

def generateCpp(allMembers, allClasses, cmdLine):
    lines = []
    lines.append("// This file was automatically generated, do not modify. To regenerate, run:")
    lines.append("// %s" % cmdLine)
    lines.append("")
    lines.append('#include "StdAfx.h"')
    lines.append('#include "functionhooks.gen.h"')
    lines.append('#include "thirdparty/mhook/mhook-lib/mhook.h"')
    lines.append('#include "extensions.h"')
    lines.append("")

    lines.append("bool gIsRecording = false;")
    lines.append("ContextState* gContextState = NULL;")

    lines.append("")

    lines.append("// Pointers to real functions")
    for member in allMembers:
        lines.append(member.asRealPointerData(True))
    lines.append("")

    lines.append("// Functions to resolve, attach and detach extensions we know about")
    lines.append("void Generated_ResolveDynamics()")
    lines.append("{")
    lines.append("\t// Per documentation, do not call FreeLibrary with this handle.")
    lines.append('\tHMODULE hOpenGL32 = GetModuleHandle(TEXT("opengl32.dll"));')
    lines.append("\tFARPROC tmpProc = NULL;")

    lines.append("\t// Note: Don't care if these don't get found.")
    for member in [m for m in allMembers if not m.needsStaticHook]:
        lines.append('\ttmpProc = %s("%s");' % (kGetDynamicProcAddress, member.name))
        lines.append('\t%s = (%s)(tmpProc ? tmpProc : %s(hOpenGL32, "%s"));' % (member.asRealPointerName, member.asRealPointerCast, kGetStaticProcAddress, member.name))
        lines.append("")
    lines.append("}\n")
    
    lines.append("void Generated_AttachStaticHooks()")
    lines.append("{")
    lines.append("\tMhook_BeginMultiOperation(FALSE);")
    lines.append("\tBOOL hookSuccess = true;")
    for member in [m for m in allMembers if m.needsStaticHook]:
        lines.append("\tif (%s != NULL) {" % (member.asRealPointerName))
        lines.append("\t\thookSuccess = Mhook_SetHook(&(PVOID&)%s, %s);" % (member.asRealPointerName, member.asDetouredName))
        lines.append("\t}")
        lines.append("")
    lines.append("\tMhook_EndMultiOperation();")
    lines.append("}\n")

    lines.append("void Generated_AttachDynamicHooks()")
    lines.append("{")
    lines.append("\tMhook_BeginMultiOperation(FALSE);")
    lines.append("\tBOOL hookSuccess = true;")
    for member in [m for m in allMembers if not m.needsStaticHook]:
        lines.append("\tif (%s != NULL) {" % (member.asRealPointerName))
        lines.append("\t\thookSuccess = Mhook_SetHook(&(PVOID&)%s, %s);" % (member.asRealPointerName, member.asDetouredName))
        lines.append("\t}")
        lines.append("")
    lines.append("\tMhook_EndMultiOperation();")
    lines.append("}\n")

    lines.append("void Generated_DetachAllHooks()")
    lines.append("{")
    lines.append("\tBOOL unhookSuccess = true;")
    for member in allMembers:
        lines.append("\tif (%s != NULL) {" % (member.asRealPointerName))
        lines.append("\t\tunhookSuccess = Mhook_Unhook(&(PVOID&)%s);" % (member.asRealPointerName))
        lines.append("\t}")
        lines.append("")
    lines.append("")
    lines.append("}\n")

    # Generate core hook definitions
    lines.append("// Hook bodies (autogenerated) -- see functionhooks.manual.cpp for hand-written hook bodies")
    for member in allMembers:
        if not member.needsManualDetour:
            lines.append("%s\n" % member.asDetouredFunction(True))

    # Generate SSerializeDataPacket::Read and SSerializeDataPacket::Write
    lines.append("void %s::Read(FileLike* _in)" % (kDataPacketStructName,))
    lines.append("{")
    lines.append("\t_in->ReadRaw(this, sizeof(*this));")
    lines.append("")
    lines.append("\tswitch(mDataType)")
    lines.append("\t{")
    for member in allMembers:
        if member.alias is not None:
            continue
        if not member.supported:
            continue
        if member.hasAnyPointers:
            lines.append("\t\tcase %s:" % member.asDataName)
            lines.append("\t\t{")
            lines.append("\t\t\tsize_t toStreamSize = 0;")
            for arg in member.args:
                if arg.isPointer:
                    lines.append("\t\t\ttoStreamSize = (size_t)(%s.%s);" % (member.asDataStructMemberName, arg.name))
                    lines.append("\t\t\tif (toStreamSize != 0) {")
                    lines.append("\t\t\t\tvoid* newBuffer = malloc(toStreamSize);")
                    lines.append("\t\t\t\tassert(newBuffer != 0);")
                    lines.append("\t\t\t\t_in->ReadRaw(newBuffer, toStreamSize);")
                    lines.append("\t\t\t\t%s.%s = (%s)newBuffer;" % (member.asDataStructMemberName, arg.name, arg.ctype))
                    lines.append("\t\t\t} else {")
                    lines.append("\t\t\t\t_in->Read((size_t*)&%s.%s);" % (member.asDataStructMemberName, arg.name))
                    lines.append("\t\t\t}")

            lines.append("\t\t\tbreak;")
            lines.append("\t\t}")
            lines.append("")
    lines.append("\t\tcase EST_Message:")
    lines.append("\t\t{")
    lines.append("\t\t\tsize_t toStreamSize = (size_t)mData_Message.messageBody;")
    lines.append("\t\t\tassert(toStreamSize != 0);")
    lines.append("\t\t\tvoid* newBuffer = malloc(toStreamSize);")
    lines.append("\t\t\tassert(newBuffer != 0);")
    lines.append("\t\t\t_in->ReadRaw(newBuffer, toStreamSize);")
    lines.append("\t\t\tmData_Message.messageBody = (char*)newBuffer;")
    lines.append("\t\t\tbreak;")
    lines.append("\t\t}")
    lines.append("")
    lines.append("\t\tdefault:")
    lines.append("\t\t\tbreak;")
    lines.append("\t};")
    lines.append("}")
    lines.append("")

    lines.append("void %s::Write(FileLike* _out) const" % (kDataPacketStructName,))
    lines.append("{")
    lines.append("\t%s tmpPkt(*this);" % (kDataPacketStructName,))
    lines.append("\ttmpPkt.mPacketId = _out->AllocatePacketId();")
    lines.append("")
    lines.append("\tswitch(mDataType)")
    lines.append("\t{")
    for member in allMembers:
        if member.alias is not None:
            continue
        if not member.supported:
            continue
        if member.hasAnyPointers:
            lines.append("\t\tcase %s:" % member.asDataName)
            lines.append("\t\t{")
            for i, arg in enumerate(member.args):
                if arg.isPointer:
                    if member.canAutoDeterminePointerLength(i):
                        lines.append("\t\t\ttmpPkt.%s.%s = (%s) %s(%s);" % (member.asDataStructMemberName, arg.name, arg.ctype, arg.asDeterminePointerLengthFunc(member.name), ", ".join(["mData_%s.%s" % (member.name, arg.name) for arg in member.args])))
                    else:
                        lines.append("\t\t\ttmpPkt.%s.%s = (%s) %s(gContextState, %s);" % (member.asDataStructMemberName, arg.name, arg.ctype, arg.asDeterminePointerLengthFunc(member.name), ", ".join(["mData_%s.%s" % (member.name, arg.name) for arg in member.args])))
            lines.append("\t\t\t_out->WriteRaw(&tmpPkt, sizeof(tmpPkt));")
            for arg in member.args:
                if arg.isPointer:
                    lines.append("\t\t\tif (tmpPkt.%s.%s != 0) {" % (member.asDataStructMemberName, arg.name))
                    lines.append("\t\t\t\t_out->WriteRaw(%s.%s, (size_t) tmpPkt.%s.%s);" % (member.asDataStructMemberName, arg.name, member.asDataStructMemberName, arg.name))
                    lines.append("\t\t\t} else {")
                    lines.append("\t\t\t\t_out->Write((size_t)%s.%s);" % (member.asDataStructMemberName, arg.name))
                    lines.append("\t\t\t}")
            lines.append("\t\t\tbreak;")
            lines.append("\t\t}")
            lines.append("")
    lines.append("\t\tcase EST_Message:")
    lines.append("\t\t{")
    lines.append("\t\t\ttmpPkt.mData_Message.messageBody = (char*)(strlen(mData_Message.messageBody) + 1);")
    lines.append("\t\t\t_out->WriteRaw(&tmpPkt, sizeof(tmpPkt));")
    lines.append("\t\t\t_out->WriteRaw(mData_Message.messageBody, (size_t)tmpPkt.mData_Message.messageBody);")
    lines.append("\t\t\tbreak;")
    lines.append("\t\t}")
    lines.append("\t\tdefault:")
    lines.append("\t\t\t// Writes out tmpPkt because it has a packet id for debugging")
    lines.append("\t\t\t_out->WriteRaw(&tmpPkt, sizeof(tmpPkt));")
    lines.append("\t\t\tbreak;")
    lines.append("\t};")
    lines.append("}")
    lines.append("")
    lines.append("void %s::Play() const" % (kDataPacketStructName,))
    lines.append("{")
    lines.append("\tswitch (mDataType)")
    lines.append("\t{")
    for member in allMembers:
        if member.alias is not None or not member.supported:
            continue
        lines.append("\t\tcase %s:" % (member.asDataName,))
        lines.append("\t\t{")
        if "APPLE" in member.name:
            lines.append("\t\t#ifdef _APPLE")
        if member.needsManualReplay:
            lines.append("\t\t\tManualPlay_%s(%s);" % (member.name, ", ".join(["mData_%s.%s" % (member.name, arg.name) for arg in member.args])))
        else:
            lines.append("\t\t\t::%s(%s);" % (member.name, ", ".join(["mData_%s.%s" % (member.name, arg.name) for arg in member.args])))
        if "APPLE" in member.name:
            lines.append("\t\t#endif /* _APPLE */")
        lines.append("\t\t\t// CHECK_GL_ERROR();")
        lines.append("\t\t\tbreak;")
        lines.append("\t\t}")
    lines.append("\t\tdefault:")
    lines.append("\t\t\tbreak;")
    lines.append("\t}")
    lines.append("}")

    lines.append("")


    for member in allMembers:
        if member.alias is not None:
            continue
        if not member.supported:
            continue
        lines.append("%s %s::%s(%s)" % (kDataPacketStructName, kDataPacketStructName, member.name, member.argsAsStr))
        lines.append("{")
        lines.append("\t%s retVal;" % (kDataPacketStructName,))
        lines.append("\tmemset(&retVal, 0, sizeof(retVal));")
        lines.append("\tretVal.mDataType = %s;" % (member.asDataName,))
        for arg in member.args:
            lines.append("\tretVal.%s.%s = %s;" % (member.asDataStructMemberName, arg.name, arg.name))
        lines.append("\treturn retVal;")
        lines.append("}")
        lines.append("")



    for stateClass in allClasses:
        lines.append("%s::%s()" % (stateClass.cname, stateClass.cname))
        lines.append("{")
        if len(stateClass.data) == 0:
            lines.append("\tmemset(this, 0, sizeof(*this));")
        else:
            lines.append("\tmemset(this, 0, offsetof(%s, mData_%s));" % (stateClass.cname, stateClass.data[0].name))
            lines.append("\tManualConstruct();")
        lines.append("}")
        lines.append("")
        if len(stateClass.data) == 0:
            lines.append("%s::~%s() { }" % (stateClass.cname, stateClass.cname))
        else:
            lines.append("%s::~%s()" % (stateClass.cname, stateClass.cname))
            lines.append("{")
            lines.append("\tManualDestruct();")
            lines.append("}")
        lines.append("")

        lines.append("void %s::Write(FileLike* _out) const" % (stateClass.cname))
        lines.append("{")
        lines.append('\t_out->Write(Checkpoint("CurrentStateBegin"));')
        for member in stateClass.members:
            if member.isState:
                if member.needsManualState or member.alias is not None or not member.supported:
                    continue
                lines.append("\t_out->Write(mHasSet_%s);" % (member.name))
                lines.append("\tif (mHasSet_%s)" % (member.name))
                lines.append("\t\t%s::%s(%s).Write(_out);" % (kDataPacketStructName, member.name, ", ".join(["mData_%s.%s" % (member.name, arg.name) for arg in member.args])))
                lines.append("")

        lines.append('\t_out->Write(Checkpoint("CurrentStateEnd"));')
        lines.append("")
        lines.append("\tManualWrite(_out);")
        lines.append("}")
        lines.append("")

        lines.append("void %s::Read(FileLike* _in)" % (stateClass.cname))
        lines.append("{")
        lines.append('\t_in->Read(Checkpoint("CurrentStateBegin"));')
        lines.append("\tbool needToReceiveState = 0;")
        lines.append("\tSSerializeDataPacket pkt;")
        for member in stateClass.members:
            if member.isState:
                if member.needsManualState or member.alias is not None or not member.supported:
                    continue
                lines.append("\t_in->Read(&needToReceiveState);")
                lines.append("\tif (needToReceiveState) {")
                lines.append("\t\t_in->Read(&pkt);")
                lines.append("\t\tassert(pkt.mDataType == %s);" % (member.asDataName))
                lines.append("\t\t%s(%s);" % (member.name, member.asDataStructFunctionArgs("pkt")))
                lines.append("\t}")
                lines.append("")
        lines.append('\t_in->Read(Checkpoint("CurrentStateEnd"));')
        lines.append("\tManualRead(_in);")
        lines.append("}")
        lines.append("")

        lines.append("void %s::Restore()" % (stateClass.cname))
        lines.append("{")
        lines.append("\tCHECK_GL_ERROR();")
        lines.append("\tManualPreRestore();")
        for member in stateClass.members:
            if member.isState:
                if member.needsManualState or member.alias is not None or not member.supported or member.needsManualRestore:
                    continue
                lines.append("\tif (mHasSet_%s) {" % (member.name))
                if "APPLE" in member.name:
                    lines.append("\t\t#ifdef _APPLE")
                lines.append("\t\t::%s(%s);" % (member.name, ", ".join(["mData_%s.%s" % (member.name, arg.name) for arg in member.args])))
                if "APPLE" in member.name:
                    lines.append("\t\t#endif /* _APPLE */")
                lines.append("\t\tCHECK_GL_ERROR();")
                lines.append("\t}")
                lines.append("")
        lines.append("\tManualRestore();")
        lines.append("}")
        lines.append("")

        for member in stateClass.members:
            if member.isState and not member.needsManualState and member.alias is None and member.supported:
                lines.append("void %s::%s(%s)" % (stateClass.cname, member.name, member.argsAsStr))
                lines.append("{")
                lines.append("\tmHasSet_%s = true;" % member.name)
                for i, arg in enumerate(member.args):
                    if arg.isPointer:
                        if arg.isPointerOrOffset:
                            lines.append("\tif (mData_%s.%s) {" % (member.name, arg.pointerOrOffsetName))
                            lines.append("\t\tSafeFree(mData_%s.%s);" % (member.name, arg.name))
                            lines.append("\t}")
                        else:
                            lines.append("\tSafeFree(mData_%s.%s);" % (member.name, arg.name))
                        if member.canAutoDeterminePointerLength(i):
                            lines.append("\tsize_t ptrSize_%s = %s(%s);" % (arg.name, arg.asDeterminePointerLengthFunc(member.name), member.argsForPassingAsStr))
                        else:
                            lines.append("\tsize_t ptrSize_%s = %s(this, %s);" % (arg.name, arg.asDeterminePointerLengthFunc(member.name), member.argsForPassingAsStr))
                        lines.append("\tif (ptrSize_%s) {" % (arg.name))
                        lines.append("\t\tmData_%s.%s = (%s)malloc(ptrSize_%s);" % (member.name, arg.name, arg.ctype, arg.name))
                        if arg.isConst:
                            lines.append("\t\t// This is only apparently const to the outside world. We're gonna trounce on it, though.")
                            lines.append("\t\tmemcpy(const_cast<%s>(mData_%s.%s), %s, ptrSize_%s);" % (arg.lvaluetype, member.name, arg.name, arg.name, arg.name))
                        else:
                            lines.append("\t\tmemcpy(mData_%s.%s, %s, ptrSize_%s);" % (member.name, arg.name, arg.name, arg.name))
                        if arg.isPointerOrOffset:
                            lines.append("\t\tmData_%s.%s = true;" % (member.name, arg.pointerOrOffsetName))
                        lines.append("\t} else {")
                        if arg.isPointerOrOffset:
                            lines.append("\t\tmData_%s.%s = %s;" % (member.name, arg.name, arg.name))
                            lines.append("\t\tmData_%s.%s = false;" % (member.name, arg.pointerOrOffsetName))
                        else:
                            lines.append('\t\tOnce(TraceWarn("Unable to determine pointer length for argument %s in method %s. Probably a trace bug."));' % (arg.name, member.name))
                        lines.append("\t}")
                    else:
                        lines.append("\tmData_%s.%s = %s;" % (member.name, arg.name, arg.name))
                lines.append("}")
                lines.append("")

    # GCC whinges if the file doesn't end with a "\n"
    lines.append("\n")
    return "\n".join(lines)

# -------------------------------------------------------------------------------------------------
def importFromClass(cls, isState, stateBlock=None, unsupported=False):
    allMembers = []
    allClasses = []
    for (name, item) in inspect.getmembers(cls):
        if name.startswith('__'):
            continue
        if inspect.ismethod(item):
            item = item.im_func
            # function we need to hook.
            item.isState = isState
            if stateBlock:
                item.stateBlock = stateBlock
            if unsupported:
                item.unsupported = True
            allMembers.append((name, item))
        elif inspect.isclass(item):
            allClasses.append((name, item))
            continue
        else:
            continue
    return (allMembers, allClasses)

# -------------------------------------------------------------------------------------------------
def importDefinitions(hookModule):
    allMemberNames = set()
    duplicates = set()

    allMembers = []
    nestedStateClasses = []

    (newMembers, newStateClasses) = importFromClass(hookModule.Hooks.GlobalState, True, kGlobalState)
    allMembers.extend(newMembers)
    nestedStateClasses.extend(newStateClasses)
    
    (newMembers, newStateClasses) = importFromClass(hookModule.Hooks.Actions, False)
    allMembers.extend(newMembers)
    if len(newStateClasses) > 0:
        raise SyntaxError("Found a nested state block under %s.Hooks.Actions--don't know what to do with this." % hookModule.__name__)

    (newMembers, newStateClasses) = importFromClass(hookModule.Hooks.Unsupported, False, unsupported=True)
    allMembers.extend(newMembers)
    if len(newStateClasses) > 0:
        raise SyntaxError("Found a nested state block under %s.Hooks.Unsupported--don't know what to do with this." % hookModule.__name__)

    for (name, pyClass) in nestedStateClasses:
        (newMembers, newStateClasses) = importFromClass(pyClass, True, name)
        allMembers.extend(newMembers)
        if len(newStateClasses) > 0:
            raise SyntaxError("Found a nested state block under %s.Hooks.GlobalState.%s--don't know what to do with this." % (hookModule.__name__, pyClass.__name__))

    # Check for duplicates (cause I'm clumsy)
    memberNames = set()
    duplicates = set()
    for (name, _) in allMembers:
        if name in memberNames:
            duplicates.add(name)
        else:
            memberNames.add(name)

    if len(duplicates) > 0:
        print("ERROR: Detected duplicate glEntry points, please remove the duplicates: %s" % (", ".join([s for s in duplicates])))
        raise SyntaxError("Duplicate Entries in %s" % hookModule.__name__) 

    allMembers.sort(lambda x,y : cmp(x[0], y[0]))
    nestedStateClasses.sort(lambda x,y : cmp(x[0], y[0]))

    return ([GLFunction.FromPythonFunction(m) for m in allMembers], [GLClass.FromPythonClass(c) for c in nestedStateClasses])
    

# -------------------------------------------------------------------------------------------------
def main():
    import functionhooks
    members, classes = importDefinitions(functionhooks)

    # Generate the strings first, then write them out. Eliminates a possible source of mismatched 
    # header and cpp files.   
    headerStr = generateHeader(members, classes, "./codegen.py")
    cppStr = generateCpp(members, classes, "./codegen.py")

    open("functionhooks.gen.h", "w").write(headerStr)
    open("functionhooks.gen.cpp", "w").write(cppStr)

# -------------------------------------------------------------------------------------------------
if __name__ == '__main__':
    main()

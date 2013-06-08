
kTypemapFilename = "gl.tm"

kSpecFileURL = "http://www.opengl.org/registry/api/gl.spec"
kSpecFileLocal = "cached.gl.spec"
kTypemapURL = "http://www.opengl.org/registry/api/gl.tm"
kTypemapLocal = "cached.gl.tm"

import json
import os
import re
import sys
import urllib2
import entrypoint

# -------------------------------------------------------------------------------------------------
def parseTypemap(fileObj):
    retDict = {}
    for line in fileObj:
        # Work with unicode, always.
        line = unicode(line)

        if line.startswith('#'):
            continue

        arguments = line.split(',')
        # Otherwise an input we don't understand.
        assert(len(arguments) == 6 or (len(arguments) == 7 and arguments[6] == '\n'))

        realType = arguments[3].strip()

        # void, for example.
        if realType == '*':
            realType = arguments[0].strip()
        retDict[arguments[0]] = realType

    return retDict

# -------------------------------------------------------------------------------------------------
def stripComment(line, commentString=None):
    commentString = commentString if commentString else '#'
    if commentString not in line:
        return line
    return line[:line.find(commentString)]

# -------------------------------------------------------------------------------------------------
# -------------------------------------------------------------------------------------------------
# -------------------------------------------------------------------------------------------------
WhitespaceRE = re.compile(r'^\s*$')
PropDefinitionRE = re.compile(r'([a-zA-Z0-9\-_]+):\s+(.*)')
FuncDefinitionRE = re.compile(r'^([a-zA-Z0-9_]+)\((.*)\).*$')
ParmDefinitionRE = re.compile(r'(\w+)\s+(.*)')
def parseSpecFile(fileObj):
    allowedProps = {}
    allFunctions = {}
    currentFunction = None

    for line in fileObj:
        # Work with unicode, always.
        line = unicode(line)

        line = stripComment(line)
        if len(line) == 0 or WhitespaceRE.match(line):
            if currentFunction is not None:
                assert(currentFunction.name not in allFunctions)
                allFunctions[currentFunction.name] = currentFunction
                currentFunction = None
            continue

        # property definitions
        if ':' in line:
            g = PropDefinitionRE.search(line)
            if g:
                if g.group(1) in ('passthru', 'newcategory'):
                    # We don't need passthrus or newcategories
                    continue

                values = g.group(2).strip().split()
                allowedProps.setdefault(g.group(1), []).extend(values)
                continue

        # Function definition start.
        g = FuncDefinitionRE.search(line)
        if g:
            funcName = g.group(1)
            funcArgs = g.group(2).split(',')
            funcArgs = [f.strip() for f in funcArgs]
            if len(funcArgs) == 1 and len(funcArgs[0]) == 0:
                funcArgs = []
            currentFunction = entrypoint.EntryPoint(funcName, funcArgs)
            continue

        # Function definition continues.
        if line.startswith('\t'):
            assert(currentFunction)
            g = ParmDefinitionRE.search(line)
            if g:
                name = g.group(1)
                args = g.group(2).split()

                findFunc = "parse_%s" % (name)
                callFunc = getattr(currentFunction, findFunc, None)
                if callFunc:
                    callFunc(args)
                else:
                    pass
                continue

        # We hit something I don't know how to parse..
        assert(0)

    if currentFunction:
        assert(currentFunction.name not in allFunctions)
        allFunctions[currentFunction.name] = currentFunction
        currentFunction = None

    return allFunctions

# -------------------------------------------------------------------------------------------------
def getOrFetch(localCache, remoteURL, useCache):
    if useCache:
        try:
            return open(localCache, "rb")
        except IOError:
            fileContents = urllib2.urlopen(remoteURL).read()
            try:
                open(localCache, "wb").write(fileContents)
                return open(localCache, "rb")
            except IOError:
                pass

    return urllib2.urlopen(remoteURL)

# -------------------------------------------------------------------------------------------------
def getSpecData(useCache):
    typemapFile = getOrFetch(kTypemapLocal, kTypemapURL, useCache)
    specFile = getOrFetch(kSpecFileLocal, kSpecFileURL, useCache)

    typemapDict = parseTypemap(typemapFile)
    spec = parseSpecFile(specFile)

    for ep in spec.itervalues():
        ep.ResolveTypes(typemapDict)
    
    returnDict = { "typemap": typemapDict, "spec": spec }
    return returnDict

# -------------------------------------------------------------------------------------------------
def getEntryPoints(useCache):
    return getSpecData(useCache)["spec"]

# -------------------------------------------------------------------------------------------------
# -------------------------------------------------------------------------------------------------
# -------------------------------------------------------------------------------------------------
def test(useCache=False):
    return getSpecData(useCache)

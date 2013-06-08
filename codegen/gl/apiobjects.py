
# -------------------------------------------------------------------------------------------------
# -------------------------------------------------------------------------------------------------
# -------------------------------------------------------------------------------------------------
class GLObject(object):
    pass

# -------------------------------------------------------------------------------------------------
class Namespace(GLObject):
    def __init__(self, glAbstractType, pyConcreteType):
        super(GLObject, self).__init__()
        self.glAbstractType = glAbstractType
        self.pyConcreteType = pyConcreteType

# -------------------------------------------------------------------------------------------------
class TextureObject(GLObject):
    AbstractType = "Texture"

    BindFunction = "BindTexture"
    DirectStateAccessBind = "BindMultiTextureEXT"

    HasType = True
    BindDeterminesType = True

    CreateFunc = "GenTextures"
    DeleteFunc = "DeleteTextures"

    StateFunctions = (
        "TexParameterf",
        "TexParameterfv", 
        "TexParameteri", 
        "TexParameteriv", 
    )

    DataFunctions = (
        "TexImage1D",
        "TexImage2D",
        "TexImage3D",
        "TexSubImage1D",
        "TexSubImage2D",
        "TexSubImage3D",
        "CopyTexImage1D",
        "CopyTexImage2D",
        "CopyTexSubImage1D",
        "CopyTexSubImage2D",
        "CopyTexSubImage3D",
    )

    DirectStateAccessFunctions = (
        "TextureParameterfEXT",
        "TextureParameterfvEXT", 
        "TextureParameteriEXT", 
        "TextureParameterivEXT", 
        "TextureImage1DEXT",
        "TextureImage2DEXT",
        "TextureSubImage1DEXT",
        "TextureSubImage2DEXT",
        "CopyTextureImage1DEXT",
        "CopyTextureImage2DEXT",
        "CopyTextureSubImage1DEXT",
        "CopyTextureSubImage2DEXT",
        "TextureImage3DEXT",
        "TextureSubImage3DEXT",
        "CopyTextureSubImage3DEXT",
    )

# -------------------------------------------------------------------------------------------------
class TextureUnit(GLObject):
    AbstractType = "TextureUnit"

    BindFunction = "ActiveTexture"

    DirectStateAccessFunctions = (
        "MultiTexParameterfEXT",
        "MultiTexParameterfvEXT", 
        "MultiTexParameteriEXT", 
        "MultiTexParameterivEXT", 
        "MultiTexImage1DEXT",
        "MultiTexImage2DEXT",
        "MultiTexSubImage1DEXT",
        "MultiTexSubImage2DEXT",
        "CopyMultiTexImage1DEXT",
        "CopyMultiTexImage2DEXT",
        "CopyMultiTexSubImage1DEXT",
        "CopyMultiTexSubImage2DEXT",
        "MultiTexImage3DEXT",
        "MultiTexSubImage3DEXT",
        "CopyMultiTexSubImage3DEXT",
        "BindMultiTextureEXT",
    )

    StateFunctions = (
        "BindTexture"
    )

    TextureBindings = Namespace("TextureTarget", "TextureObject")

# -------------------------------------------------------------------------------------------------
class ContextState(GLObject):
    TextureUnits = Namespace("TextureUnit", "TextureUnit")
    TextureObjects = Namespace("Texture", "TextureObject")

    BindFunction = "MakeCurrent"


# -------------------------------------------------------------------------------------------------
def AllAPIObjects():
    # return (ContextState, TextureUnit, TextureObject)
    return (TextureObject, )

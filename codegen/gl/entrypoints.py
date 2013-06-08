
import apiobjects
import entrypoint
import copy


TexParameterValuesAndTypes = (
    ("GL_TEXTURE_MIN_FILTER",          "GLint",        "GL_NEAREST_MIPMAP_LINEAR"),
    ("GL_TEXTURE_MAG_FILTER",          "GLint",        "GL_LINEAR"),
    ("GL_TEXTURE_MIN_LOD",             "GLfloat",      "-1000"),
    ("GL_TEXTURE_MAX_LOD",             "GLfloat",      "1000"),
    ("GL_TEXTURE_BASE_LEVEL",          "GLint",        "0"),
    ("GL_TEXTURE_MAX_LEVEL",           "GLint",        "1000"),
    ("GL_TEXTURE_WRAP_S",              "GLint",        "GL_REPEAT"),
    ("GL_TEXTURE_WRAP_T",              "GLint",        "GL_REPEAT"),
    ("GL_TEXTURE_WRAP_R",              "GLint",        "GL_REPEAT"),
    ("GL_TEXTURE_BORDER_COLOR",        "GLfloat[4]",   ), # The color will be specified in ManualConstruct
    ("GL_TEXTURE_PRIORITY",            "GLfloat",      "1"),
    ("GL_TEXTURE_COMPARE_MODE",        "GLint",        "GL_NONE"),
    ("GL_TEXTURE_COMPARE_FUNC",        "GLint",        "GL_ALWAYS"),
    ("GL_TEXTURE_SRGB_DECODE_EXT",     "GLint",        "GL_DECODE_EXT"),
    ("GL_DEPTH_TEXTURE_MODE",          "GLint",        "GL_LUMINANCE"),
    ("GL_GENERATE_MIPMAP",             "GLint",        "GL_FALSE"),
    ("GL_TEXTURE_MAX_ANISOTROPY_EXT",  "GLfloat",      "1.0f"),
)

def annotate(entrypoints, typemap):
    allEntryPoints = copy.deepcopy(entrypoints)
    
    for k in allEntryPoints:
        if k.startswith("TexParameter"):
            allEntryPoints[k].SetMultiState("TextureParameterName", TexParameterValuesAndTypes)

    return allEntryPoints, None

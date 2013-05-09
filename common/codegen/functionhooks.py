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

def alias(otherfunc):
    def wrapper(func):
        setattr(func, 'alias', otherfunc)
        return func
    return wrapper

def manual_detour(func):
    ''' If you need to manually define a detour body, use this. '''
    setattr(func, 'manual_detour', True)
    return func

def manual_restore(func):
    ''' If the contents of this need to be manually restored, use this. '''
    setattr(func, 'manual_restore', True)
    return func

def manual_replay(func):
    ''' If the function needs to have a manual replay body, use this. '''
    setattr(func, 'manual_replay', True)
    return func

def public_real(func):
    ''' Makes the real_ function available in the header so other code can have it. '''
    setattr(func, 'public_real', True)
    return func

def multi_state(paramName, stateEnums, defaultCtype=None, defaultCVal=None):
    ''' Marks that this entry point controls multiple pieces of state.

    paramName specifies which parameter controls the switch statement
    stateEnums is an enumerable that contains the names of states, or pairs of (name, ctype, defaultVal) for storage.
    defaultCtype, if provided, controls the default storage type for each value in the stateEnum. 
    defaultCVal, if provided, is what the values are set to (just a string, pasted in for you). '''
    def wrapper(func):
        setattr(func, 'multi_state', (paramName, stateEnums, defaultCtype, defaultCVal))
        return func
    return wrapper

def pointer_or_offset(paramName):
    ''' Marks that a particular argument could be a pointer or an offset. '''
    def wrapper(func):
        setattr(func, 'pointer_or_offset', paramName)
        return func
    return wrapper

def returns(returnType):
    ''' Specifies the return type for an entry point. '''
    def wrapper(func):
        setattr(func, 'returntype', returnType)
        return func
    return wrapper

def state(func):
    setattr(func, 'isState', True)
    return func

def manual_state(func):
    func = state(func)
    setattr(func, 'manual_state', True)
    return func

EnableCaps = (
    "GL_ALPHA_TEST",
    "GL_AUTO_NORMAL",
    "GL_BLEND",
    "GL_CLIP_PLANEi",
    "GL_COLOR_LOGIC_OP",
    "GL_COLOR_MATERIAL",
    "GL_COLOR_SUM",
    "GL_COLOR_TABLE",
    "GL_CONVOLUTION_1D",
    "GL_CONVOLUTION_2D",
    "GL_CULL_FACE",
    "GL_DEPTH_TEST",
    "GL_DITHER",
    "GL_FOG",
    "GL_HISTOGRAM",
    "GL_INDEX_LOGIC_OP",
    "GL_LIGHTi",
    "GL_LIGHTING",
    "GL_LINE_SMOOTH",
    "GL_LINE_STIPPLE",
    "GL_MAP1_COLOR_4",
    "GL_MAP1_INDEX",
    "GL_MAP1_NORMAL",
    "GL_MAP1_TEXTURE_COORD_1",
    "GL_MAP1_TEXTURE_COORD_2",
    "GL_MAP1_TEXTURE_COORD_3",
    "GL_MAP1_TEXTURE_COORD_4",
    "GL_MAP1_VERTEX_3",
    "GL_MAP1_VERTEX_4",
    "GL_MAP2_COLOR_4",
    "GL_MAP2_INDEX",
    "GL_MAP2_NORMAL",
    "GL_MAP2_TEXTURE_COORD_1",
    "GL_MAP2_TEXTURE_COORD_2",
    "GL_MAP2_TEXTURE_COORD_3",
    "GL_MAP2_TEXTURE_COORD_4",
    "GL_MAP2_VERTEX_3",
    "GL_MAP2_VERTEX_4",
    "GL_MINMAX",
    "GL_MULTISAMPLE",
    "GL_NORMALIZE",
    "GL_POINT_SMOOTH",
    "GL_POINT_SPRITE",
    "GL_POLYGON_OFFSET_FILL",
    "GL_POLYGON_OFFSET_LINE",
    "GL_POLYGON_OFFSET_POINT",
    "GL_POLYGON_SMOOTH",
    "GL_POLYGON_STIPPLE",
    "GL_POST_COLOR_MATRIX_COLOR_TABLE",
    "GL_POST_CONVOLUTION_COLOR_TABLE",
    "GL_RESCALE_NORMAL",
    "GL_SAMPLE_ALPHA_TO_COVERAGE",
    "GL_SAMPLE_ALPHA_TO_ONE",
    "GL_SAMPLE_COVERAGE",
    "GL_SEPARABLE_2D",
    "GL_SCISSOR_TEST",
    "GL_STENCIL_TEST",
    "GL_TEXTURE_1D",
    "GL_TEXTURE_2D",
    "GL_TEXTURE_3D",
    "GL_TEXTURE_CUBE_MAP",
    "GL_TEXTURE_GEN_Q",
    "GL_TEXTURE_GEN_R",
    "GL_TEXTURE_GEN_S",
    "GL_TEXTURE_GEN_T",
    "GL_VERTEX_PROGRAM_POINT_SIZE",
    "GL_VERTEX_PROGRAM_TWO_SIDE",
)

def static_hook(func):
    ''' Specifies an entry point should be hooked statically (ie, should be resolved without glGetProcAddress) '''
    setattr(func, 'static_hook', True)
    return func

# Gives us things to look for as base classes.
class GLObject(object):
    pass

class Hooks:
    ''' Container class for all functions we want to hook for OGL support. '''

    class GlobalState(GLObject):
        ''' Global state, consists of one or more Contexts. Currently only one context is supported. '''

        class ContextState(GLObject):
            ''' These are members that affect the global state vector (which is actually tied to each context). '''

            Data = (
                ### Objects for manual data holding.
                # Which thread owns this context (ie, most recently called MakeCurrent while owner was null)
                { "name": "OwnerThread",            "ctype": "DWORD" },

                # The texture units. Each texture unit can have one texture of each type bound to it.
                { "name": "TextureUnits",           "ctype": "std::map<std::pair<GLuint, GLenum>, GLuint>" },
                # The texture objects themselves. Note that unless sampler objects are used, the texture object also contains
                # its sampler state.
                { "name": "TextureObjects",         "ctype": "std::map<GLuint, GLTexture*>" },
                # State set by calling glPixelStoreState{f|i}.
                { "name": "PixelStoreState",        "ctype": "GLPixelStoreState" },
                # Pixel Transfer state, which is a multi-state.
                { "name": "PixelTransferState",     "ctype": "GLPixelTransferState" },
                # Buffer objects.
                { "name": "BufferBindings",         "ctype": "std::map<GLenum, GLuint>" },
                { "name": "BufferObjects",          "ctype": "std::map<GLuint, GLBuffer*>" },
                # Program objects.
                { "name": "ProgramObjectsGLSL",     "ctype": "std::map<GLuint, GLProgram*>" },
                { "name": "ShaderObjectsGLSL",      "ctype": "std::map<GLuint, GLShader*>" },

                # ARB Program objects. Currently minimal support for these.
                { "name": "ProgramBindingsARB",     "ctype": "std::map<GLenum, GLuint>" },
                { "name": "ProgramObjectsARB",      "ctype": "std::map<GLuint, GLProgramARB*>" },

                # Enable/Disable
                { "name": "EnableCap",              "ctype": "std::map<GLenum, GLboolean>" },
                { "name": "TextureEnableCap",       "ctype": "std::map<std::pair<GLenum, GLenum>, GLboolean>" },

                # FrameBufferObjects/RenderBufferObjects
                { "name": "FrameBufferBindings",    "ctype": "std::map<GLenum, GLuint>" },
                { "name": "FrameBufferObjects",     "ctype": "std::map<GLuint, GLFrameBufferObject*>" },
                { "name": "RenderBufferBindings",   "ctype": "std::map<GLenum, GLuint>" },
                { "name": "RenderBufferObjects",    "ctype": "std::map<GLuint, GLRenderBufferObject*>" },

                # Clip plane Equations
                { "name": "ClipPlaneEquations",     "ctype": "std::map<GLenum, GLClipPlane>" },

                # Draw and Read Buffer when an FBO is not bound.
                { "name": "DrawBuffer",             "ctype": "GLenum" },
                { "name": "ReadBuffer",             "ctype": "GLenum" },

                # Sampler Objects
                { "name": "SamplerBindings",         "ctype": "std::map<GLuint, GLuint>" },
                { "name": "SamplerObjects",          "ctype": "std::map<GLuint, GLSampler*>" },

                # Generic vertex attribute enable/disable
                { "name": "VertexAttribEnabled",    "ctype": "std::map<GLuint, bool>" },

                # Queries.
                # { "name": "QueryObjects",      "ctype": "std::map<GLuint, GLQuery*>" }, TODO
            )

            ### Core stuff ###
            def glAccum(GLenum_op, GLfloat_value): pass
            def glAlphaFunc(GLenum_func, GLclampf_ref): pass

            @manual_replay
            @manual_state
            def glBindTexture(GLenum_target, GLuint_texture): pass

            def glBlendFunc(GLenum_sfactor, GLenum_dfactor): pass
            def glClearAccum(GLfloat_red, GLfloat_green, GLfloat_blue, GLfloat_alpha): pass

            def glClearColor(GLclampf_red, GLclampf_green, GLclampf_blue, GLclampf_alpha): pass

            def glClearDepth(GLclampd_depth): pass
            def glClearIndex(GLfloat_c): pass
            def glClearStencil(GLint_s): pass

            @manual_state
            def glClipPlane(GLenum_plane, const_GLdouble_ptr_equation): pass

            def glColorMask(GLboolean_red, GLboolean_green, GLboolean_blue, GLboolean_alpha): pass
            def glColorMaterial(GLenum_face, GLenum_mode): pass

            @manual_state
            @returns('GLhandleARB')
            def glCreateProgramObjectARB(): pass

            @manual_state
            @returns('GLhandleARB')
            def glCreateShaderObjectARB(GLenum_type): pass

            @manual_state
            def glDeleteTextures(GLsizei_n, const_GLuint_ptr_textures): pass

            def glDepthFunc(GLenum_func): pass
            def glDepthMask(GLboolean_flag): pass
            def glDepthRange(GLclampd_zNear, GLclampd_zFar): pass

            @manual_state
            def glDisable(GLenum_cap): pass

            def glDisableClientState(GLenum_array): pass
            def glEdgeFlag(GLboolean_flag): pass

            @pointer_or_offset("pointer")
            def glEdgeFlagPointer(GLsizei_stride, const_GLvoid_ptr_pointer): pass

            def glEdgeFlagv(const_GLboolean_ptr_flag): pass
            
            @manual_state
            def glEnable(GLenum_cap): pass

            def glEnableClientState(GLenum_array): pass
            def glFeedbackBuffer(GLsizei_size, GLenum_type, GLfloat_ptr_buffer): pass
            def glFogf(GLenum_pname, GLfloat_param): pass
            def glFogfv(GLenum_pname, const_GLfloat_ptr_params): pass
            def glFogi(GLenum_pname, GLint_param): pass
            def glFogiv(GLenum_pname, const_GLint_ptr_params): pass
            def glFrontFace(GLenum_mode): pass
            def glFrustum(GLdouble_left, GLdouble_right, GLdouble_bottom, GLdouble_top, GLdouble_zNear, GLdouble_zFar): pass
    
            @returns('GLuint')
            def glGenLists(GLsizei_range): pass

            @manual_state
            def glGenTextures(GLsizei_n, GLuint_ptr_textures): pass
            
            @manual_state
            def glPixelStoref(GLenum_pname, GLfloat_param): pass

            @manual_state
            def glPixelStorei(GLenum_pname, GLint_param): pass
            
            @manual_state
            def glPixelTransferf(GLenum_pname, GLfloat_param): pass

            @manual_state
            def glPixelTransferi(GLenum_pname, GLint_param): pass

            def glPixelZoom(GLfloat_xfactor, GLfloat_yfactor): pass
            def glPointSize(GLfloat_size): pass
            def glPolygonMode(GLenum_face, GLenum_mode): pass
            def glPolygonOffset(GLfloat_factor, GLfloat_units): pass
            def glPolygonStipple(const_GLubyte_ptr_mask): pass
            def glPrioritizeTextures(GLsizei_n, const_GLuint_ptr_textures, const_GLclampf_ptr_priorities): pass
            def glRasterPos2d(GLdouble_x, GLdouble_y): pass
            def glRasterPos2dv(const_GLdouble_ptr_v): pass
            def glRasterPos2f(GLfloat_x, GLfloat_y): pass
            def glRasterPos2fv(const_GLfloat_ptr_v): pass
            def glRasterPos2i(GLint_x, GLint_y): pass
            def glRasterPos2iv(const_GLint_ptr_v): pass
            def glRasterPos2s(GLshort_x, GLshort_y): pass
            def glRasterPos2sv(const_GLshort_ptr_v): pass
            def glRasterPos3d(GLdouble_x, GLdouble_y, GLdouble_z): pass
            def glRasterPos3dv(const_GLdouble_ptr_v): pass
            def glRasterPos3f(GLfloat_x, GLfloat_y, GLfloat_z): pass
            def glRasterPos3fv(const_GLfloat_ptr_v): pass
            def glRasterPos3i(GLint_x, GLint_y, GLint_z): pass
            def glRasterPos3iv(const_GLint_ptr_v): pass
            def glRasterPos3s(GLshort_x, GLshort_y, GLshort_z): pass
            def glRasterPos3sv(const_GLshort_ptr_v): pass
            def glRasterPos4d(GLdouble_x, GLdouble_y, GLdouble_z, GLdouble_w): pass
            def glRasterPos4dv(const_GLdouble_ptr_v): pass
            def glRasterPos4f(GLfloat_x, GLfloat_y, GLfloat_z, GLfloat_w): pass
            def glRasterPos4fv(const_GLfloat_ptr_v): pass
            def glRasterPos4i(GLint_x, GLint_y, GLint_z, GLint_w): pass
            def glRasterPos4iv(const_GLint_ptr_v): pass
            def glRasterPos4s(GLshort_x, GLshort_y, GLshort_z, GLshort_w): pass
            def glRasterPos4sv(const_GLshort_ptr_v): pass
            
            @manual_state
            def glReadBuffer(GLenum_mode): pass
            
            def glScissor(GLint_x, GLint_y, GLsizei_width, GLsizei_height): pass
            def glSelectBuffer(GLsizei_size, GLuint_ptr_buffer): pass
            def glShadeModel(GLenum_mode): pass
            def glStencilFunc(GLenum_func, GLint_ref, GLuint_mask): pass
            def glStencilMask(GLuint_mask): pass
            def glStencilOp(GLenum_fail, GLenum_zfail, GLenum_zpass): pass
            
            @pointer_or_offset("pointer")
            def glTexCoordPointer(GLint_size, GLenum_type, GLsizei_stride, const_GLvoid_ptr_pointer): pass

            def glTexEnvf(GLenum_target, GLenum_pname, GLfloat_param): pass
            def glTexEnvfv(GLenum_target, GLenum_pname, const_GLfloat_ptr_params): pass
            def glTexEnvi(GLenum_target, GLenum_pname, GLint_param): pass
            def glTexEnviv(GLenum_target, GLenum_pname, const_GLint_ptr_params): pass
            def glTexImage1D(GLenum_target, GLint_level, GLint_internalformat, GLsizei_width, GLint_border, GLenum_format, GLenum_type, const_GLvoid_ptr_pixels): pass
            
            @manual_state
            def glTexImage2D(GLenum_target, GLint_level, GLint_internalformat, GLsizei_width, GLsizei_height, GLint_border, GLenum_format, GLenum_type, const_GLvoid_ptr_pixels): pass

            def glTexSubImage1D(GLenum_target, GLint_level, GLint_xoffset, GLsizei_width, GLenum_format, GLenum_type, const_GLvoid_ptr_pixels): pass

            @manual_state
            def glTexSubImage2D(GLenum_target, GLint_level, GLint_xoffset, GLint_yoffset, GLsizei_width, GLsizei_height, GLenum_format, GLenum_type, const_GLvoid_ptr_pixels): pass

            @pointer_or_offset("pointer")
            def glVertexPointer(GLint_size, GLenum_type, GLsizei_stride, const_GLvoid_ptr_pointer): pass
            def glViewport(GLint_x, GLint_y, GLsizei_width, GLsizei_height): pass

            @manual_state
            def glTexParameterf(GLenum_target, GLenum_pname, GLfloat_param): pass

            @manual_state
            def glTexParameterfv(GLenum_target, GLenum_pname, const_GLfloat_ptr_params): pass

            @manual_state
            def glTexParameteri(GLenum_target, GLenum_pname, GLint_param): pass

            @manual_state
            def glTexParameteriv(GLenum_target, GLenum_pname, const_GLint_ptr_params): pass


            ### Extensions ###
            def glActiveTexture(GLenum_texture): pass

            @manual_state
            def glAttachShader(GLuint_program, GLuint_shader): pass
            
            @alias(glAttachShader)
            def glAttachObjectARB(GLhandleARB_a, GLhandleARB_b): pass

            @manual_state
            def glBindAttribLocation(GLuint_program, GLuint_index, const_GLchar_ptr_name): pass
            
            @alias(glBindAttribLocation)
            def glBindAttribLocationARB(GLhandleARB_program, GLuint_index, const_GLcharARB_ptr_name): pass
            
            @manual_replay
            @manual_state
            def glBindBuffer(GLenum_target, GLuint_buffer): pass

            @alias(glBindBuffer)
            def glBindBufferARB(GLenum_target, GLuint_buffer): pass

            @manual_replay
            @manual_state
            def glBindMultiTextureEXT(GLenum_texunit, GLenum_target, GLuint_texture): pass
            
            @manual_replay
            @manual_state
            def glBindProgramARB(GLenum_target,GLuint_program): pass
            def glBlendColor(GLclampf_a,GLclampf_b,GLclampf_c,GLclampf_d): pass
            def glBlendEquation(GLenum_a): pass

            @manual_state
            def glBufferData(GLenum_target, GLsizeiptr_size, const_GLvoid_ptr_data, GLenum_usage): pass

            @alias(glBufferData)
            def glBufferDataARB(GLenum_target, GLsizeiptrARB_size, const_GLvoid_ptr_data, GLenum_usage): pass

            @manual_state
            def glCompileShader(GLuint_shader): pass

            @alias(glCompileShader)
            def glCompileShaderARB(GLhandleARB_shader): pass
            
            @manual_state
            def glCompressedTexImage2D(GLenum_target,GLint_level,GLenum_internalformat,GLsizei_width,GLsizei_height,GLint_border,GLsizei_imagesize,const_GLvoid_ptr_data): pass

            @manual_state
            def glCompressedTexImage3D(GLenum_target,GLint_level,GLenum_internalformat,GLsizei_width,GLsizei_height,GLsizei_depth,GLint_border,GLsizei_imagesize,const_GLvoid_ptr_data): pass

            @manual_state
            def glDeleteBuffersARB(GLsizei_n,const_GLuint_ptr_buffers): pass

            def glDeleteObjectARB(GLhandleARB_a): pass
            
            @manual_state
            def glDeleteProgramsARB(GLsizei_n,const_GLuint_ptr_programs): pass

            def glDeleteQueriesARB(GLsizei_n,const_GLuint_ptr_b): pass

            @manual_state
            def glDeleteShader(GLuint_a): pass
            
            @manual_state
            def glDetachShader(GLuint_program, GLuint_shader): pass

            @alias(glDetachShader)
            def glDetachObjectARB(GLhandleARB_container,GLhandleARB_attached): pass

            @manual_state
            def glDisableVertexAttribArray(GLuint_index): pass

            @manual_state
            def glDrawBuffer(GLenum_mode): pass
            
            @manual_state
            def glEnableVertexAttribArray(GLuint_index): pass

            @manual_state
            def glGenBuffersARB(GLsizei_n,GLuint_ptr_buffers): pass
            
            @manual_state
            def glGenProgramsARB(GLsizei_n,GLuint_ptr_programs): pass
            
            # @manual_state TODO
            def glGenQueriesARB(GLsizei_n,GLuint_ptr_queries): pass

            @manual_state
            def glLinkProgram(GLuint_program): pass

            @alias(glLinkProgram)
            def glLinkProgramARB(GLhandleARB_program): pass
    
            @manual_state
            def glProgramStringARB(GLenum_target,GLenum_format,GLsizei_len,const_GLvoid_ptr_string): pass
            
            @manual_state
            def glTexImage3D(GLenum_target,GLint_level,GLint_internalFormat,GLsizei_width,GLsizei_height,GLsizei_depth,GLint_border,GLenum_format,GLenum_type,const_GLvoid_ptr_data): pass

            @manual_replay
            @manual_state
            def glUniform1f(GLint_location, GLfloat_v0): pass

            @manual_replay
            @manual_state
            def glUniform1i(GLint_location, GLint_v0): pass

            @alias(glUniform1i)
            def glUniform1iARB(GLint_location, GLint_v0): pass

            @manual_replay
            @manual_state
            def glUniform4fv(GLint_location, GLsizei_count, const_GLfloat_ptr_value): pass

            @manual_detour
            @manual_state
            @returns('GLboolean')
            def glUnmapBuffer(GLenum_target): pass

            @manual_replay
            @manual_restore
            def glUseProgram(GLuint_program): pass

            @pointer_or_offset("pointer")
            def glVertexAttribPointer(GLuint_index, GLint_size, GLenum_type, GLboolean_normalized, GLsizei_stride, const_GLvoid_ptr_pointer): pass
            def glClientActiveTexture(GLenum_a): pass

            def glProgramEnvParameters4fvEXT(GLenum_target,GLuint_index,GLsizei_count,const_GLfloat_ptr_params): pass
            def glStencilOpSeparate(GLenum_a,GLenum_b,GLenum_c,GLenum_d): pass
            def glStencilFuncSeparate(GLenum_a,GLenum_b,GLint_c,GLuint_d): pass

            @alias("glDeleteRenderbuffers")
            def glDeleteRenderbuffersEXT(GLsizei_n,const_GLuint_ptr_b): pass

            @alias("glFramebufferRenderbuffer")
            def glFramebufferRenderbufferEXT(GLenum_a,GLenum_b,GLenum_c,GLuint_d): pass
            
            @alias("glFramebufferTexture2D")
            def glFramebufferTexture2DEXT(GLenum_a,GLenum_b,GLenum_c,GLuint_d,GLint_e): pass
            
            @alias("glFramebufferTexture3D")
            def glFramebufferTexture3DEXT(GLenum_a,GLenum_b,GLenum_c,GLuint_d,GLint_e,GLint_f): pass
            
            @alias("glGenFramebuffers") # TODO: This one should actually be different, so it can say "this was an EXT, not a core frame buffer"
            def glGenFramebuffersEXT(GLsizei_a,GLuint_ptr_b): pass

            @alias("glGenRenderbuffers") # TODO: This one should actually be different, so it can say "this was an EXT, not a core render buffer"
            def glGenRenderbuffersEXT(GLsizei_a,GLuint_ptr_b): pass

            @alias("glDeleteFramebuffers")
            def glDeleteFramebuffersEXT(GLsizei_n,const_GLuint_ptr_framebuffers): pass

            @alias("glRenderbufferStorageMultisample")
            def glRenderbufferStorageMultisampleEXT(GLenum_a,GLsizei_b,GLenum_c,GLsizei_d,GLsizei_e): pass

            def glColorMaskIndexedEXT(GLuint_a,GLboolean_b,GLboolean_c,GLboolean_d,GLboolean_e): pass
            def glEnableIndexedEXT(GLenum_a,GLuint_b): pass
            def glDisableIndexedEXT(GLenum_a,GLuint_b): pass

            # @manual_state
            def glUniformBufferEXT(GLuint_a,GLint_b,GLuint_c): pass

            def glBufferParameteriAPPLE(GLenum_a,GLenum_b,GLint_c): pass

            @manual_state
            def glBufferSubData(GLenum_target,GLintptr_offset,GLsizeiptr_size,const_GLvoid_ptr_data): pass

            @manual_replay
            @manual_state
            def glBindFramebuffer(GLenum_target,GLuint_framebuffer): pass

            @alias(glBindFramebuffer)
            def glBindFramebufferEXT(GLenum_target,GLuint_framebuffer): pass

            @manual_replay
            @manual_state
            def glBindRenderbuffer(GLenum_target,GLuint_renderbuffer): pass

            @alias(glBindRenderbuffer)
            def glBindRenderbufferEXT(GLenum_target,GLuint_renderbuffer): pass
    
            @manual_state
            def glDeleteRenderbuffers(GLsizei_n,const_GLuint_ptr_b): pass

            @manual_replay
            @manual_state
            def glFramebufferRenderbuffer(GLenum_target,GLenum_attachment,GLenum_renderbuffertarget,GLuint_renderbuffer): pass

            @manual_replay
            @manual_state
            def glFramebufferTexture2D(GLenum_target,GLenum_attachment,GLenum_textarget,GLuint_texture,GLint_level): pass

            @manual_replay
            @manual_state
            def glFramebufferTexture3D(GLenum_target,GLenum_attachment,GLenum_textarget,GLuint_texture,GLint_level, GLint_layer): pass

            @manual_state
            def glGenFramebuffers(GLsizei_n,GLuint_ptr_ids): pass

            @manual_state
            def glGenRenderbuffers(GLsizei_n,GLuint_ptr_renderbuffers): pass

            @manual_state
            def glGenSamplers(GLsizei_n,GLuint_ptr_samplers): pass

            @manual_replay
            @manual_state
            def glDeleteSamplers(GLsizei_n,const_GLuint_ptr_samplers): pass

            @manual_replay
            @manual_state
            def glBindSampler(GLuint_unit,GLuint_sampler): pass

            @manual_replay
            @manual_state
            def glSamplerParameteri(GLuint_sampler,GLenum_pname,GLint_param): pass

            @manual_replay
            @manual_state
            def glSamplerParameterf(GLuint_sampler,GLenum_pname,GLfloat_param): pass

            @manual_replay
            @manual_state
            def glSamplerParameterfv(GLuint_sampler,GLenum_pname,const_GLfloat_ptr_params): pass

            @manual_state
            @returns('GLint')
            def glGetUniformLocation(GLuint_program,const_GLchar_ptr_name): pass

            @manual_state
            @alias(glGetUniformLocation)
            @returns('GLint')
            def glGetUniformLocationARB(GLhandleARB_program,const_GLcharARB_ptr_name): pass

            @manual_state
            def glDeleteFramebuffers(GLsizei_n,const_GLuint_ptr_framebuffers): pass

            @manual_state
            def glRenderbufferStorageMultisample(GLenum_target, GLsizei_samples, GLenum_internalformat, GLsizei_width, GLsizei_height): pass

            @manual_detour
            @manual_state
            @returns('GLvoid_ptr')
            def glMapBufferARB(GLenum_target, GLenum_access): pass

            @manual_detour
            @manual_state
            @returns('GLvoid_ptr')
            def glMapBufferRange(GLenum_target, GLintptr_offset, GLsizeiptr_length, GLbitfield_access): pass

            @manual_detour
            @manual_state
            def glFlushMappedBufferRange(GLenum_target, GLintptr_offset, GLsizeiptr_length): pass

            @manual_state
            def glShaderSource(GLuint_shader,GLsizei_count,const_GLchar_ptr_ptr_string,const_GLint_ptr_length): pass

            @alias(glShaderSource)
            def glShaderSourceARB(GLhandleARB_shader,GLsizei_count,const_GLcharARB_ptr_ptr_string,const_GLint_ptr_length): pass
            

    class Actions:
        ''' Action methods (drawing, blitting, etc) '''

        ### Core stuff ###
        @returns('GLboolean')
        def glAreTexturesResident(GLsizei_n, const_GLuint_ptr_textures, GLboolean_ptr_residences): pass

        def glBitmap(GLsizei_width, GLsizei_height, GLfloat_xorig, GLfloat_yorig, GLfloat_xmove, GLfloat_ymove, const_GLubyte_ptr_bitmap): pass
        def glCallList(GLuint_list): pass
        def glCallLists(GLsizei_n, GLenum_type, const_GLvoid_ptr_lists): pass
        def glClear(GLbitfield_mask): pass

        @pointer_or_offset("pointer")
        def glColorPointer(GLint_size, GLenum_type, GLsizei_stride, const_GLvoid_ptr_pointer): pass

        def glCopyPixels(GLint_x, GLint_y, GLsizei_width, GLsizei_height, GLenum_type): pass
        def glCopyTexImage1D(GLenum_target, GLint_level, GLenum_internalFormat, GLint_x, GLint_y, GLsizei_width, GLint_border): pass
        def glCopyTexImage2D(GLenum_target, GLint_level, GLenum_internalFormat, GLint_x, GLint_y, GLsizei_width, GLsizei_height, GLint_border): pass
        def glCopyTexSubImage1D(GLenum_target, GLint_level, GLint_xoffset, GLint_x, GLint_y, GLsizei_width): pass
        def glCopyTexSubImage2D(GLenum_target, GLint_level, GLint_xoffset, GLint_yoffset, GLint_x, GLint_y, GLsizei_width, GLsizei_height): pass
        def glCullFace(GLenum_mode): pass
        def glDeleteLists(GLuint_list, GLsizei_range): pass
        ### Extensions ###
        def glDrawArrays(GLenum_mode, GLint_first, GLsizei_count): pass
        
        
        def glDrawElements(GLenum_mode, GLsizei_count, GLenum_type, const_GLvoid_ptr_indices): pass
        def glDrawPixels(GLsizei_width, GLsizei_height, GLenum_format, GLenum_type, const_GLvoid_ptr_pixels): pass

        def glEnd(): pass
        def glEndList(): pass
    
        def glFinish(): pass
    
        def glFlush(): pass
    
        def glGetBooleanv(GLenum_pname, GLboolean_ptr_params): pass
        def glGetClipPlane(GLenum_plane, GLdouble_ptr_equation): pass
        def glGetDoublev(GLenum_pname, GLdouble_ptr_params): pass
    
        @returns('GLenum')
        def glGetError(): pass

        @public_real
        def glGetFloatv(GLenum_pname, GLfloat_ptr_params): pass
        
        @public_real
        def glGetIntegerv(GLenum_pname, GLint_ptr_params): pass
    
        @public_real
        def glGetShaderiv(GLuint_shader, GLenum_pname, GLint_ptr_params): pass

        @public_real
        def glGetProgramiv(GLuint_program,GLenum_pname,GLint_ptr_params): pass

        @returns('const_GLubyte_ptr')
        def glGetString(GLenum_name): pass

        @returns('GLboolean')
        def glIsEnabled(GLenum_cap): pass

        @returns('GLboolean')
        def glIsList(GLuint_list): pass

        @returns('GLboolean')
        def glIsTexture(GLuint_texture): pass

        def glReadPixels(GLint_x, GLint_y, GLsizei_width, GLsizei_height, GLenum_format, GLenum_type, GLvoid_ptr_pixels): pass
    
        @returns('GLint')
        def glRenderMode(GLenum_mode): pass

        @manual_detour
        @static_hook
        @returns('BOOL')
        def wglMakeCurrent(HDC_hdc, HGLRC_hglrc): pass

        def glDrawRangeElements(GLenum_mode,GLuint_start,GLuint_end,GLsizei_count,GLenum_type,const_GLvoid_ptr_indices): pass
        def glDrawRangeElementsBaseVertex(GLenum_mode,GLuint_start,GLuint_end,GLsizei_count,GLenum_type,const_GLvoid_ptr_indices,GLint_basevertex): pass
        def glGetCompressedTexImage(GLenum_a,GLint_b,GLvoid_ptr_c): pass
        def glGetObjectParameterivARB(GLhandleARB_a,GLenum_b,GLint_ptr_c): pass
    
        @returns('GLenum')
        def glCheckFramebufferStatusEXT(GLenum_a): pass
    
        @manual_state
        def glBlitFramebufferEXT(GLint_srcX0,GLint_srcY0,GLint_srcX1,GLint_srcY1,GLint_dstX0,GLint_dstY0,GLint_dstX1,GLint_dstY1,GLbitfield_mask,GLenum_filter): pass

        def glSetFenceAPPLE(GLuint_a): pass
        def glFinishFenceAPPLE(GLuint_a): pass
        def glDeleteFencesAPPLE(GLsizei_n,const_GLuint_ptr_b): pass
        def glGenFencesAPPLE(GLsizei_a,GLuint_ptr_b): pass
    
        @returns('GLboolean')
        def glTestFenceNV(GLuint_a): pass

        def glSetFenceNV(GLuint_a,GLenum_b): pass
        def glFinishFenceNV(GLuint_a): pass
        def glDeleteFencesNV(GLsizei_n,const_GLuint_ptr_b): pass
        def glGenFencesNV(GLsizei_a,GLuint_ptr_b): pass
        def glGetSynciv(GLsync_a, GLenum_b, GLsizei_c, GLsizei_ptr_d, GLint_ptr_e): pass
    
        @returns('GLenum')
        def glClientWaitSync(GLsync_a, GLbitfield_b, GLuint64_c): pass

        def glWaitSync(GLsync_a, GLbitfield_b, GLuint64_c): pass
        def glDeleteSync(GLsync_a): pass
    
        @returns('GLsync')
        def glFenceSync(GLenum_a, GLbitfield_b): pass
    
        def glGetBooleanIndexedvEXT(GLenum_a,GLuint_b,GLboolean_ptr_c): pass

    
        @returns('GLint')
        def glGetUniformBufferSizeEXT(GLenum_a, GLenum_b): pass
    
        @returns('GLintptr')
        def glGetUniformOffsetEXT(GLenum_a, GLenum_b): pass
    
        def glFlushMappedBufferRangeAPPLE(GLenum_a,GLintptr_b,GLsizeiptr_c): pass

        def glBeginQueryARB(GLenum_a,GLuint_b): pass
        def glEndQueryARB(GLenum_a): pass

        @manual_replay
        def glGetQueryObjectivARB(GLuint_id,GLenum_pname,GLint_ptr_params): pass

        @manual_replay
        def glGetQueryObjectuivARB(GLuint_id,GLenum_pname,GLuint_ptr_params): pass

        def glTextureRangeAPPLE(GLenum_a,GLsizei_b,void_ptr_c): pass
        def glGetTexParameterPointervAPPLE(GLenum_a,GLenum_b,void_ptr_c): pass
    
        @returns('GLenum')
        def glCheckFramebufferStatus(GLenum_a): pass

        @manual_state
        def glBlitFramebuffer(GLint_srcX0,GLint_srcY0,GLint_srcX1,GLint_srcY1,GLint_dstX0,GLint_dstY0,GLint_dstX1,GLint_dstY1,GLbitfield_mask,GLenum_filter): pass
        def glStringMarkerGREMEDY(GLsizei_a,const_void_ptr_b): pass
        # TODO: glDebugMessageCallbackARB def glDebugMessageCallbackARB(void (APIENTRY ptr_a)(GLenum, GLenum , GLuint , GLenum , GLsizei , const GLchar_ptr , GLvoid_ptr) ,void_ptr b)
        # TODO: glDebugMessageControlARB def glDebugMessageControlARB(GLenum_a, GLenum_b, GLenum_c, GLsizei_d, const GLuint_ptr_e, GLboolean_f)

        @manual_replay
        @manual_detour
        @static_hook
        @returns('BOOL')
        def SwapBuffers(HDC_hdc): pass

    class Unsupported:
        ''' Unsupported things are... unsupported. They will be hooked and will insert an error into the trace if called. '''
        def glHint(GLenum_target, GLenum_mode): pass
        def glIndexMask(GLuint_mask): pass
        def glIndexPointer(GLenum_type, GLsizei_stride, const_GLvoid_ptr_pointer): pass
        def glIndexd(GLdouble_c): pass
        def glIndexdv(const_GLdouble_ptr_c): pass
        def glIndexf(GLfloat_c): pass
        def glIndexfv(const_GLfloat_ptr_c): pass
        def glIndexi(GLint_c): pass
        def glIndexiv(const_GLint_ptr_c): pass
        def glIndexs(GLshort_c): pass
        def glIndexsv(const_GLshort_ptr_c): pass
        def glIndexub(GLubyte_c): pass
        def glIndexubv(const_GLubyte_ptr_c): pass
        def glInterleavedArrays(GLenum_format, GLsizei_stride, const_GLvoid_ptr_pointer): pass
        def glLightModelf(GLenum_pname, GLfloat_param): pass
        def glLightModelfv(GLenum_pname, const_GLfloat_ptr_params): pass
        def glLightModeli(GLenum_pname, GLint_param): pass
        def glLightModeliv(GLenum_pname, const_GLint_ptr_params): pass
        def glLightf(GLenum_light, GLenum_pname, GLfloat_param): pass
        def glLightfv(GLenum_light, GLenum_pname, const_GLfloat_ptr_params): pass
        def glLighti(GLenum_light, GLenum_pname, GLint_param): pass
        def glLightiv(GLenum_light, GLenum_pname, const_GLint_ptr_params): pass
        def glLineStipple(GLint_factor, GLushort_pattern): pass
        def glLineWidth(GLfloat_width): pass
        def glListBase(GLuint_base): pass
        def glLogicOp(GLenum_opcode): pass
        def glMaterialf(GLenum_face, GLenum_pname, GLfloat_param): pass
        def glMaterialfv(GLenum_face, GLenum_pname, const_GLfloat_ptr_params): pass
        def glMateriali(GLenum_face, GLenum_pname, GLint_param): pass
        def glMaterialiv(GLenum_face, GLenum_pname, const_GLint_ptr_params): pass
        def glMatrixMode(GLenum_mode): pass
        def glNormalPointer(GLenum_type, GLsizei_stride, const_GLvoid_ptr_pointer): pass
        def glOrtho(GLdouble_left, GLdouble_right, GLdouble_bottom, GLdouble_top, GLdouble_zNear, GLdouble_zFar): pass
        def glPassThrough(GLfloat_token): pass
        def glPixelMapfv(GLenum_map, GLsizei_mapsize, const_GLfloat_ptr_values): pass
        def glPixelMapuiv(GLenum_map, GLsizei_mapsize, const_GLuint_ptr_values): pass
        def glPixelMapusv(GLenum_map, GLsizei_mapsize, const_GLushort_ptr_values): pass


        def glArrayElement(GLint_i): pass

        def glBegin(GLenum_mode): pass

        def glColor3b(GLbyte_red, GLbyte_green, GLbyte_blue): pass
        def glColor3bv(const_GLbyte_ptr_v): pass
        def glColor3d(GLdouble_red, GLdouble_green, GLdouble_blue): pass
        def glColor3dv(const_GLdouble_ptr_v): pass
        def glColor3f(GLfloat_red, GLfloat_green, GLfloat_blue): pass
        def glColor3fv(const_GLfloat_ptr_v): pass
        def glColor3i(GLint_red, GLint_green, GLint_blue): pass
        def glColor3iv(const_GLint_ptr_v): pass
        def glColor3s(GLshort_red, GLshort_green, GLshort_blue): pass
        def glColor3sv(const_GLshort_ptr_v): pass
        def glColor3ub(GLubyte_red, GLubyte_green, GLubyte_blue): pass
        def glColor3ubv(const_GLubyte_ptr_v): pass
        def glColor3ui(GLuint_red, GLuint_green, GLuint_blue): pass
        def glColor3uiv(const_GLuint_ptr_v): pass
        def glColor3us(GLushort_red, GLushort_green, GLushort_blue): pass
        def glColor3usv(const_GLushort_ptr_v): pass
        def glColor4b(GLbyte_red, GLbyte_green, GLbyte_blue, GLbyte_alpha): pass
        def glColor4bv(const_GLbyte_ptr_v): pass
        def glColor4d(GLdouble_red, GLdouble_green, GLdouble_blue, GLdouble_alpha): pass
        def glColor4dv(const_GLdouble_ptr_v): pass
        def glColor4f(GLfloat_red, GLfloat_green, GLfloat_blue, GLfloat_alpha): pass
        def glColor4fv(const_GLfloat_ptr_v): pass
        def glColor4i(GLint_red, GLint_green, GLint_blue, GLint_alpha): pass
        def glColor4iv(const_GLint_ptr_v): pass
        def glColor4s(GLshort_red, GLshort_green, GLshort_blue, GLshort_alpha): pass
        def glColor4sv(const_GLshort_ptr_v): pass
        def glColor4ub(GLubyte_red, GLubyte_green, GLubyte_blue, GLubyte_alpha): pass
        def glColor4ubv(const_GLubyte_ptr_v): pass
        def glColor4ui(GLuint_red, GLuint_green, GLuint_blue, GLuint_alpha): pass
        def glColor4uiv(const_GLuint_ptr_v): pass
        def glColor4us(GLushort_red, GLushort_green, GLushort_blue, GLushort_alpha): pass
        def glColor4usv(const_GLushort_ptr_v): pass

        def glEvalCoord1d(GLdouble_u): pass
        def glEvalCoord1dv(const_GLdouble_ptr_u): pass
        def glEvalCoord1f(GLfloat_u): pass
        def glEvalCoord1fv(const_GLfloat_ptr_u): pass
        def glEvalCoord2d(GLdouble_u, GLdouble_v): pass
        def glEvalCoord2dv(const_GLdouble_ptr_u): pass
        def glEvalCoord2f(GLfloat_u, GLfloat_v): pass
        def glEvalCoord2fv(const_GLfloat_ptr_u): pass
        def glEvalMesh1(GLenum_mode, GLint_i1, GLint_i2): pass
        def glEvalMesh2(GLenum_mode, GLint_i1, GLint_i2, GLint_j1, GLint_j2): pass
        def glEvalPoint1(GLint_i): pass
        def glEvalPoint2(GLint_i, GLint_j): pass

        def glGetLightfv(GLenum_light, GLenum_pname, GLfloat_ptr_params): pass
        def glGetLightiv(GLenum_light, GLenum_pname, GLint_ptr_params): pass
        def glGetMapdv(GLenum_target, GLenum_query, GLdouble_ptr_v): pass
        def glGetMapfv(GLenum_target, GLenum_query, GLfloat_ptr_v): pass
        def glGetMapiv(GLenum_target, GLenum_query, GLint_ptr_v): pass
        def glGetMaterialfv(GLenum_face, GLenum_pname, GLfloat_ptr_params): pass
        def glGetMaterialiv(GLenum_face, GLenum_pname, GLint_ptr_params): pass
        def glGetPixelMapfv(GLenum_map, GLfloat_ptr_values): pass
        def glGetPixelMapuiv(GLenum_map, GLuint_ptr_values): pass
        def glGetPixelMapusv(GLenum_map, GLushort_ptr_values): pass
        def glGetPointerv(GLenum_pname, GLvoid_ptr_ptr_params): pass
        def glGetPolygonStipple(GLubyte_ptr_mask): pass

        def glGetTexEnvfv(GLenum_target, GLenum_pname, GLfloat_ptr_params): pass
        def glGetTexEnviv(GLenum_target, GLenum_pname, GLint_ptr_params): pass
        def glGetTexGendv(GLenum_coord, GLenum_pname, GLdouble_ptr_params): pass
        def glGetTexGenfv(GLenum_coord, GLenum_pname, GLfloat_ptr_params): pass
        def glGetTexGeniv(GLenum_coord, GLenum_pname, GLint_ptr_params): pass
        def glGetTexImage(GLenum_target, GLint_level, GLenum_format, GLenum_type, GLvoid_ptr_pixels): pass
        def glGetTexLevelParameterfv(GLenum_target, GLint_level, GLenum_pname, GLfloat_ptr_params): pass
        def glGetTexLevelParameteriv(GLenum_target, GLint_level, GLenum_pname, GLint_ptr_params): pass
        def glGetTexParameterfv(GLenum_target, GLenum_pname, GLfloat_ptr_params): pass
        def glGetTexParameteriv(GLenum_target, GLenum_pname, GLint_ptr_params): pass
        def glInitNames(): pass
        def glLoadIdentity(): pass
        def glLoadMatrixd(const_GLdouble_ptr_m): pass
        def glLoadMatrixf(const_GLfloat_ptr_m): pass
        def glLoadName(GLuint_name): pass

        def glMap1d(GLenum_target, GLdouble_u1, GLdouble_u2, GLint_stride, GLint_order, const_GLdouble_ptr_points): pass
        def glMap1f(GLenum_target, GLfloat_u1, GLfloat_u2, GLint_stride, GLint_order, const_GLfloat_ptr_points): pass
        def glMap2d(GLenum_target, GLdouble_u1, GLdouble_u2, GLint_ustride, GLint_uorder, GLdouble_v1, GLdouble_v2, GLint_vstride, GLint_vorder, const_GLdouble_ptr_points): pass
        def glMap2f(GLenum_target, GLfloat_u1, GLfloat_u2, GLint_ustride, GLint_uorder, GLfloat_v1, GLfloat_v2, GLint_vstride, GLint_vorder, const_GLfloat_ptr_points): pass
        def glMapGrid1d(GLint_un, GLdouble_u1, GLdouble_u2): pass
        def glMapGrid1f(GLint_un, GLfloat_u1, GLfloat_u2): pass
        def glMapGrid2d(GLint_un, GLdouble_u1, GLdouble_u2, GLint_vn, GLdouble_v1, GLdouble_v2): pass
        def glMapGrid2f(GLint_un, GLfloat_u1, GLfloat_u2, GLint_vn, GLfloat_v1, GLfloat_v2): pass

        def glMultMatrixd(const_GLdouble_ptr_m): pass
        def glMultMatrixf(const_GLfloat_ptr_m): pass
        def glNewList(GLuint_list, GLenum_mode): pass
        def glNormal3b(GLbyte_nx, GLbyte_ny, GLbyte_nz): pass
        def glNormal3bv(const_GLbyte_ptr_v): pass
        def glNormal3d(GLdouble_nx, GLdouble_ny, GLdouble_nz): pass
        def glNormal3dv(const_GLdouble_ptr_v): pass
        def glNormal3f(GLfloat_nx, GLfloat_ny, GLfloat_nz): pass
        def glNormal3fv(const_GLfloat_ptr_v): pass
        def glNormal3i(GLint_nx, GLint_ny, GLint_nz): pass
        def glNormal3iv(const_GLint_ptr_v): pass
        def glNormal3s(GLshort_nx, GLshort_ny, GLshort_nz): pass
        def glNormal3sv(const_GLshort_ptr_v): pass

        def glPopAttrib(): pass
        def glPopClientAttrib(): pass
        def glPopMatrix(): pass
        def glPopName(): pass

        def glPushAttrib(GLbitfield_mask): pass
        def glPushClientAttrib(GLbitfield_mask): pass
        def glPushMatrix(): pass
        def glPushName(GLuint_name): pass

        def glRectd(GLdouble_x1, GLdouble_y1, GLdouble_x2, GLdouble_y2): pass
        def glRectdv(const_GLdouble_ptr_v1, const_GLdouble_ptr_v2): pass
        def glRectf(GLfloat_x1, GLfloat_y1, GLfloat_x2, GLfloat_y2): pass
        def glRectfv(const_GLfloat_ptr_v1, const_GLfloat_ptr_v2): pass
        def glRecti(GLint_x1, GLint_y1, GLint_x2, GLint_y2): pass
        def glRectiv(const_GLint_ptr_v1, const_GLint_ptr_v2): pass
        def glRects(GLshort_x1, GLshort_y1, GLshort_x2, GLshort_y2): pass
        def glRectsv(const_GLshort_ptr_v1, const_GLshort_ptr_v2): pass
        def glRotated(GLdouble_angle, GLdouble_x, GLdouble_y, GLdouble_z): pass
        def glRotatef(GLfloat_angle, GLfloat_x, GLfloat_y, GLfloat_z): pass
        def glScaled(GLdouble_x, GLdouble_y, GLdouble_z): pass
        def glScalef(GLfloat_x, GLfloat_y, GLfloat_z): pass

        def glTexCoord1d(GLdouble_s): pass
        def glTexCoord1dv(const_GLdouble_ptr_v): pass
        def glTexCoord1f(GLfloat_s): pass
        def glTexCoord1fv(const_GLfloat_ptr_v): pass
        def glTexCoord1i(GLint_s): pass
        def glTexCoord1iv(const_GLint_ptr_v): pass
        def glTexCoord1s(GLshort_s): pass
        def glTexCoord1sv(const_GLshort_ptr_v): pass
        def glTexCoord2d(GLdouble_s, GLdouble_t): pass
        def glTexCoord2dv(const_GLdouble_ptr_v): pass
        def glTexCoord2f(GLfloat_s, GLfloat_t): pass
        def glTexCoord2fv(const_GLfloat_ptr_v): pass
        def glTexCoord2i(GLint_s, GLint_t): pass
        def glTexCoord2iv(const_GLint_ptr_v): pass
        def glTexCoord2s(GLshort_s, GLshort_t): pass
        def glTexCoord2sv(const_GLshort_ptr_v): pass
        def glTexCoord3d(GLdouble_s, GLdouble_t, GLdouble_r): pass
        def glTexCoord3dv(const_GLdouble_ptr_v): pass
        def glTexCoord3f(GLfloat_s, GLfloat_t, GLfloat_r): pass
        def glTexCoord3fv(const_GLfloat_ptr_v): pass
        def glTexCoord3i(GLint_s, GLint_t, GLint_r): pass
        def glTexCoord3iv(const_GLint_ptr_v): pass
        def glTexCoord3s(GLshort_s, GLshort_t, GLshort_r): pass
        def glTexCoord3sv(const_GLshort_ptr_v): pass
        def glTexCoord4d(GLdouble_s, GLdouble_t, GLdouble_r, GLdouble_q): pass
        def glTexCoord4dv(const_GLdouble_ptr_v): pass
        def glTexCoord4f(GLfloat_s, GLfloat_t, GLfloat_r, GLfloat_q): pass
        def glTexCoord4fv(const_GLfloat_ptr_v): pass
        def glTexCoord4i(GLint_s, GLint_t, GLint_r, GLint_q): pass
        def glTexCoord4iv(const_GLint_ptr_v): pass
        def glTexCoord4s(GLshort_s, GLshort_t, GLshort_r, GLshort_q): pass
        def glTexCoord4sv(const_GLshort_ptr_v): pass

        def glTexGend(GLenum_coord, GLenum_pname, GLdouble_param): pass
        def glTexGendv(GLenum_coord, GLenum_pname, const_GLdouble_ptr_params): pass
        def glTexGenf(GLenum_coord, GLenum_pname, GLfloat_param): pass
        def glTexGenfv(GLenum_coord, GLenum_pname, const_GLfloat_ptr_params): pass
        def glTexGeni(GLenum_coord, GLenum_pname, GLint_param): pass
        def glTexGeniv(GLenum_coord, GLenum_pname, const_GLint_ptr_params): pass

        def glTranslated(GLdouble_x, GLdouble_y, GLdouble_z): pass
        def glTranslatef(GLfloat_x, GLfloat_y, GLfloat_z): pass
        def glVertex2d(GLdouble_x, GLdouble_y): pass
        def glVertex2dv(const_GLdouble_ptr_v): pass
        def glVertex2f(GLfloat_x, GLfloat_y): pass
        def glVertex2fv(const_GLfloat_ptr_v): pass
        def glVertex2i(GLint_x, GLint_y): pass
        def glVertex2iv(const_GLint_ptr_v): pass
        def glVertex2s(GLshort_x, GLshort_y): pass
        def glVertex2sv(const_GLshort_ptr_v): pass
        def glVertex3d(GLdouble_x, GLdouble_y, GLdouble_z): pass
        def glVertex3dv(const_GLdouble_ptr_v): pass
        def glVertex3f(GLfloat_x, GLfloat_y, GLfloat_z): pass
        def glVertex3fv(const_GLfloat_ptr_v): pass
        def glVertex3i(GLint_x, GLint_y, GLint_z): pass
        def glVertex3iv(const_GLint_ptr_v): pass
        def glVertex3s(GLshort_x, GLshort_y, GLshort_z): pass
        def glVertex3sv(const_GLshort_ptr_v): pass
        def glVertex4d(GLdouble_x, GLdouble_y, GLdouble_z, GLdouble_w): pass
        def glVertex4dv(const_GLdouble_ptr_v): pass
        def glVertex4f(GLfloat_x, GLfloat_y, GLfloat_z, GLfloat_w): pass
        def glVertex4fv(const_GLfloat_ptr_v): pass
        def glVertex4i(GLint_x, GLint_y, GLint_z, GLint_w): pass
        def glVertex4iv(const_GLint_ptr_v): pass
        def glVertex4s(GLshort_x, GLshort_y, GLshort_z, GLshort_w): pass
        def glVertex4sv(const_GLshort_ptr_v): pass

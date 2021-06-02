#include "PCH.h"

#include"gfx/gl/OpenGL.h"

void CheckOpenGLError(const char* stmt, const char* fname, int line)
{
	GLenum err = glGetError();
	if (err != GL_NO_ERROR)
	{
		std::stringstream str;
		cs::log::print(cs::LogError, "OpenGL Error: ", err, " at ", fname, ":", line, " for ", stmt);
		// abort();
	}
}

namespace cs
{
	
#if defined(CS_WINDOWS)
	void *GetAnyGLFuncAddress(const char *name)
	{
		void *p = (void *)wglGetProcAddress(name);
		if (p == 0 ||
			(p == (void*)0x1) ||
			(p == (void*)0x2) ||
			(p == (void*)0x3) ||
			(p == (void*)-1))
		{
			HMODULE module = LoadLibraryA("glew32.dll");
			p = (void *)GetProcAddress(module, name);
		}

		return p;
	}
#else
    void *GetAnyGLFuncAddress(const char *name)
    {
        assert(false);
    }
#endif

}

namespace gltest
{
    GLuint programObject;
    GLuint vertexBuffer;
    GLuint indexBuffer;
    
    void checkError()
    {
        GLenum err(glGetError());
        
        while(err != GL_NO_ERROR)
        {
            std::string error;
            
            switch (err)
            {
                case GL_INVALID_OPERATION:      error = "INVALID_OPERATION";      break;
                case GL_INVALID_ENUM:           error = "INVALID_ENUM";           break;
                case GL_INVALID_VALUE:          error = "INVALID_VALUE";          break;
                case GL_OUT_OF_MEMORY:          error = "OUT_OF_MEMORY";          break;
                case GL_INVALID_FRAMEBUFFER_OPERATION:  error = "INVALID_FRAMEBUFFER_OPERATION";  break;
            }
            
            std::cerr << "GL_" << error.c_str() <<" - "<< std::endl;
            err = glGetError();
        }
    }

    GLuint loadShader(GLenum type, const GLchar *shaderSrc)
    {
        GLuint shader;
        GLint compiled;
        // Create the shader object
        checkError();
        
        shader = glCreateShader(type);
        checkError();
        
        if (shader == 0)
        {
            std::cerr << "Could not create OpenGL shader " << std::endl;
            return 0;
        }
        
        // Load the shader source
        
        glShaderSource(shader, 1, &shaderSrc, NULL);
        
        // Compile the shader
        glCompileShader(shader);
        
        // Check the compile status
        glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
        
        if (!compiled)
        {
            GLint infoLen = 0;
            
            glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLen);
            
            if (infoLen > 1)
            {
                char* infoLog = (char*)malloc(sizeof(char) * infoLen);
                
                glGetShaderInfoLog(shader, infoLen, NULL, infoLog);
                std::cerr << "Error compiling shader: " << infoLog << std::endl;
                free(infoLog);
            }
            
            glDeleteShader(shader);
            return 0;
        }
        
        return shader;
        
    }


    ///
    // Initialize the shader and program object
    //
    int initialize()
    {
        
        const char vertexShaderString[] =
        "attribute vec4 vPosition;    \n"
        "void main()                  \n"
        "{                            \n"
        "   gl_Position = vPosition;  \n"
        "}                            \n";
        
        const char fragmentShaderString[] =
        "precision mediump float;\n"\
        "void main()                                  \n"
        "{                                            \n"
        "  gl_FragColor = vec4 ( 1.0, 0.0, 0.0, 1.0 );\n"
        "}                                            \n";
        
        GLuint vertexShader;
        GLuint fragmentShader;
        GLint linked;
        
        vertexShader = loadShader(GL_VERTEX_SHADER, vertexShaderString);
        fragmentShader = loadShader(GL_FRAGMENT_SHADER, fragmentShaderString);
        
        programObject = glCreateProgram();
        
        if (programObject == 0)
        {
            std::cerr << "Could not create OpenGL program" << std::endl;
            return 0;
        }
        
        glAttachShader(programObject, vertexShader);
        glAttachShader(programObject, fragmentShader);
        glBindAttribLocation(programObject, 0, "vPosition");
        glLinkProgram(programObject);
        
        glGetProgramiv(programObject, GL_LINK_STATUS, &linked);
        
        if (!linked)
        {
            GLint infoLen = 0;
            glGetProgramiv(programObject, GL_INFO_LOG_LENGTH, &infoLen);
            if (infoLen > 1)
            {
                
                char* infoLog = (char*) malloc (sizeof (char) * infoLen);
                glGetProgramInfoLog(programObject, infoLen, NULL, infoLog);
                std::cerr << "Error linking program: " << infoLog << std::endl;
                free (infoLog);
                
            }
            
            glDeleteProgram(programObject);
            return 0;
            
        }
        
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        
        GLfloat vVertices[] = {
            0.0f, 1.0f, 0.0f,
            -1.0f, -1.0f, 0.0f,
            1.0f, -1.0f, 0.0f
        };
        
        //
        glGenBuffers(1, &vertexBuffer);
        glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vVertices), vVertices, GL_DYNAMIC_DRAW);
        
        uint16 indices[] = {0, 1, 2};
        glGenBuffers(1, &indexBuffer);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_DYNAMIC_DRAW);
        
        return true;
    }
    
    void update()
    {
        glClearColor(255.0f, 0.0f, 255.0f, 1);
        
        glViewport (0, 0, 320, 480);
        
        glColorMask (GL_TRUE, GL_TRUE, GL_TRUE, GL_FALSE);
        glClear (GL_COLOR_BUFFER_BIT);
        
        glUseProgram(programObject);
        
        glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), 0);
        glEnableVertexAttribArray(0);
        
        //glDrawArrays (GL_TRIANGLES, 0, 3);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
        glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_SHORT, (void*)0);
    }
    
    bool testFrameBuffers()
    {
        GLuint t;
        GLuint f;
        GLsizei w = 1024;
        GLsizei h = 1024;
        {
            GLint defaultFrameBuffer;
            glGetIntegerv(GL_FRAMEBUFFER_BINDING, &defaultFrameBuffer);
            
            glGenFramebuffers(1, &f);
            glGenTextures(1, &t);
            
            glBindFramebuffer(defaultFrameBuffer, f);
            
            glBindTexture(GL_TEXTURE_2D, t);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, t, 0);
            
            GLuint depthbuffer;
            glGenRenderbuffers(1, &depthbuffer);
            glBindRenderbuffer(GL_RENDERBUFFER, depthbuffer);
            glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, w, h);
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthbuffer);
            
            bool ret = glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;
           
            glBindFramebuffer(defaultFrameBuffer, defaultFrameBuffer);
            return ret;
        }
    }
}

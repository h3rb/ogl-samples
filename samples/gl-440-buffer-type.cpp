//**********************************
// OpenGL Buffer Type
// 10/05/2010
//**********************************
// Christophe Riccio
// ogl-samples@g-truc.net
//**********************************
// G-Truc Creation
// www.g-truc.net
//**********************************

#include <glf/glf.hpp>

namespace
{
	char const * SAMPLE_NAME("OpenGL Buffer Type");
	char const * VERT_SHADER_SOURCE("gl-440/buffer-type.vert");
	char const * FRAG_SHADER_SOURCE("gl-440/buffer-type.frag");
	int const SAMPLE_SIZE_WIDTH(640);
	int const SAMPLE_SIZE_HEIGHT(480);
	int const SAMPLE_MAJOR_VERSION(4);
	int const SAMPLE_MINOR_VERSION(4);

	class i10i10i10i2
	{
	public:
		i10i10i10i2(float x, float y, float z) :
			x(int(511.0f * x)),
			y(int(511.0f * y)),
			z(int(511.0f * z)),
			w(1)
		{}

		operator glm::vec3()
		{
			return glm::vec3(
				float(x) / 511.0f,
				float(y) / 511.0f,
				float(z) / 511.0f);
		}

	private:
		int x : 10;
		int y : 10;
		int z : 10;
		int w : 2;
	};

	glf::window Window(glm::ivec2(SAMPLE_SIZE_WIDTH, SAMPLE_SIZE_HEIGHT));

	GLsizei const VertexCount(6);
	GLsizeiptr const PositionSizeF16 = VertexCount * sizeof(glm::half) * 2;
	glm::half const PositionDataF16[VertexCount * 2] =
	{
		glm::half(-1.0f), glm::half(-1.0f),
		glm::half( 1.0f), glm::half(-1.0f),
		glm::half( 1.0f), glm::half( 1.0f),
		glm::half( 1.0f), glm::half( 1.0f),
		glm::half(-1.0f), glm::half( 1.0f),
		glm::half(-1.0f), glm::half(-1.0f)
	};

	GLsizeiptr const PositionSizeF32 = VertexCount * sizeof(glm::vec2);
	glm::vec2 const PositionDataF32[VertexCount] =
	{
		glm::vec2(-1.0f,-1.0f),
		glm::vec2( 1.0f,-1.0f),
		glm::vec2( 1.0f, 1.0f),
		glm::vec2( 1.0f, 1.0f),
		glm::vec2(-1.0f, 1.0f),
		glm::vec2(-1.0f,-1.0f)
	};

	GLsizeiptr const PositionSizeI8 = VertexCount * sizeof(glm::i8vec2);
	glm::i8vec2 const PositionDataI8[VertexCount] =
	{
		glm::i8vec2(-1,-1),
		glm::i8vec2( 1,-1),
		glm::i8vec2( 1, 1),
		glm::i8vec2( 1, 1),
		glm::i8vec2(-1, 1),
		glm::i8vec2(-1,-1)
	};

	GLsizeiptr const PositionSizeRGB10A2 = VertexCount * sizeof(i10i10i10i2);
	i10i10i10i2 const PositionDataRGB10A2[VertexCount] =
	{
		i10i10i10i2(-1.0f,-1.0f, 0.0f),
		i10i10i10i2( 1.0f,-1.0f, 0.0f),
		i10i10i10i2( 1.0f, 1.0f, 0.0f),
		i10i10i10i2( 1.0f, 1.0f, 0.0f),
		i10i10i10i2(-1.0f, 1.0f, 0.0f),
		i10i10i10i2(-1.0f,-1.0f, 0.0f)
	};

	GLsizeiptr const PositionSizeI32 = VertexCount * sizeof(glm::i32vec2);
	glm::i32vec2 const PositionDataI32[VertexCount] =
	{
		glm::i32vec2(-1,-1),
		glm::i32vec2( 1,-1),
		glm::i32vec2( 1, 1),
		glm::i32vec2( 1, 1),
		glm::i32vec2(-1, 1),
		glm::i32vec2(-1,-1)
	};

	namespace buffer
	{
		enum type
		{
			RGB10A2,
			//F16,
			F32,
			I8,
			I32,
			TRANSFORM,
			MAX
		};
	}

	GLuint PipelineName(0);
	GLuint ProgramName(0);
	GLint UniformMVP(0);
	std::vector<GLuint> BufferName(buffer::MAX);
	std::vector<GLuint> VertexArrayName(buffer::MAX);
	std::vector<glm::ivec4> Viewport(buffer::MAX);

}//namespace

bool initDebugOutput()
{
	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS_ARB);
	glDebugMessageControlARB(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, GL_TRUE);
	glDebugMessageCallbackARB(&glf::debugOutput, NULL);

	return glf::checkError("initDebugOutput");
}

bool initProgram()
{
	bool Validated(true);
	
	glGenProgramPipelines(1, &PipelineName);

	if(Validated)
	{
		glf::compiler Compiler;
		GLuint VertShaderName = Compiler.create(GL_VERTEX_SHADER, 
			glf::DATA_DIRECTORY + VERT_SHADER_SOURCE, 
			"--version 440 --profile core");
		GLuint FragShaderName = Compiler.create(GL_FRAGMENT_SHADER, 
			glf::DATA_DIRECTORY + FRAG_SHADER_SOURCE,
			"--version 440 --profile core");
		Validated = Validated && Compiler.check();

		ProgramName = glCreateProgram();
		glProgramParameteri(ProgramName, GL_PROGRAM_SEPARABLE, GL_TRUE);
		glAttachShader(ProgramName, VertShaderName);
		glAttachShader(ProgramName, FragShaderName);
		glLinkProgram(ProgramName);
		glDeleteShader(VertShaderName);
		glDeleteShader(FragShaderName);

		Validated = Validated && glf::checkProgram(ProgramName);
	}

	// Get variables locations
	if(Validated)
	{
		UniformMVP = glGetUniformLocation(ProgramName, "MVP");
	}

	if(Validated)
		glUseProgramStages(PipelineName, GL_VERTEX_SHADER_BIT | GL_FRAGMENT_SHADER_BIT, ProgramName);

	return Validated && glf::checkError("initProgram");
}

bool initBuffer()
{
	// Generate a buffer object
	glGenBuffers(buffer::MAX, &BufferName[0]);

	// Allocate and copy buffers memory
	//glBindBuffer(GL_ARRAY_BUFFER, BufferName[buffer::F16]);
	//glBufferStorage(GL_ARRAY_BUFFER, PositionSizeF16, PositionDataF16, 0);

	glBindBuffer(GL_ARRAY_BUFFER, BufferName[buffer::F32]);
	glBufferStorage(GL_ARRAY_BUFFER, PositionSizeF32, PositionDataF32, 0);

	glBindBuffer(GL_ARRAY_BUFFER, BufferName[buffer::I8]);
	glBufferStorage(GL_ARRAY_BUFFER, PositionSizeI8, PositionDataI8, 0);

	glBindBuffer(GL_ARRAY_BUFFER, BufferName[buffer::I32]);
	glBufferStorage(GL_ARRAY_BUFFER, PositionSizeI32, PositionDataI32, 0);

	glBindBuffer(GL_ARRAY_BUFFER, BufferName[buffer::RGB10A2]);
	glBufferStorage(GL_ARRAY_BUFFER, PositionSizeRGB10A2, PositionDataRGB10A2, 0);

	GLint UniformBufferOffset(0);

	glGetIntegerv(
		GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT,
		&UniformBufferOffset);

	GLint UniformBlockSize = glm::max(GLint(sizeof(glm::mat4)), UniformBufferOffset);

	glBindBuffer(GL_UNIFORM_BUFFER, BufferName[buffer::TRANSFORM]);
	glBufferData(GL_UNIFORM_BUFFER, UniformBlockSize, NULL, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	return glf::checkError("initBuffer");
}

bool initVertexArray()
{
	glGenVertexArrays(buffer::MAX, &VertexArrayName[0]);

	/*
	glBindVertexArray(VertexArrayName[buffer::F16]);
		glBindBuffer(GL_ARRAY_BUFFER, BufferName[buffer::F16]);
		glVertexAttribPointer(glf::semantic::attr::POSITION, 2, GL_HALF_FLOAT, GL_FALSE, sizeof(glm::hvec2), GLF_BUFFER_OFFSET(0));
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		glEnableVertexAttribArray(glf::semantic::attr::POSITION);
	glBindVertexArray(0);
	*/

	glBindVertexArray(VertexArrayName[buffer::F32]);
		glBindBuffer(GL_ARRAY_BUFFER, BufferName[buffer::F32]);
		glVertexAttribPointer(glf::semantic::attr::POSITION, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), GLF_BUFFER_OFFSET(0));
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		glEnableVertexAttribArray(glf::semantic::attr::POSITION);
	glBindVertexArray(0);

	glBindVertexArray(VertexArrayName[buffer::I8]);
		glBindBuffer(GL_ARRAY_BUFFER, BufferName[buffer::I8]);
		glVertexAttribPointer(glf::semantic::attr::POSITION, 2, GL_BYTE, GL_FALSE, sizeof(glm::u8vec2), GLF_BUFFER_OFFSET(0));
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		glEnableVertexAttribArray(glf::semantic::attr::POSITION);
	glBindVertexArray(0);

	glBindVertexArray(VertexArrayName[buffer::I32]);
		glBindBuffer(GL_ARRAY_BUFFER, BufferName[buffer::I32]);
		glVertexAttribPointer(glf::semantic::attr::POSITION, 2, GL_INT, GL_FALSE, sizeof(glm::i32vec2), GLF_BUFFER_OFFSET(0));
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		glEnableVertexAttribArray(glf::semantic::attr::POSITION);
	glBindVertexArray(0);

	glBindVertexArray(VertexArrayName[buffer::RGB10A2]);
		glBindBuffer(GL_ARRAY_BUFFER, BufferName[buffer::RGB10A2]);
		glVertexAttribPointer(glf::semantic::attr::POSITION, 4, GL_INT_2_10_10_10_REV, GL_TRUE, sizeof(i10i10i10i2), GLF_BUFFER_OFFSET(0));
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		glEnableVertexAttribArray(glf::semantic::attr::POSITION);
	glBindVertexArray(0);

	return glf::checkError("initVertexArray");
}

bool begin()
{
	Viewport[buffer::RGB10A2] = glm::ivec4(0, 0, Window.Size >> 1);
	Viewport[buffer::F32] = glm::ivec4(Window.Size.x >> 1, 0, Window.Size >> 1);
	Viewport[buffer::I8]  = glm::ivec4(Window.Size.x >> 1, Window.Size.y >> 1, Window.Size >> 1);
	Viewport[buffer::I32] = glm::ivec4(0, Window.Size.y >> 1, Window.Size >> 1);

	bool Validated = true;
	Validated = Validated && glf::checkGLVersion(SAMPLE_MAJOR_VERSION, SAMPLE_MINOR_VERSION);

	if(Validated && glf::checkExtension("GL_ARB_debug_output"))
		Validated = initDebugOutput();
	if(Validated)
		Validated = initProgram();
	if(Validated)
		Validated = initBuffer();
	if(Validated)
		Validated = initVertexArray();

	return Validated && glf::checkError("begin");
}

bool end()
{
	glDeleteBuffers(buffer::MAX, &BufferName[0]);
	glDeleteVertexArrays(buffer::MAX, &VertexArrayName[0]);
	glDeleteProgramPipelines(1, &PipelineName);
	glDeleteProgram(ProgramName);

	return glf::checkError("end");
}

void display()
{
	{
		glBindBuffer(GL_UNIFORM_BUFFER, BufferName[buffer::TRANSFORM]);
		glm::mat4* Pointer = (glm::mat4*)glMapBufferRange(
			GL_UNIFORM_BUFFER, 0,	sizeof(glm::mat4),
			GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);

		// Compute the MVP (Model View Projection matrix)
		glm::mat4 Projection = glm::perspective(45.0f, 4.0f / 3.0f, 0.1f, 100.0f);
		glm::mat4 ViewTranslateZ = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -Window.TranlationCurrent.y));
		glm::mat4 ViewRotateX = glm::rotate(ViewTranslateZ, Window.RotationCurrent.y, glm::vec3(1.f, 0.f, 0.f));
		glm::mat4 ViewRotateY = glm::rotate(ViewRotateX, Window.RotationCurrent.x, glm::vec3(0.f, 1.f, 0.f));
		glm::mat4 View = ViewRotateY;
		glm::mat4 Model = glm::mat4(1.0f);
		glm::mat4 MVP = Projection * View * Model;

		*Pointer = MVP;

		// Make sure the uniform buffer is uploaded
		glUnmapBuffer(GL_UNIFORM_BUFFER);
	}

	glClearBufferfv(GL_COLOR, 0, &glm::vec4(1.0f)[0]);

	glBindProgramPipeline(PipelineName);
	glBindBufferBase(GL_UNIFORM_BUFFER, glf::semantic::uniform::TRANSFORM0, BufferName[buffer::TRANSFORM]);

	for(std::size_t Index = 0; Index < buffer::MAX; ++Index)
	{
		glViewport(Viewport[Index].x, Viewport[Index].y, Viewport[Index].z, Viewport[Index].w);

		glBindVertexArray(VertexArrayName[Index]);
		glDrawArraysInstanced(GL_TRIANGLES, 0, VertexCount, 1);
	}

	glf::checkError("display");
	glf::swapBuffers();
}

int main(int argc, char* argv[])
{
	return glf::run(
		argc, argv,
		glm::ivec2(::SAMPLE_SIZE_WIDTH, ::SAMPLE_SIZE_HEIGHT), 
		GLF_CONTEXT_CORE_PROFILE_BIT, 
		::SAMPLE_MAJOR_VERSION, ::SAMPLE_MINOR_VERSION);
}

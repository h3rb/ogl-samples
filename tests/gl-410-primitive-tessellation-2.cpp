//**********************************
// OpenGL Tessellation Pipeline
// 14/05/2010 - 21/09/2010
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
	glf::window Window("gl-410-primitive-tessellation-2");

	std::string const SAMPLE_VERT_SHADER("gl-410/tess.vert");
	std::string const SAMPLE_CONT_SHADER("gl-410/tess.cont");
	std::string const SAMPLE_EVAL_SHADER("gl-410/tess.eval");
	std::string const SAMPLE_GEOM_SHADER("gl-410/tess.geom");
	std::string const SAMPLE_FRAG_SHADER("gl-410/tess.frag");

	GLsizei const VertexCount(4);
	GLsizeiptr const VertexSize = VertexCount * sizeof(glf::vertex_v2fc4f);
	glf::vertex_v2fc4f const VertexData[VertexCount] =
	{
		glf::vertex_v2fc4f(glm::vec2(-1.0f,-1.0f), glm::vec4(1.0f, 0.0f, 0.0f, 1.0f)),
		glf::vertex_v2fc4f(glm::vec2( 1.0f,-1.0f), glm::vec4(1.0f, 1.0f, 0.0f, 1.0f)),
		glf::vertex_v2fc4f(glm::vec2( 1.0f, 1.0f), glm::vec4(0.0f, 1.0f, 0.0f, 1.0f)),
		glf::vertex_v2fc4f(glm::vec2(-1.0f, 1.0f), glm::vec4(0.0f, 0.0f, 1.0f, 1.0f))
	};

	namespace program
	{
		enum type
		{
			VERT,
			FRAG,
			MAX
		};
	}//namespace program

	GLuint PipelineName(0);
	GLuint ProgramName[program::MAX] = {0, 0};
	GLuint ArrayBufferName(0);
	GLuint VertexArrayName(0);
	GLint UniformMVP(0);
}//namespace

bool initProgram()
{
	bool Validated = true;
	
	glGenProgramPipelines(1, &PipelineName);
	glBindProgramPipeline(PipelineName);
	glBindProgramPipeline(0);

	if(Validated)
	{
		glf::compiler Compiler;
		GLuint VertShaderName = Compiler.create(GL_VERTEX_SHADER, glf::DATA_DIRECTORY + SAMPLE_VERT_SHADER, "--version 410 --profile core");
		GLuint ContShaderName = Compiler.create(GL_TESS_CONTROL_SHADER, glf::DATA_DIRECTORY + SAMPLE_CONT_SHADER, "--version 410 --profile core");
		GLuint EvalShaderName = Compiler.create(GL_TESS_EVALUATION_SHADER, glf::DATA_DIRECTORY + SAMPLE_EVAL_SHADER, "--version 410 --profile core");
		GLuint GeomShaderName = Compiler.create(GL_GEOMETRY_SHADER, glf::DATA_DIRECTORY + SAMPLE_GEOM_SHADER, "--version 410 --profile core");
		GLuint FragShaderName = Compiler.create(GL_FRAGMENT_SHADER, glf::DATA_DIRECTORY + SAMPLE_FRAG_SHADER, "--version 410 --profile core");
		Validated = Validated && Compiler.check();

		ProgramName[program::VERT] = glCreateProgram();
		ProgramName[program::FRAG] = glCreateProgram();
		glProgramParameteri(ProgramName[program::VERT], GL_PROGRAM_SEPARABLE, GL_TRUE);
		glProgramParameteri(ProgramName[program::FRAG], GL_PROGRAM_SEPARABLE, GL_TRUE);

		glAttachShader(ProgramName[program::VERT], VertShaderName);
		glAttachShader(ProgramName[program::VERT], ContShaderName);
		glAttachShader(ProgramName[program::VERT], EvalShaderName);
		glAttachShader(ProgramName[program::VERT], GeomShaderName);
		glLinkProgram(ProgramName[program::VERT]);

		glAttachShader(ProgramName[program::FRAG], FragShaderName);
		glLinkProgram(ProgramName[program::FRAG]);

		glDeleteShader(VertShaderName);
		glDeleteShader(ContShaderName);
		glDeleteShader(EvalShaderName);
		glDeleteShader(GeomShaderName);
		glDeleteShader(FragShaderName);

		Validated = Validated && glf::checkProgram(ProgramName[program::VERT]);
		Validated = Validated && glf::checkProgram(ProgramName[program::FRAG]);
	}

	if(Validated)
	{
		glUseProgramStages(PipelineName, GL_VERTEX_SHADER_BIT | GL_TESS_CONTROL_SHADER_BIT | GL_TESS_EVALUATION_SHADER_BIT | GL_GEOMETRY_SHADER_BIT, ProgramName[program::VERT]);
		glUseProgramStages(PipelineName, GL_FRAGMENT_SHADER_BIT, ProgramName[program::FRAG]);
	}

	if(Validated)
	{
		UniformMVP = glGetUniformLocation(ProgramName[program::VERT], "MVP");
	}

	return Validated && glf::checkError("initProgram");
}

bool initVertexArray()
{
	glGenVertexArrays(1, &VertexArrayName);
	glBindVertexArray(VertexArrayName);
		glBindBuffer(GL_ARRAY_BUFFER, ArrayBufferName);
		glVertexAttribPointer(glf::semantic::attr::POSITION, 2, GL_FLOAT, GL_FALSE, sizeof(glf::vertex_v2fc4f), GLF_BUFFER_OFFSET(0));
		glVertexAttribPointer(glf::semantic::attr::COLOR, 4, GL_FLOAT, GL_FALSE, sizeof(glf::vertex_v2fc4f), GLF_BUFFER_OFFSET(sizeof(glm::vec2)));
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		glEnableVertexAttribArray(glf::semantic::attr::POSITION);
		glEnableVertexAttribArray(glf::semantic::attr::COLOR);
	glBindVertexArray(0);

	return glf::checkError("initVertexArray");
}

bool initArrayBuffer()
{
	// Generate a buffer object
	glGenBuffers(1, &ArrayBufferName);
	glBindBuffer(GL_ARRAY_BUFFER, ArrayBufferName);
	glBufferData(GL_ARRAY_BUFFER, VertexSize, VertexData, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	return glf::checkError("initArrayBuffer");
}

bool begin()
{
	bool Validated = true;

	if(Validated)
		Validated = initProgram();
	if(Validated)
		Validated = initArrayBuffer();
	if(Validated)
		Validated = initVertexArray();

	return Validated && glf::checkError("begin");
}

bool end()
{
	glDeleteVertexArrays(1, &VertexArrayName);
	glDeleteBuffers(1, &ArrayBufferName);
	for(std::size_t i = 0; i < program::MAX; ++i)
		glDeleteProgram(ProgramName[i]);

	return glf::checkError("end");
}

void display()
{
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	glm::mat4 Projection = glm::perspective(glm::pi<float>() * 0.25f, 4.0f / 3.0f, 0.1f, 100.0f);
	glm::mat4 ViewTranslate = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -Window.TranlationCurrent.y));
	glm::mat4 ViewRotateX = glm::rotate(ViewTranslate, Window.RotationCurrent.y, glm::vec3(1.f, 0.f, 0.f));
	glm::mat4 View = glm::rotate(ViewRotateX, Window.RotationCurrent.x, glm::vec3(0.f, 1.f, 0.f));
	glm::mat4 Model = glm::mat4(1.0f);
	glm::mat4 MVP = Projection * View * Model;

	glProgramUniformMatrix4fv(ProgramName[program::VERT], UniformMVP, 1, GL_FALSE, &MVP[0][0]);

	glViewportIndexedfv(0, &glm::vec4(0, 0, Window.Size)[0]);
	glClearBufferfv(GL_COLOR, 0, &glm::vec4(0.0f)[0]);

	glBindProgramPipeline(PipelineName);

	glBindVertexArray(VertexArrayName);
	glPatchParameteri(GL_PATCH_VERTICES, VertexCount);
	glDrawArraysInstanced(GL_PATCHES, 0, VertexCount, 1);

	glf::checkError("display");

}

int main(int argc, char* argv[])
{
	return glf::run(argc, argv, glf::CORE, 4, 1);
}

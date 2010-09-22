//**********************************
// OpenGL Image 2D Compressed
// 27/08/2009
//**********************************
// Christophe Riccio
// g.truc.creation@gmail.com
//**********************************
// G-Truc Creation
// www.g-truc.net
//**********************************

#include <glf/glf.hpp>

namespace
{
	std::string const SAMPLE_NAME = "OpenGL Image 2D Compressed";
	std::string const VERTEX_SHADER_SOURCE(glf::DATA_DIRECTORY + "330/image-2d.vert");
	std::string const FRAGMENT_SHADER_SOURCE(glf::DATA_DIRECTORY + "330/image-2d.frag");
	std::string const TEXTURE_DIFFUSE_RGB8(glf::DATA_DIRECTORY + "kueken256-rgb8.dds");
	std::string const TEXTURE_DIFFUSE_DXT1(glf::DATA_DIRECTORY + "kueken256-dxt1.dds");
	std::string const TEXTURE_DIFFUSE_DXT3(glf::DATA_DIRECTORY + "kueken256-dxt3.dds");
	std::string const TEXTURE_DIFFUSE_DXT5(glf::DATA_DIRECTORY + "kueken256-dxt5.dds");
	int const SAMPLE_SIZE_WIDTH = 640;
	int const SAMPLE_SIZE_HEIGHT = 480;
	int const SAMPLE_MAJOR_VERSION = 3;
	int const SAMPLE_MINOR_VERSION = 3;

	glf::window Window(glm::ivec2(SAMPLE_SIZE_WIDTH, SAMPLE_SIZE_HEIGHT));

	struct vertex
	{
		vertex
		(
			glm::vec2 const & Position,
			glm::vec2 const & Texcoord
		) :
			Position(Position),
			Texcoord(Texcoord)
		{}

		glm::vec2 Position;
		glm::vec2 Texcoord;
	};

	// With DDS textures, v texture coordinate are reversed, from top to bottom
	GLsizei const VertexCount = 6;
	GLsizeiptr const VertexSize = VertexCount * sizeof(vertex);
	vertex const VertexData[VertexCount] =
	{
		vertex(glm::vec2(-1.0f,-1.0f), glm::vec2(0.0f, 1.0f)),
		vertex(glm::vec2( 1.0f,-1.0f), glm::vec2(1.0f, 1.0f)),
		vertex(glm::vec2( 1.0f, 1.0f), glm::vec2(1.0f, 0.0f)),
		vertex(glm::vec2( 1.0f, 1.0f), glm::vec2(1.0f, 0.0f)),
		vertex(glm::vec2(-1.0f, 1.0f), glm::vec2(0.0f, 0.0f)),
		vertex(glm::vec2(-1.0f,-1.0f), glm::vec2(0.0f, 1.0f))
	};

	enum texture_type
	{
		TEXTURE_RGB8,
		TEXTURE_COMP,
		TEXTURE_DXT1,
		TEXTURE_DXT5,
		TEXTURE_MAX
	};

	GLuint VertexArrayName = 0;
	GLuint ProgramName = 0;

	GLuint BufferName = 0;
	GLuint Texture2DName[TEXTURE_MAX];

	GLuint UniformMVP = 0;
	GLuint UniformDiffuse = 0;

	glm::ivec4 Viewport[TEXTURE_MAX];

}//namespace

bool initProgram()
{
	bool Validated = true;
	
	if(Validated)
	{
		GLuint VertexShaderName = glf::createShader(GL_VERTEX_SHADER, VERTEX_SHADER_SOURCE);
		GLuint FragmentShaderName = glf::createShader(GL_FRAGMENT_SHADER, FRAGMENT_SHADER_SOURCE);

		ProgramName = glCreateProgram();
		glAttachShader(ProgramName, VertexShaderName);
		glAttachShader(ProgramName, FragmentShaderName);
		glDeleteShader(VertexShaderName);
		glDeleteShader(FragmentShaderName);

		glLinkProgram(ProgramName);
		Validated = glf::checkProgram(ProgramName);
	}

	if(Validated)
	{
		UniformMVP = glGetUniformLocation(ProgramName, "MVP");
		UniformDiffuse = glGetUniformLocation(ProgramName, "Diffuse");
	}

	// Set some variables 
	if(Validated)
	{
		// Bind the program for use
		glUseProgram(ProgramName);

		// Set uniform value
		glUniform1i(UniformDiffuse, 0);

		// Unbind the program
		glUseProgram(0);
	}

	return glf::checkError("initProgram");
}

bool initArrayBuffer()
{
	glGenBuffers(1, &BufferName);

    glBindBuffer(GL_ARRAY_BUFFER, BufferName);
    glBufferData(GL_ARRAY_BUFFER, VertexSize, VertexData, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	return glf::checkError("initArrayBuffer");;
}

bool initTexture2D()
{
	glActiveTexture(GL_TEXTURE0);
	glGenTextures(TEXTURE_MAX, Texture2DName);

	// Set image
	{
		glBindTexture(GL_TEXTURE_2D, Texture2DName[TEXTURE_RGB8]);

		gli::image Image = gli::import_as(TEXTURE_DIFFUSE_RGB8);
		for(std::size_t Level = 0; Level < Image.levels(); ++Level)
		{
			glTexImage2D(
				GL_TEXTURE_2D, 
				GLint(Level), 
				GL_RGB,
				GLsizei(Image[Level].dimensions().x), 
				GLsizei(Image[Level].dimensions().y), 
				0,  
				GL_BGR, 
				GL_UNSIGNED_BYTE, 
				Image[Level].data());
		}
	}

	{
		glBindTexture(GL_TEXTURE_2D, Texture2DName[TEXTURE_COMP]);

		gli::image Image = gli::import_as(TEXTURE_DIFFUSE_RGB8);
		for(std::size_t Level = 0; Level < Image.levels(); ++Level)
		{
			glTexImage2D(
				GL_TEXTURE_2D, 
				GLint(Level), 
				GL_COMPRESSED_RGB,
				GLsizei(Image[Level].dimensions().x), 
				GLsizei(Image[Level].dimensions().y), 
				0,  
				GL_BGR, 
				GL_UNSIGNED_BYTE, 
				Image[Level].data());
		}
	}

	{
		gli::image Image = gli::import_as(TEXTURE_DIFFUSE_DXT1);

		glBindTexture(GL_TEXTURE_2D, Texture2DName[TEXTURE_DXT1]);

		for(std::size_t Level = 0; Level < Image.levels(); ++Level)
		{
			glCompressedTexImage2D(
				GL_TEXTURE_2D,
				GLint(Level),
				GL_COMPRESSED_RGB_S3TC_DXT1_EXT,
				GLsizei(Image[Level].dimensions().x), 
				GLsizei(Image[Level].dimensions().y), 
				0, 
				GLsizei(Image[Level].capacity()), 
				Image[Level].data());
		}
	}

	{
		gli::image Image = gli::import_as(TEXTURE_DIFFUSE_DXT5);

		glBindTexture(GL_TEXTURE_2D, Texture2DName[TEXTURE_DXT5]);

		for(std::size_t Level = 0; Level < Image.levels(); ++Level)
		{
			glCompressedTexImage2D(
				GL_TEXTURE_2D,
				GLint(Level),
				GL_COMPRESSED_RGBA_S3TC_DXT5_EXT,
				GLsizei(Image[Level].dimensions().x), 
				GLsizei(Image[Level].dimensions().y), 
				0, 
				GLsizei(Image[Level].capacity()),
				Image[Level].data());
		}
	}

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, 0);

	return glf::checkError("initTexture2D");
}

bool initVertexArray()
{
	// Create a dummy vertex array object where all the attribute buffers and element buffers would be attached 
	glGenVertexArrays(1, &VertexArrayName);
    glBindVertexArray(VertexArrayName);
		glBindBuffer(GL_ARRAY_BUFFER, BufferName);
		glVertexAttribPointer(glf::semantic::attr::POSITION, 2, GL_FLOAT, GL_FALSE, sizeof(vertex), GLF_BUFFER_OFFSET(0));
		glVertexAttribPointer(glf::semantic::attr::TEXCOORD, 2, GL_FLOAT, GL_FALSE, sizeof(vertex), GLF_BUFFER_OFFSET(sizeof(glm::vec2)));
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		glEnableVertexAttribArray(glf::semantic::attr::POSITION);
		glEnableVertexAttribArray(glf::semantic::attr::TEXCOORD);
	glBindVertexArray(0);

	return glf::checkError("initVertexArray");
}

bool begin()
{
	Viewport[TEXTURE_RGB8] = glm::ivec4(0, 0, Window.Size >> 1);
	Viewport[TEXTURE_COMP] = glm::ivec4(Window.Size.x >> 1, 0, Window.Size >> 1);
	Viewport[TEXTURE_DXT1] = glm::ivec4(Window.Size.x >> 1, Window.Size.y >> 1, Window.Size >> 1);
	Viewport[TEXTURE_DXT5] = glm::ivec4(0, Window.Size.y >> 1, Window.Size >> 1);

	GLint MajorVersion = 0;
	GLint MinorVersion = 0;
	glGetIntegerv(GL_MAJOR_VERSION, &MajorVersion);
	glGetIntegerv(GL_MINOR_VERSION, &MinorVersion);
	bool Validated = (MajorVersion * 10 + MinorVersion) >= (SAMPLE_MAJOR_VERSION * 10 + SAMPLE_MINOR_VERSION);

	if(Validated)
		Validated = initProgram();
	if(Validated)
		Validated = initArrayBuffer();
	if(Validated)
		Validated = initVertexArray();
	if(Validated)
		Validated = initTexture2D();

	return Validated && glf::checkError("begin");
}

bool end()
{
	glDeleteBuffers(1, &BufferName);
	glDeleteProgram(ProgramName);
	glDeleteTextures(TEXTURE_MAX, Texture2DName);
	glDeleteVertexArrays(1, &VertexArrayName);

	return glf::checkError("end");
}

void display()
{
	// Compute the MVP (Model View Projection matrix)
	glm::mat4 Projection = glm::perspective(45.0f, 4.0f / 3.0f, 0.1f, 1000.0f);
	glm::mat4 ViewTranslate = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -Window.TranlationCurrent.y));
	glm::mat4 ViewRotateX = glm::rotate(ViewTranslate, Window.RotationCurrent.y, glm::vec3(1.f, 0.f, 0.f));
	glm::mat4 View = glm::rotate(ViewRotateX, Window.RotationCurrent.x, glm::vec3(0.f, 1.f, 0.f));
	glm::mat4 Model = glm::mat4(1.0f);
	glm::mat4 MVP = Projection * View * Model;

	glClearColor(1.0f, 0.5f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	// Bind the program for use
	glUseProgram(ProgramName);

	glUniformMatrix4fv(UniformMVP, 1, GL_FALSE, &MVP[0][0]);
	glUniform1i(UniformDiffuse, 0);

	glBindVertexArray(VertexArrayName);

	glActiveTexture(GL_TEXTURE0);
	for(std::size_t Index = 0; Index < TEXTURE_MAX; ++Index)
	{
		glViewport(
			Viewport[Index].x, 
			Viewport[Index].y, 
			Viewport[Index].z, 
			Viewport[Index].w);

		glBindTexture(GL_TEXTURE_2D, Texture2DName[Index]);

		glDrawArrays(GL_TRIANGLES, 0, VertexCount);
	}

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, 0);

	glf::checkError("display");
	glf::swapBuffers();
}

int main(int argc, char* argv[])
{
	if(glf::run(
		argc, argv,
		glm::ivec2(::SAMPLE_SIZE_WIDTH, ::SAMPLE_SIZE_HEIGHT), 
		::SAMPLE_MAJOR_VERSION, 
		::SAMPLE_MINOR_VERSION))
		return 0;
	return 1;
}
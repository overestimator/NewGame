#include "stdafx.h"
#include "gl.h"
#include <App\Global\Debug.h>
#include <conio.h>
#include <App\Global\App.h>
#include <array>
#include "..\BaseGL\Shader_Loader.h"
#include "..\Camera.h"
#include "glDebug.h"
#include <glm\gtc\type_ptr.hpp>
#include "..\BaseGL\Shader.h"
#include "..\BaseGL\Texture.h"
#include "..\GUI\Font_Loader.h"
#include "..\GUI\GUI.h"
#include "../Models/Render.h"
#include "../Models/Models.h"
#include "../Models/Model_Loader.h"
#include "../GUI/Text.h"
#include "../BaseGL/Framebuffer.h"
#include "../Lighting/Lights.h"
#include "../GUI/Colored_Quads.h"
#include "../GUI/Buttons.h"
int gl::MAX_WORK_GROUP_COUNT = 0;
glm::ivec3 gl::MAX_WORK_GROUP_SIZE = {};
unsigned int gl::MAX_LIGHT_COUNT = 100;
std::vector<std::string> gl::EXTENSION_LIST = {};
int gl::EXTENSIONS_SUPPORTED_NUM = 0;
int gl::OPENGL_VERSION[2] = {};
std::string gl::GLSL_VERSION = "";
std::string gl::SYSTEM_RENDERER = "";
float gl::resolution = 1.0f;
int gl::MAX_TEXTURE_UNIT_COUNT;
unsigned int gl::screenWidth = 0;
unsigned int gl::screenHeight = 0;
unsigned int gl::mergeComputeShader = 0;
unsigned int gl::finalMergeShader;
unsigned int gl::mainFrame = 0;
unsigned int gl::mainFBO  = 0;
unsigned int gl::quadVBO = 0;
unsigned int gl::quadEBO = 0; 
unsigned int gl::generalUniformBuffer = 0;
unsigned int gl::screenQuadVAO;
unsigned int gl::screenShaderProgram;


void gl::init()
{
	initGLEW();
	getOpenGLInitValues();
	Debug::generateDebugGrid("grid1.0", 1.0f, 1000, 1.0f, 1.0f, 1.0f, 0.9f);
	Debug::generateDebugGrid("grid0.5", 0.5f, 2000, 1.0f, 1.0f, 1.0f, 0.3f);
	Debug::initCoordinateSystem("coord");
	Debug::init();
	Camera::init();
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
	gl::Debug::getGLError("gl::init():");
	App::Debug::printErrors();
	//init framebuffers
	//include shaders
        Debug::initDebugShader();
        initScreenShader();
        Models::initNormalShader();
        Models::initMeshShader();
	Lighting::initLightShader();
        GUI::initButtonIndexShader();
	GUI::initColoredQuadShader();
	GUI::Text::initFontShader();
	
	Shader::Loader::buildShaderPrograms();
	
	//generals
	initGeneralUniformBuffer();
	initGeneralQuadVBO();
	Texture::initGBuffer();
	Texture::initLightFBO();
        Texture::initButtonFBO();
        initScreenVAO();
	Lighting::initLightVAO();
	Lighting::initLightDataBuffer();
	Debug::getGLError("gl::init()3");
	App::Debug::printErrors();
	
	/*FONTS
	times,
	FreeSans,
	Input_Regular_Mono,
	Input_Light_Mono,
	FreeMono,
	C64_Pro_Mono
	C64_Pro_Regular,
	Ubuntu_Regular_Mono,
	SourceCodePro_Regular,
	SourceCodePro_Medium,
	VCR_OSD_MONO
	Generic
	justabit
	BetterPixels
	UnnamedFont
	TinyUnicode
	Hack-Regular
	*/
	GUI::Text::Initializer::initFreeType();
	GUI::Text::Initializer::includeFont("Hack-Regular.ttf", 14, 30, 200, 0, 1);
	GUI::Text::Initializer::loadFonts();
	
	
	//bind uniform buffers to shaders
	
	gl::GUI::Text::initFontVAO();
	GUI::initColoredQuadVAO();
        Models::initMeshVAO();
	
	loadModels();
        Models::fillMeshVAO();
        
	Shader::bindUniformBufferToShader(Lighting::lightShaderProgram, Lighting::lightDataUBO, "LightDataBuffer");
        
        Shader::bindUniformBufferToShader(Lighting::lightShaderProgram, generalUniformBuffer, "GeneralUniformBuffer");
        App::Debug::printErrors();
	Shader::bindUniformBufferToShader(GUI::Text::glyphShapeProgram, generalUniformBuffer, "GeneralUniformBuffer");
        
	Shader::bindUniformBufferToShader(Debug::lineShaderID, generalUniformBuffer, "GeneralUniformBuffer");
        
	Shader::bindUniformBufferToShader(Models::meshShaderProgram, generalUniformBuffer, "GeneralUniformBuffer");
	Shader::bindUniformBufferToShader(Models::meshShaderProgram, Models::entityTransformBuffer, "EntityTransformBuffer");
	Shader::bindUniformBufferToShader(Models::meshShaderProgram, Models::transformIndexBuffer, "TransformIndexBuffer");
        App::Debug::printErrors();

        Shader::bindUniformBufferToShader(gl::GUI::buttonIndexShader, gl::GUI::quadBuffer, "QuadBuffer");

	Shader::bindUniformBufferToShader(gl::GUI::coloredQuadShader, gl::GUI::quadColorBuffer, "ColorBuffer");
	Shader::bindUniformBufferToShader(gl::GUI::coloredQuadShader, gl::GUI::quadBuffer, "QuadBuffer");
        App::Debug::printErrors();
	
	
	Debug::getGLError("gl::init()4");
	App::Debug::printErrors();
}

void gl::configure()
{
	setViewport(App::mainWindow);
	glBindFramebuffer(GL_FRAMEBUFFER, Texture::gBuffer);
	setViewport(App::mainWindow);
	
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void gl::setViewport(App::ContextWindow::Window& pViewport) {
	screenWidth = pViewport.width;
	screenHeight = pViewport.height;
	glViewport(0, 0, screenWidth, screenHeight);
}

void gl::getOpenGLInitValues()
{
	VAO::STREAM_FLAGS = GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT;
	glGetIntegerv(GL_MAJOR_VERSION, &OPENGL_VERSION[0]);
	glGetIntegerv(GL_MINOR_VERSION, &OPENGL_VERSION[1]);
	GLSL_VERSION = std::string((char*)glGetString(GL_SHADING_LANGUAGE_VERSION));
	SYSTEM_RENDERER = std::string((char*)glGetString(GL_RENDERER));

	glGetIntegerv(GL_NUM_EXTENSIONS, &EXTENSIONS_SUPPORTED_NUM);
	EXTENSION_LIST.resize(EXTENSIONS_SUPPORTED_NUM);
	for (int k = 0; k < EXTENSIONS_SUPPORTED_NUM; ++k) {
		EXTENSION_LIST[k] = std::string((char*)glGetStringi(GL_EXTENSIONS, k));
	}

	glGetIntegeri_v(GL_MAX_COMPUTE_FIXED_GROUP_SIZE_ARB, 0, &MAX_WORK_GROUP_SIZE.x);
	glGetIntegeri_v(GL_MAX_COMPUTE_FIXED_GROUP_SIZE_ARB, 1, &MAX_WORK_GROUP_SIZE.y);
	glGetIntegeri_v(GL_MAX_COMPUTE_FIXED_GROUP_SIZE_ARB, 2, &MAX_WORK_GROUP_SIZE.z);

	glGetIntegerv(GL_SHADER_STORAGE_BUFFER_OFFSET_ALIGNMENT, &VAO::SHADER_STORAGE_BUFFER_OFFSET_ALIGNMENT);
	glGetIntegerv(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, &VAO::UNIFORM_BUFFER_OFFSET_ALIGNMENT);
	glGetIntegerv(GL_MAX_UNIFORM_BUFFER_BINDINGS, &VAO::MAX_UNIFORM_BUFFER_BINDINGS);
	glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &MAX_TEXTURE_UNIT_COUNT);
	glGetIntegerv(GL_MIN_MAP_BUFFER_ALIGNMENT, &VAO::MIN_MAP_BUFFER_ALIGNMENT);
	screenWidth = App::mainWindow.width;
	screenHeight = App::mainWindow.height;

	//glBindImageTexture(0, mainFrame, 1, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
	Debug::getGLError("gl::initOpenGL()");
}

void gl::initGLEW() {
	unsigned int glew = glewInit();
	if (glew != GLEW_OK) {

		App::Debug::pushError(std::string("\nApp::init() - Unable to initialize GLEW (glewInit() return code: %i)\nGLEW Error Log\n %s"
			+ glew) + std::string((const char*)glewGetErrorString(glew)), App::Debug::Error::Severity::Fatal);
		while (!_getch()) {}
		exit(glew);
	}
}

void gl::initGeneralQuadVBO()
{

	/*
	3---2
	|  /|
	| / |
	|/  |
	0---1
	*/
	float varr[4 * 2] = {
		0.0f, 0.0f, 
		1.0f, 0.0f,
		1.0f, 1.0f, 
		0.0f, 1.0f
	};
	unsigned int iarr[6] = {
		0, 1, 2, 0, 2, 3
	};
	quadVBO = VAO::createStorage(sizeof(float) * 4 * 2, &varr[0], 0);
	quadEBO = VAO::createStorage(sizeof(unsigned int) * 6, &iarr[0], 0);
}

void gl::initGeneralUniformBuffer()
{
	//General Uniform buffer Contents
	//projectionMatrix
	//viewMatrix
	//camera position
	//
	unsigned int generalUniformDataSize = sizeof(float) * (16 + 16 + 4);
	
	generalUniformBuffer = VAO::createStorage(generalUniformDataSize, nullptr, GL_MAP_WRITE_BIT | VAO::STREAM_FLAGS);
	VAO::createStream(generalUniformBuffer,  GL_MAP_WRITE_BIT);
	VAO::bindStorage(GL_UNIFORM_BUFFER, generalUniformBuffer);
	Debug::getGLError("gl::initGeneralUniformBuffer()1:");
}

void gl::updateGeneralUniformBuffer()
{
	std::vector<float> generalUniformData(36);
	
	std::memcpy(&generalUniformData[0], glm::value_ptr(Camera::infiniteProjectionMatrix), sizeof(float) * 16);
	std::memcpy(&generalUniformData[16], glm::value_ptr(Camera::viewMatrix), sizeof(float) * 16);
	std::memcpy(&generalUniformData[32], glm::value_ptr(Camera::pos), sizeof(float) * 3);
	VAO::streamStorage(generalUniformBuffer, sizeof(float) * 36, &generalUniformData[0]);
	
	Debug::getGLError("gl::update():");
}

void gl::frameStart()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
}

void gl::frameEnd()
{
	
	glfwSwapBuffers(App::mainWindow.window);
	Debug::getGLError("FrameEnd");
	App::Debug::printErrors();
}

void gl::loadModels()
{
        Models::Loader::includeModel("sphere.3ds");
        Models::Loader::loadModelFiles();

}


void gl::renderScreenQuad(){
	glBindVertexArray(screenQuadVAO);
	Shader::use(screenShaderProgram);
	glDepthFunc(GL_ALWAYS);
	glActiveTexture(GL_TEXTURE0); 
	glBindTexture(GL_TEXTURE_2D, Texture::lightColorTexture);
		
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);	
	glDepthFunc(GL_LESS);
	glBindVertexArray(0);
	Shader::unuse();
}
void gl::initScreenVAO()
{
	glCreateVertexArrays(1, &screenQuadVAO);
	glVertexArrayVertexBuffer(screenQuadVAO, 0, quadVBO + 1, 0, sizeof(float) * 2);
	glVertexArrayElementBuffer(screenQuadVAO, quadEBO + 1);
	VAO::setVertexAttrib(screenQuadVAO, 0, 0, 2, GL_FLOAT, 0);
}

void gl::initScreenShader()
{
	screenShaderProgram = Shader::newProgram("screenShaderProgram", Shader::newModule("screenShaderProgram.vert"), Shader::newModule("screenShaderProgram.frag")).ID;
	Shader::addVertexAttribute(screenShaderProgram, "corner_pos", 0);

	
}

#pragma once
#include <glm\glm.hpp>
#include <vector>

namespace gl {
	namespace Models {
		void initMeshVAO();
		void initMeshShader();
		void fillMeshVAO();
		void storeMaterials();
		void render();
                void initNormalShader();
                void renderNormals();
		void updateBuffers();
		extern unsigned int meshShaderProgram;
                extern unsigned int normalShaderProgram;
		extern unsigned int meshVAO;
		extern unsigned int meshVBO;
		extern unsigned int meshIBO;
		extern unsigned int materialUBO;
		extern unsigned int entityTransformBuffer;
		extern unsigned int transformIndexBuffer;
	}
}

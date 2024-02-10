#pragma once

#include <vector>
#include <glad/glad.h>
#include <Rendering_Framework/src/Rendering/Camera/Camera.h>
#include <Rendering_Framework/src/Scene/SpatialSample.h>

typedef struct {
	int width;
	int height;
	unsigned char* data;
} TextureData;

typedef struct {
	GLuint diffuse_tex;
} Material;

typedef struct {
	int drawCounts;
} Modelc;

typedef struct {
	glm::vec4 position;
	//glm::ivec4 passed;
} InstanceProperties;

typedef struct {
	glm::vec4 position;
	glm::ivec4 passed;
} RawInstanceProperties;

typedef struct {
	unsigned int count;
	unsigned int instanceCount;
	unsigned int firstIndex;
	unsigned int baseVertex;
	unsigned int baseInstance;
} DrawElementsIndirectCommand;

using namespace std;

namespace INANOA {
	namespace SCENE {
		namespace EXPERIMENTAL {
			class Foliages
			{
			public:
				explicit Foliages(const int numCascade, const Camera* camera);
				virtual ~Foliages();

			public:
				void init();
				void update(const Camera* camera);
				void render();

				void initTextureDataArray();
				void initModels();
				void initVAO_VBO_IBO();
				void initSpatialSamples();
				void initSSBO();
				void initDrawCommands();
				Modelc* loadModels(const char* path, int textureIndex);

				void collectVisibleInstances(const glm::mat4& viewProjMat);
				void resetRenderParameters();

			private:
				Modelc* grass_model;
				Modelc* bush01_model;
				Modelc* bush05_model;

				unsigned char* textureArrayData;
				GLuint textureArrayHandle;

				GLuint vaoHandle;
				vector<GLfloat> vbo_p;
				vector<GLfloat> vbo_t;
				vector<GLfloat> vbo_n;
				vector<unsigned int> ibo;
				GLuint vboHandle_position;
				GLuint vboHandle_texcoord;
				GLuint vboHandle_normal;
				GLuint iboHandle;
				int offset_1;
				int offset_2;

				DrawElementsIndirectCommand drawCommands[3];
				GLuint cmdBufferHandle;
				vector<DrawElementsIndirectCommand> drawCommandsArray;
				vector<DrawElementsIndirectCommand> allCommands;

				SpatialSample* grass_sample;
				SpatialSample* bush01_sample;
				SpatialSample* bush05_sample;
				int bush01_num_sample = 0;
				int bush05_num_sample = 0;
				int grass_num_sample = 0;
				int total_num_sample = 0;

				RawInstanceProperties* rawInstanceData;
				GLuint rawInstanceDataBufferHandle;
				GLuint validInstanceDataBufferHandle;	
			};
		}
	}
}




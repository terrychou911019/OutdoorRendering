#include "RFoliages.h"

#include <iostream>
#include <vector>
#include <algorithm>

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/transform.hpp>
#include <Rendering_Framework/src/Rendering/ShaderParameterBindingPoint.h>
#include <Rendering_Framework/src/Scene/SpatialSample.h>

#include "../../../externals/include/stb_image.h"
#include "../../../externals/include/assimp/cimport.h"
#include "../../../externals/include/assimp/scene.h"
#include "../../../externals/include/assimp/postprocess.h"
#include "../../../externals/include/assimp/Importer.hpp"

Assimp::Importer importerFoliages;

const int NUM_TEXTURE = 3;
const int IMG_WIDTH = 1024;
const int IMG_HEIGHT = 1024;
const int IMG_CHANNEL = 4;
const int singleTextureSize = IMG_WIDTH * IMG_HEIGHT * IMG_CHANNEL;

namespace INANOA {
	namespace SCENE {
		namespace EXPERIMENTAL {
			Foliages::Foliages(const int numCascade, const Camera* camera)
			{
				this->init();
			};
			Foliages::~Foliages() {};

			void Foliages::init() {
				this->initTextureDataArray();
				this->initModels();
				this->initVAO_VBO_IBO();
				this->initSpatialSamples();
				this->initSSBO();
				this->initDrawCommands();
			};

			void Foliages::initTextureDataArray() {
				this->textureArrayData = new unsigned char[IMG_WIDTH * IMG_HEIGHT * IMG_CHANNEL * NUM_TEXTURE];
				for (int i = 0; i < NUM_TEXTURE; i++) {
					char* path;
					switch (i) {
					case 0:
						path = "textures/grassB_albedo.png";
						break;
					case 1:
						path = "textures/bush01.png";
						break;
					case 2:
						path = "textures/bush05.png";
						break;	
					default:
						path = "textures/bush01.png";
						break;
					}

					int width, height, channels;
					stbi_uc* singleTextureData = stbi_load(path, &width, &height, &channels, IMG_CHANNEL);

					if (singleTextureData) {
						size_t offset = i * singleTextureSize;
						memcpy(this->textureArrayData + offset, singleTextureData, singleTextureSize);
						stbi_image_free(singleTextureData);
						cout << "foliage texture " << path << " load successfully\n";
					}
					else {
						cout << "foliage texture " << path << " fail to load\n";
					}
				}

				glGenTextures(1, &this->textureArrayHandle);
				glBindTexture(GL_TEXTURE_2D_ARRAY, this->textureArrayHandle);
				glTexStorage3D(GL_TEXTURE_2D_ARRAY, 11, GL_RGBA8, IMG_WIDTH, IMG_HEIGHT, NUM_TEXTURE);
				glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, 0, IMG_WIDTH, IMG_HEIGHT, NUM_TEXTURE, GL_RGBA, GL_UNSIGNED_BYTE, this->textureArrayData);
				glTexParameterf(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				glTexParameterf(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				glTexParameterf(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_REPEAT);
				glTexParameterf(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_REPEAT);
				glGenerateMipmap(GL_TEXTURE_2D_ARRAY);
			}

			void Foliages::initModels() {
				const char* grass_path = "models/foliages/grassB.obj";
				const char* bush01_path = "models/foliages/bush01_lod2.obj";
				const char* bush05_path = "models/foliages/bush05_lod2.obj";
				this->grass_model = this->loadModels(grass_path, 0);
				this->bush01_model = this->loadModels(bush01_path, 1);
				this->bush05_model = this->loadModels(bush05_path, 2);	
				this->offset_1 = this->grass_model->drawCounts;
				this->offset_2 = this->grass_model->drawCounts + this->bush01_model->drawCounts;
			}

			void Foliages::initVAO_VBO_IBO() {
				glGenVertexArrays(1, &this->vaoHandle);
				glBindVertexArray(this->vaoHandle);

				glGenBuffers(1, &this->vboHandle_position);
				glGenBuffers(1, &this->vboHandle_texcoord);
				glGenBuffers(1, &this->vboHandle_normal);
				glGenBuffers(1, &this->iboHandle);

				glBindBuffer(GL_ARRAY_BUFFER, this->vboHandle_position);
				glBufferData(GL_ARRAY_BUFFER, this->vbo_p.size() * sizeof(GLfloat), &this->vbo_p.at(0), GL_STATIC_DRAW);
				glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

				glBindBuffer(GL_ARRAY_BUFFER, this->vboHandle_texcoord);
				glBufferData(GL_ARRAY_BUFFER, this->vbo_t.size() * sizeof(GLfloat), &this->vbo_t.at(0), GL_STATIC_DRAW);
				glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, 0, 0);

				glBindBuffer(GL_ARRAY_BUFFER, this->vboHandle_normal);
				glBufferData(GL_ARRAY_BUFFER, this->vbo_n.size() * sizeof(GLfloat), &this->vbo_n.at(0), GL_STATIC_DRAW);
				glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);

				glEnableVertexAttribArray(0);
				glEnableVertexAttribArray(4);
				glEnableVertexAttribArray(2);

				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->iboHandle);
				glBufferData(GL_ELEMENT_ARRAY_BUFFER, this->ibo.size() * sizeof(unsigned int), &this->ibo.at(0), GL_STATIC_DRAW);

				// done
				glBindVertexArray(0);
			}

			void Foliages::initSpatialSamples() {
				this->grass_sample = SpatialSample::importBinaryFile("models\\spatialSamples\\poissonPoints_155304s.ss2");
				this->bush01_sample = SpatialSample::importBinaryFile("models\\spatialSamples\\poissonPoints_1010s.ss2");
				this->bush05_sample = SpatialSample::importBinaryFile("models\\spatialSamples\\poissonPoints_2797s.ss2");
				this->grass_num_sample = grass_sample->numSample();
				this->bush01_num_sample = bush01_sample->numSample();
				this->bush05_num_sample = bush05_sample->numSample();
				this->total_num_sample = this->grass_num_sample + this->bush01_num_sample + this->bush05_num_sample;

				cout << "bush01's number of samples: " << this->bush01_num_sample << " \n";
				cout << "bush05's number of samples: " << this->bush05_num_sample << "\n";
				cout << "grass's number of samples: " << this->grass_num_sample << "\n";
				cout << "total number of samples: " << this->total_num_sample << "\n";
			}

			void Foliages::initSSBO() {
				this->rawInstanceData = new RawInstanceProperties[this->total_num_sample];
				int i = 0;

				for (int idx = 0; idx < this->grass_sample->numSample(); idx++, i++) {
					const float* positionBuffer = this->grass_sample->position(idx);
					this->rawInstanceData[i].position = glm::vec4(positionBuffer[0], positionBuffer[1], positionBuffer[2], 0.0);
					this->rawInstanceData[i].passed = glm::ivec4(0, 0, 0, 0);
				}

				for (int idx = 0; idx < this->bush01_sample->numSample(); idx++, i++) {
					const float* positionBuffer = this->bush01_sample->position(idx);
					this->rawInstanceData[i].position = glm::vec4(positionBuffer[0], positionBuffer[1], positionBuffer[2], 0.0);
					this->rawInstanceData[i].passed = glm::ivec4(0, 0, 0, 0);
				}

				for (int idx = 0; idx < this->bush05_sample->numSample(); idx++, i++) {
					const float* positionBuffer = this->bush05_sample->position(idx);
					this->rawInstanceData[i].position = glm::vec4(positionBuffer[0], positionBuffer[1], positionBuffer[2], 0.0);
					this->rawInstanceData[i].passed = glm::ivec4(0, 0, 0, 0);
				}

				// SSBO for the raw instance data
				glGenBuffers(1, &this->rawInstanceDataBufferHandle);
				glBindBuffer(GL_SHADER_STORAGE_BUFFER, this->rawInstanceDataBufferHandle);
				glBufferStorage(GL_SHADER_STORAGE_BUFFER, this->total_num_sample * sizeof(RawInstanceProperties), this->rawInstanceData, GL_MAP_READ_BIT);
				glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, this->rawInstanceDataBufferHandle);

				// SSBO for the valid instance data
				glGenBuffers(1, &this->validInstanceDataBufferHandle);
				glBindBuffer(GL_SHADER_STORAGE_BUFFER, this->validInstanceDataBufferHandle);
				glBufferStorage(GL_SHADER_STORAGE_BUFFER, this->total_num_sample * sizeof(InstanceProperties), nullptr, GL_MAP_READ_BIT);
				glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, this->validInstanceDataBufferHandle);

				// SSBO as vertex shader attribute
				glBindVertexArray(this->vaoHandle);
				glBindBuffer(GL_ARRAY_BUFFER, this->validInstanceDataBufferHandle);
				glVertexAttribPointer(3, 4, GL_FLOAT, false, 0, nullptr);
				glEnableVertexAttribArray(3);
				glVertexAttribDivisor(3, 1);

				// done
				glBindVertexArray(0);
			}

			void Foliages::initDrawCommands() {
				this->drawCommandsArray.resize(3);

				// grass
				this->drawCommandsArray[0].count = this->grass_model->drawCounts;
				this->drawCommandsArray[0].instanceCount = 0;
				this->drawCommandsArray[0].firstIndex = 0;
				this->drawCommandsArray[0].baseVertex = 0;
				this->drawCommandsArray[0].baseInstance = 0;

				// bush01
				this->drawCommandsArray[1].count = this->bush01_model->drawCounts;
				this->drawCommandsArray[1].instanceCount = 0;
				this->drawCommandsArray[1].firstIndex = this->offset_1;
				this->drawCommandsArray[1].baseVertex = this->offset_1;
				this->drawCommandsArray[1].baseInstance = this->grass_sample->numSample();

				// bush05
				this->drawCommandsArray[2].count = this->bush05_model->drawCounts;
				this->drawCommandsArray[2].instanceCount = 0;
				this->drawCommandsArray[2].firstIndex = this->offset_2;
				this->drawCommandsArray[2].baseVertex = this->offset_2;
				this->drawCommandsArray[2].baseInstance = this->grass_sample->numSample() + this->bush01_sample->numSample();

				this->allCommands.insert(this->allCommands.end(), this->drawCommandsArray.begin(), this->drawCommandsArray.end());

				glGenBuffers(1, &this->cmdBufferHandle);
				glBindBuffer(GL_SHADER_STORAGE_BUFFER, this->cmdBufferHandle);
				glBufferStorage(GL_SHADER_STORAGE_BUFFER, sizeof(DrawElementsIndirectCommand) * allCommands.size(), allCommands.data(), GL_MAP_READ_BIT);
				glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, this->cmdBufferHandle);
			};

			Modelc* Foliages::loadModels(const char* path, int textureIndex) {
				Modelc* model = new Modelc();
				const aiScene* scene;
				scene = importerFoliages.ReadFile(path, aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_CalcTangentSpace | aiProcess_FlipUVs);
				if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
					cout << "foliage model " << path << " fail to load\n" << importerFoliages.GetErrorString() << "\n";
				}
				else {
					cout << "foliage model " << path << " load successfully\n";
				}
				aiMesh* mesh = scene->mMeshes[0];
				for (unsigned int v = 0; v < mesh->mNumVertices; ++v)
				{

					GLfloat x = mesh->mVertices[v][0];
					GLfloat y = mesh->mVertices[v][1];
					GLfloat z = mesh->mVertices[v][2];

					GLfloat nx = mesh->mNormals[v][0];
					GLfloat ny = mesh->mNormals[v][1];
					GLfloat nz = mesh->mNormals[v][2];

					GLfloat u0 = mesh->mTextureCoords[0][v][0];
					GLfloat u1 = mesh->mTextureCoords[0][v][1];

					this->vbo_p.push_back(x);
					this->vbo_p.push_back(y);
					this->vbo_p.push_back(z);

					this->vbo_n.push_back(nx);
					this->vbo_n.push_back(ny);
					this->vbo_n.push_back(nz);

					this->vbo_t.push_back(u0);
					this->vbo_t.push_back(u1);
					this->vbo_t.push_back(textureIndex);
				}

				for (unsigned int f = 0; f < mesh->mNumFaces; ++f) {
					unsigned int index1 = mesh->mFaces[f].mIndices[0];
					unsigned int index2 = mesh->mFaces[f].mIndices[1];
					unsigned int index3 = mesh->mFaces[f].mIndices[2];

					this->ibo.push_back(index1);
					this->ibo.push_back(index2);
					this->ibo.push_back(index3);
				}

				model->drawCounts = mesh->mNumFaces * 3;

				return model;
			}

			void Foliages::resetRenderParameters() {
				glDispatchCompute(1, 1, 1);
				glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
			}

			void Foliages::collectVisibleInstances(const glm::mat4& viewProjMat) {
				glUniformMatrix4fv(SHADER_PARAMETER_BINDING::VIEW_MAT_LOCATION, 1, false, glm::value_ptr(viewProjMat));
				glUniform1i(SHADER_PARAMETER_BINDING::GRASS_NUM_INSTANCE_LOCATION, this->grass_sample->numSample());
				glUniform1i(SHADER_PARAMETER_BINDING::BUSH01_NUM_INSTANCE_LOCATION, this->bush01_sample->numSample());
				glUniform1i(SHADER_PARAMETER_BINDING::BUSH05_NUM_INSTANCE_LOCATION, this->bush05_sample->numSample());
				glUniform1i(SHADER_PARAMETER_BINDING::TOTAL_NUM_INSTANCE_LOCATION, this->total_num_sample);
				glDispatchCompute((this->total_num_sample / 1024) + 1, 1, 1);
				glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
			}

			void Foliages::render() {
				glBindVertexArray(this->vaoHandle);
				glBindTexture(GL_TEXTURE_2D_ARRAY, this->textureArrayHandle);
				glBindTexture(GL_TEXTURE_2D, 0);
				glBindBuffer(GL_DRAW_INDIRECT_BUFFER, this->cmdBufferHandle);
				glUniformMatrix4fv(SHADER_PARAMETER_BINDING::MODEL_MAT_LOCATION, 1, false, glm::value_ptr(glm::translate(glm::vec3(0, 0, 0))));
				glMultiDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, nullptr, this->drawCommandsArray.size(), 0);
			}

			void Foliages::update(const Camera* camera) {
				// nothing
			}
		}
	}
}
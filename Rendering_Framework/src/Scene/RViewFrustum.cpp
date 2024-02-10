#include "RViewFrustum.h"

#include <glm/gtc/type_ptr.hpp>
#include <Rendering_Framework/src/Rendering/ShaderParameterBindingPoint.h>

namespace INANOA {
	namespace SCENE {
		RViewFrustum::RViewFrustum(const int numCascade, const Camera* camera): 
			m_numCascade(numCascade), m_numVertex((numCascade + 1) * 4), m_numIndex((numCascade * 4 + (numCascade + 1) * 4) * 2){
			this->init(camera);
		}
		RViewFrustum::~RViewFrustum(){}


		void RViewFrustum::init(const Camera* camera) {
			const int NUM_VERTEX = this->m_numVertex;
			const int NUM_CASCADE = this->m_numCascade;								
			const int NUM_INDEX = this->m_numIndex;
				
			// initialize the CONSTANT index buffer
			unsigned int* indexBuffer = new unsigned int[NUM_INDEX];
			unsigned int indexBufferOffset = 0;
			for (unsigned int i = 0; i < NUM_CASCADE + 1; i++) {
				const unsigned int currLayerStartIdx = i * 4;
				const unsigned int nextLayerStartIdx = (i + 1) * 4;

				// z-direction line
				if (i < NUM_CASCADE) {
					for (unsigned int j = 0; j < 4; j++) {
						indexBuffer[indexBufferOffset + 0] = nextLayerStartIdx + j;
						indexBuffer[indexBufferOffset + 1] = currLayerStartIdx + j;
						indexBufferOffset = indexBufferOffset + 2;
					}
				}

				// x-direction line
				for (unsigned int j = 0; j < 3; j++) {
					indexBuffer[indexBufferOffset + 0] = currLayerStartIdx + j;
					indexBuffer[indexBufferOffset + 1] = currLayerStartIdx + j + 1;
					indexBufferOffset = indexBufferOffset + 2;
				}
				indexBuffer[indexBufferOffset + 0] = currLayerStartIdx + 3;
				indexBuffer[indexBufferOffset + 1] = currLayerStartIdx + 0;
				indexBufferOffset = indexBufferOffset + 2;
			}
				
			// create GL_ELEMENT_ARRAY_BUFFER
			GLuint iboHandle = 0u;
			glGenBuffers(1, &iboHandle);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iboHandle);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * NUM_INDEX, indexBuffer, GL_STATIC_DRAW);

			delete[] indexBuffer;

			// create a DYNAMIC VBO (only vertex is used)
			this->m_vertexBuffer = new float[NUM_VERTEX * 3]{ 0.0f };
			GLuint vboHandle = 0u;
			glGenBuffers(1, &vboHandle);
			glBindBuffer(GL_ARRAY_BUFFER, vboHandle);
			glBufferData(GL_ARRAY_BUFFER, sizeof(float) * NUM_VERTEX * 3, this->m_vertexBuffer, GL_DYNAMIC_DRAW);
			this->m_vertexBufferHandle = vboHandle;

			// create VAO
			GLuint vaoHandle = 0u;
			glGenVertexArrays(1, &vaoHandle);
			glBindVertexArray(vaoHandle);
			glEnableVertexAttribArray(SHADER_PARAMETER_BINDING::VERTEX_LOCATION);
			glBindBuffer(GL_ARRAY_BUFFER, vboHandle);
			glVertexAttribPointer(SHADER_PARAMETER_BINDING::VERTEX_LOCATION, 3, GL_FLOAT, false, 12, nullptr);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iboHandle);
			glBindVertexArray(0u);
			this->m_vaoHandle = vaoHandle;			
		}

		void RViewFrustum::render() {
			// bind vao
			glBindVertexArray(this->m_vaoHandle);
			// submit model matrix
			glUniformMatrix4fv(SHADER_PARAMETER_BINDING::MODEL_MAT_LOCATION, 1, false, glm::value_ptr(this->m_modelMat));
			// render
			const int indicesPtr = 0;
			glDrawElements(GL_LINES, this->m_numIndex, GL_UNSIGNED_INT, (GLvoid*)(indicesPtr));
		}


		void RViewFrustum::resize(const Camera* camera) {
			const float depths[] = { 
				camera->near(), camera->far() 
			};
			
			// collect cascade corners
			for (int i = 0; i < this->m_numCascade + 1; i++) {
				camera->viewFrustumClipPlaneCornersInViewSpace(depths[i], this->m_vertexBuffer + i * 12);
			}

			// update buffer
			glBindBuffer(GL_ARRAY_BUFFER, this->m_vertexBufferHandle);
			glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float) * this->m_numVertex * 3, this->m_vertexBuffer);	
		}
	}
}

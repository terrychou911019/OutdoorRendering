#pragma once

#include <glad/glad.h>
#include <Rendering_Framework/src/Rendering/Camera/Camera.h>
#include <Rendering_Framework/src/Scene/Trajectory.h>

namespace INANOA {
	namespace SCENE {
		namespace EXPERIMENTAL {
			class Slime
			{
			public:
				explicit Slime(const int numCascade, const Camera* camera);
				virtual ~Slime();

				Slime(const Slime&) = delete;
				Slime(const Slime&&) = delete;
				Slime& operator=(const Slime&) = delete;

			public:
				void init(const Camera* camera);
				void update(const Camera* camera);
				void render();

			private:
				glm::mat4 m_modelMat;
				Trajectory* m_trajectory;
				GLuint m_vaoHandle = 0u;
				GLuint m_vertexBufferHandle_vertex = 0u;
				GLuint m_vertexBufferHandle_texcoord = 0u;
				GLuint m_vertexBufferHandle_normal = 0u;
				GLuint m_iboHandle = 0u;
			};
		}
	}
}




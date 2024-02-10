#pragma once

#include <glad/glad.h>

namespace INANOA {
	namespace SHADER_PARAMETER_BINDING {
		const GLuint VERTEX_LOCATION = 0;

		const GLint MODEL_MAT_LOCATION = 0;
		const GLint VIEW_MAT_LOCATION = 1;
		const GLint PROJ_MAT_LOCATION = 2;
		const GLint SHADING_MODEL_ID_LOCATION = 5;

		const GLuint GRASS_NUM_INSTANCE_LOCATION = 6;
		const GLuint BUSH01_NUM_INSTANCE_LOCATION = 7;
		const GLuint BUSH05_NUM_INSTANCE_LOCATION = 8;
		const GLuint TOTAL_NUM_INSTANCE_LOCATION = 9;

		const GLuint SLIME_X = 10;
		const GLuint SLIME_Y = 11;
		const GLuint SLIME_Z = 12;
	}
}

#pragma once

#include <vector>

#include <Rendering_Framework/src/Rendering/RendererBase.h>
#include <Rendering_Framework/src/Scene/RViewFrustum.h>
#include <Rendering_Framework/src/Scene/RHorizonGround.h>
#include <Rendering_Framework/src/Scene/RSlime.h>
#include <Rendering_Framework/src/Scene/RFoliages.h>

namespace INANOA {
	class RenderingOrderExp
	{
	public:
		RenderingOrderExp();
		virtual ~RenderingOrderExp();

	public:
		bool init(const int w, const int h) ;
		void resize(const int w, const int h) ;
		void update(int dir, float rad, float yaw, float pitch);
		void render();
	private:
		SCENE::RViewFrustum* m_viewFrustum = nullptr;
		SCENE::EXPERIMENTAL::HorizonGround* m_horizontalGround = nullptr;
		SCENE::EXPERIMENTAL::Slime* m_slime = nullptr;
		SCENE::EXPERIMENTAL::Foliages* m_foliages = nullptr;

		Camera* m_playerCamera = nullptr;
		Camera* m_godCamera = nullptr;

		glm::vec3 m_cameraForwardMagnitude;
		float m_cameraForwardSpeed;

		int m_frameWidth;
		int m_frameHeight;

		string slime_path;
		string grass_path;
		string bush_path1;
		string bush_path2;

		OPENGL::RendererBase* m_renderer = nullptr;
		OPENGL::RendererBase* m_computeShader = nullptr;
		OPENGL::RendererBase* m_resetShader = nullptr;
	};

}



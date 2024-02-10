#include "RenderingOrderExp.h"
#include <iostream>

using namespace std;

namespace INANOA {	

	// ===========================================================
	RenderingOrderExp::RenderingOrderExp(){
		this->m_cameraForwardSpeed = 0.15f;
		this->m_cameraForwardMagnitude = glm::vec3(0.0f, 0.0f, 0.0f);
		this->m_frameWidth = 64;
		this->m_frameHeight = 64;
	}
	RenderingOrderExp::~RenderingOrderExp(){}

	bool RenderingOrderExp::init(const int w, const int h) {
		INANOA::OPENGL::RendererBase* renderer = new INANOA::OPENGL::RendererBase();
		const std::string vsFile = "src\\shader\\vertexShader_ogl_450.glsl";
		const std::string fsFile = "src\\shader\\fragmentShader_ogl_450.glsl";
		if (renderer->init(vsFile, fsFile, w, h) == false) {
			return false;
		}

		INANOA::OPENGL::RendererBase* computeShader = new INANOA::OPENGL::RendererBase();
		const std::string csFile = "src\\shader\\computeShader.glsl";
		if (computeShader->init(csFile) == false) {
			return false;
		}

		INANOA::OPENGL::RendererBase* resetShader = new INANOA::OPENGL::RendererBase();
		const std::string rsFile = "src\\shader\\resetShader.glsl";
		if (resetShader->init(rsFile) == false) {
			return false;
		}

		this->m_renderer = renderer;
		this->m_computeShader = computeShader;
		this->m_resetShader = resetShader;

		this->m_godCamera = new Camera(glm::vec3(0.0f, 0.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), 5.0f, 60.0f, 0.1f, 512.0f);
		this->m_godCamera->resize(w, h);

		this->m_godCamera->setViewOrg(glm::vec3(0.0f, 55.0f, 50.0f));
		this->m_godCamera->setLookCenter(glm::vec3(0.0f, 32.0f, -12.0f));
		this->m_godCamera->setDistance(70.0f);
		this->m_godCamera->update();

		this->m_playerCamera = new Camera(glm::vec3(0.0f, 10.0f, 0.0f), glm::vec3(0.0f, 9.5f, -5.0f), glm::vec3(0.0f, 1.0f, 0.0f), 10.0, 45.0f, 1.0f, 150.0f);
		this->m_playerCamera->resize(w, h);
		this->m_playerCamera->update();

		m_renderer->setCamera(
			this->m_godCamera->projMatrix(),
			this->m_godCamera->viewMatrix(),
			this->m_godCamera->viewOrig()
		);

		// view frustum and horizontal ground
		{
			this->m_viewFrustum = new SCENE::RViewFrustum(1, nullptr);
			this->m_viewFrustum->resize(this->m_playerCamera);

			this->m_horizontalGround = new SCENE::EXPERIMENTAL::HorizonGround(2, nullptr);
			this->m_horizontalGround->resize(this->m_playerCamera);
		}

		// foliages
		this->m_foliages = new SCENE::EXPERIMENTAL::Foliages(1, nullptr);
		cout << "\n";

		// slime
		this->m_slime = new SCENE::EXPERIMENTAL::Slime(1, nullptr);

		this->resize(w, h);		
		return true;
	}
	void RenderingOrderExp::resize(const int w, const int h) {
		const int HW = w * 0.5;

		this->m_playerCamera->resize(HW, h);
		this->m_godCamera->resize(HW, h);
		this->m_renderer->resize(w, h);
		this->m_frameWidth = w;
		this->m_frameHeight = h;

		this->m_viewFrustum->resize(this->m_playerCamera);
		this->m_horizontalGround->resize(this->m_playerCamera);
	}
	void RenderingOrderExp::update(int dir, float rad, float yaw, float pitch) {
		// camera update (god)
		this->m_godCamera->considerYawAndPitch(yaw, pitch);
		this->m_godCamera->update();

		// camera update (player)
		if (dir == 1) {
			this->m_cameraForwardMagnitude = glm::vec3(0.0f, 0.7f, 0.0f);
		}
		else if (dir == -1) {
			this->m_cameraForwardMagnitude = glm::vec3(0.0f, -0.7f, 0.0f);
		}
		else {
			this->m_cameraForwardMagnitude = glm::vec3(0.0f, 0.0f, 0.0f);
		}
		this->m_playerCamera->forward(this->m_cameraForwardMagnitude, true);
		this->m_playerCamera->rotateLookCenterAccordingToViewOrg(rad * 3);
		this->m_playerCamera->update();

		// lock to view space
		this->m_viewFrustum->update(this->m_playerCamera);
		this->m_horizontalGround->update(this->m_playerCamera);

		// foliages
		this->m_foliages->update(this->m_playerCamera);

		//slime
		this->m_computeShader->useShader();
		this->m_slime->update(this->m_playerCamera);
	}

	void RenderingOrderExp::render() {		
		this->m_renderer->clearRenderTarget();
		const int HW = this->m_frameWidth * 0.5;

		// reset shader
		this->m_resetShader->useShader();
		this->m_foliages->resetRenderParameters();

		// compute shader
		this->m_computeShader->useShader();
		this->m_foliages->collectVisibleInstances(m_playerCamera->projMatrix() * m_playerCamera->viewMatrix());

		// =====================================================
		// god view
		this->m_renderer->useShader();

		this->m_renderer->setCamera(
			m_godCamera->projMatrix(),
			m_godCamera->viewMatrix(),
			m_godCamera->viewOrig()
		);

		this->m_renderer->setViewport(0, 0, HW, this->m_frameHeight);
		this->m_renderer->setShadingModel(OPENGL::ShadingModelType::UNLIT);
		this->m_viewFrustum->render();
		this->m_renderer->setShadingModel(OPENGL::ShadingModelType::PROCEDURAL_GRID);
		this->m_horizontalGround->render();
		this->m_renderer->setShadingModel(OPENGL::ShadingModelType::FOLIAGES);
		this->m_foliages->render();
		this->m_renderer->setShadingModel(OPENGL::ShadingModelType::SLIME);
		this->m_slime->render();

		// =====================================================
		// player view
		this->m_renderer->clearDepth();
		this->m_renderer->setCamera(
			this->m_playerCamera->projMatrix(),
			this->m_playerCamera->viewMatrix(),
			this->m_playerCamera->viewOrig()
		);

		this->m_renderer->setViewport(HW, 0, HW, this->m_frameHeight);
		this->m_renderer->setShadingModel(OPENGL::ShadingModelType::PROCEDURAL_GRID);
		this->m_horizontalGround->render();		
		this->m_renderer->setShadingModel(OPENGL::ShadingModelType::FOLIAGES);
		this->m_foliages->render();
		this->m_renderer->setShadingModel(OPENGL::ShadingModelType::SLIME);
		this->m_slime->render();

	}
}

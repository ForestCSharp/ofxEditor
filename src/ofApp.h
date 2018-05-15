#pragma once

#include "ofMain.h"
#include "ofxImGui.h"
#include "ThemeTest.h"

#include <map>

#include "ofxPBR.h"
#include "ofxAssimpModelLoader.h"
#include "ofxEditorCam.h"
#include "ofxBullet.h"

#include "PostProcessEffect.hpp"


#include "PropertySystem.hpp"

class ofApp : public ofBaseApp {
public:

	static ofApp* AppPtr;
	inline static ofApp* const GetAppPtr() { return AppPtr; }

	void setup();
	void update();
	void draw();
	void renderScene();

	void keyPressed(int key);
	void keyReleased(int key);
	void mouseMoved(int x, int y);
	void mouseDragged(int x, int y, int button);
	void mousePressed(int x, int y, int button);
	void mouseReleased(int x, int y, int button);
	void windowResized(int w, int h);
	void dragEvent(ofDragInfo dragInfo);
	void gotMessage(ofMessage msg);
	void mouseScrolled(float x, float y);

	//ImGUI
	ofxImGui::Gui imgui;

	ofImage imageButtonSource;
	GLuint imageButtonID;

	ofPixels pixelsButtonSource;
	GLuint pixelsButtonID;

	ofTexture textureSource;
	GLuint textureSourceID;

	bool bShowPieMenu = false;

	//PBR
	function<void()> scene;

	ofxPBRCubeMap cubeMap;
	ofxPBRMaterial material;
	ofxPBRLight light;
	ofxPBR pbr;

	//Post Processing
	ofFbo ScenePassFBO;
	ofFbo PostProcessFBO;
	ofShader PostProcessShader;

	ofxPostProcessEffect PostProcessEffect;

	//Physics
	ofxBulletWorldRigid BulletWorld;
	ofxBulletBox		BulletGround;
	ofxBulletBox		BulletBox;

	//Camera
	ofxEditorCam EditorCam;

	//Editor Gizmo
	ImGuizmo::OPERATION mCurrentGizmoOperation = ImGuizmo::TRANSLATE;
	ImGuizmo::MODE mCurrentGizmoMode = ImGuizmo::WORLD;

	float GizmoTestMatrix[16] =
	{ 1.f, 0.f, 0.f, 0.f,
		0.f, 1.f, 0.f, 0.f,
		0.f, 0.f, 1.f, 0.f,
		0.f, 0.f, 0.f, 1.f };

	//PBR/ImGUI Variables
	ofFloatColor BaseColor;
	float Metallic = 0.0f;
	float Roughness = 0.0f;

	//Airship
	ofxAssimpModelLoader Airship;

	std::map<int, bool> KeyPressMap;
	inline const bool IsKeyPressed(const int Key) { return KeyPressMap[Key]; }
};

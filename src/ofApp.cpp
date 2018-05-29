#include "ofApp.h"

#include "ImGuiRadialMenu.hpp"

ofApp* ofApp::AppPtr = nullptr;

//--------------------------------------------------------------
void ofApp::setup()
{
	AppPtr = this;

	ofSetLogLevel(OF_LOG_VERBOSE);

	//required call
	imgui.setup();

	ImGui::GetIO().MouseDrawCursor = false;

	//load your own ofImage
	imageButtonSource.load("test.png");
	imageButtonID = imgui.loadImage(imageButtonSource);

	//or have the loading done for you if you don't need the ofImage reference
	//imageButtonID = gui.loadImage("of.png");

	//can also use ofPixels in same manner
	ofLoadImage(pixelsButtonSource, "of_upside_down.png");
	pixelsButtonID = imgui.loadPixels(pixelsButtonSource);

	//and alt method
	//pixelsButtonID = gui.loadPixels("of_upside_down.png");

	//pass in your own texture reference if you want to keep it
	textureSourceID = imgui.loadTexture(textureSource, "of_upside_down.png");

	//or just pass a path
	//textureSourceID = gui.loadTexture("of_upside_down.png");

	ofLogVerbose() << "textureSourceID: " << textureSourceID;

	//PBR Setup
	ofDisableArbTex();

	EditorCam.setupPerspective(false, 60, 1, 120000);

	cubeMap.load("Barce_Rooftop_C_3k.jpg", 1024, true, "filteredMapCache");
	pbr.setup(2048);
	pbr.setCubeMap(&cubeMap);

	scene = bind(&ofApp::renderScene, this);

	//TODO: Pretty sure point light shadows aren't omnidirectional at the moment
	light.setLightType(LightType_Directional);
	light.setPosition(-1500, 1000, 1500);
	light.lookAt(ofVec3f(0));
	light.setScale(5.5);
	
	light.setColor(ofFloatColor(1.0));
	light.setShadowType(ShadowType_Soft);
	pbr.addLight(&light);

	cubeMap.setEnvLevel(0.3);

	Airship.loadModel("LunaMoth.obj");
	Airship.disableMaterials();

	EditorCam.setTarget(ofVec3f(0, 0, 0));

	//ofxBullet Testing 
	BulletWorld.setup();
	BulletWorld.enableGrabbing();
	BulletWorld.enableDebugDraw();
	BulletWorld.setCamera(&EditorCam);
	
	BulletBox.create(BulletWorld.world, ofVec3f(), .05, 100., 100., 100.);
	BulletBox.add();

	BulletGround.create(BulletWorld.world, ofVec3f(0., -2500., 0.), 0., 10000.f, 500.f, 10000.f);
	BulletGround.setProperties(.25, .95);
	BulletGround.add();

	string fragShaderSrc = STRINGIFY(
		uniform sampler2D InputTexture;
		uniform vec2 resolution;

		out vec4 outputColor;

		void main()
		{
			outputColor = texture(InputTexture, gl_FragCoord.xy * resolution) + vec4(1.0, 0.0, 0.0, 1.0);
		}
	);

	string fxaaShaderSrc = STRINGIFY(
		uniform sampler2D InputTexture;
		uniform vec2 resolution;

		const float FXAA_REDUCE_MIN = 1.0 / 128.0;
		const float FXAA_REDUCE_MUL = 1.0 / 8.0;
		const float FXAA_SPAN_MAX = 8.0;

		out vec4 outputColor;

		void main() {

			vec3 rgbNW = texture(InputTexture, (gl_FragCoord.xy + vec2(-1.0, -1.0)) * resolution).xyz;
			vec3 rgbNE = texture(InputTexture, (gl_FragCoord.xy + vec2(1.0, -1.0)) * resolution).xyz;
			vec3 rgbSW = texture(InputTexture, (gl_FragCoord.xy + vec2(-1.0, 1.0)) * resolution).xyz;
			vec3 rgbSE = texture(InputTexture, (gl_FragCoord.xy + vec2(1.0, 1.0)) * resolution).xyz;
			vec4 rgbaM = texture(InputTexture, gl_FragCoord.xy  * resolution);
			vec3 rgbM = rgbaM.xyz;
			float opacity = rgbaM.w;

			vec3 luma = vec3(0.299, 0.587, 0.114);

			float lumaNW = dot(rgbNW, luma);
			float lumaNE = dot(rgbNE, luma);
			float lumaSW = dot(rgbSW, luma);
			float lumaSE = dot(rgbSE, luma);
			float lumaM = dot(rgbM, luma);
			float lumaMin = min(lumaM, min(min(lumaNW, lumaNE), min(lumaSW, lumaSE)));
			float lumaMax = max(lumaM, max(max(lumaNW, lumaNE), max(lumaSW, lumaSE)));

			vec2 dir;
			dir.x = -((lumaNW + lumaNE) - (lumaSW + lumaSE));
			dir.y = ((lumaNW + lumaSW) - (lumaNE + lumaSE));

			float dirReduce = max((lumaNW + lumaNE + lumaSW + lumaSE) * (0.25 * FXAA_REDUCE_MUL), FXAA_REDUCE_MIN);

			float rcpDirMin = 1.0 / (min(abs(dir.x), abs(dir.y)) + dirReduce);
			dir = min(vec2(FXAA_SPAN_MAX, FXAA_SPAN_MAX),
				max(vec2(-FXAA_SPAN_MAX, -FXAA_SPAN_MAX),
					dir * rcpDirMin)) * resolution;

			vec3 rgbA = 0.5 * (
				texture(InputTexture, gl_FragCoord.xy  * resolution + dir * (1.0 / 3.0 - 0.5)).xyz +
				texture(InputTexture, gl_FragCoord.xy  * resolution + dir * (2.0 / 3.0 - 0.5)).xyz);

			vec3 rgbB = rgbA * 0.5 + 0.25 * (
				texture(InputTexture, gl_FragCoord.xy  * resolution + dir * -0.5).xyz +
				texture(InputTexture, gl_FragCoord.xy  * resolution + dir * 0.5).xyz);

			float lumaB = dot(rgbB, luma);

			if ((lumaB < lumaMin) || (lumaB > lumaMax)) {

				outputColor = vec4(rgbA, opacity);

			}
			else {

				outputColor = vec4(rgbB, opacity);

			}

		}
		);


	ostringstream oss;

	oss << "#version 150" << endl;
	oss << fxaaShaderSrc;

	//Disable this on App Startup so FBOs work
	ofDisableArbTex();
	ScenePassFBO.allocate(ofGetWidth(), ofGetHeight(), GL_RGBA);
	PostProcessEffect.SetSize(ofGetWidth(), ofGetHeight());
	PostProcessEffect.SetFragmentShader(oss.str());
}

//--------------------------------------------------------------
void ofApp::update() {
	BulletBox.applyCentralForce(ofVec3f(0, -30, 0));
	BulletBox.applyTorque(ofVec3f(0, 0, -100.0f));
	BulletWorld.update();
}

float airships_x = 10;
float airships_z = 10;
bool show_test_window = false;
bool enable_post_process = true;

//--------------------------------------------------------------
void ofApp::draw() {

	//Render Scene into FBO
	ScenePassFBO.bind();

	ofClear(255, 255, 255);

	//PBR
	pbr.makeDepthMap(scene);
	EditorCam.begin();
	pbr.drawEnvironment(&EditorCam);

	scene();

	//Physics Debug
	//FIXME: Broken
	//ofSetLineWidth(1.f);
	//ofSetColor(255, 0, 200);
	//BulletWorld.drawDebug();

	EditorCam.end();

	ScenePassFBO.unbind();

	//Can chain post process effects here
	auto FXAA_Result = PostProcessEffect.Process(ScenePassFBO);

	if (enable_post_process)
	{
		PostProcessEffect.Draw();
	}
	else
	{
		ScenePassFBO.getTextureReference().getTextureData().bFlipTexture = true;
		ScenePassFBO.draw(0, 0);
	}

	//required to call this at beginning
	imgui.begin();

	ImGuizmo::BeginFrame();
	ImGuiIO& io = ImGui::GetIO();
	ImGuizmo::SetRect(0, 0, io.DisplaySize.x, io.DisplaySize.y);
	ImGuizmo::Manipulate(EditorCam.getModelViewMatrix().getPtr(), EditorCam.getProjectionMatrix().getPtr(), mCurrentGizmoOperation, mCurrentGizmoMode, GizmoTestMatrix, NULL, NULL /* Snapping */);

	//In between gui.begin() and gui.end() you can use ImGui as you would anywhere else

	{

		AggregateProperty AggProp(std::string("AggregateProperty"));

		AggProp.AddProperty(std::string("Enable Post-Processing"), &enable_post_process);
		AggProp.AddProperty(std::string("Color"), &BaseColor);
		AggProp.AddProperty(std::string("Metallic"), &Metallic);
		AggProp.AddProperty(std::string("Roughness"), &Roughness);
		AggProp.AddProperty(std::string("AirShipsX"), &airships_x);
		AggProp.AddProperty(std::string("AirShipsZ"), &airships_z);

		//TODO: Prevent infinite recursion?
		AggregateProperty InnerAggregate(std::string("InnerAggregate"));
		AggProp.AddProperty(std::string("InnerAggregate"), &InnerAggregate);
		static float InnerFloat = 2.0f;
		InnerAggregate.AddProperty(std::string("InnerFloat"), &InnerFloat);

		//This actually works, as they start collapsed by default!
		AggProp.AddProperty(std::string("CyclicalReference"), &AggProp);

		PropWindow.Selected = &AggProp;
		PropWindow.Render();
		
		//bool pressed = ImGui::ImageButton((ImTextureID)(uintptr_t)imageButtonID, ImVec2(200, 200));

		//Testing Pie Menu
		if (bShowPieMenu)
		{
			ImGui::OpenPopup("PieMenu");
		}

		//TODO: Pass Keycode instead of mousebutton into pie

		if (BeginPiePopup("PieMenu", 1))
		{
			if (PieMenuItem("Test1")) { /*TODO*/ }
			if (PieMenuItem("Test2")) { /*TODO*/ }

			if (PieMenuItem("Test3", false)) { /*TODO*/ }

			if (BeginPieMenu("Sub"))
			{
				if (BeginPieMenu("Sub sub\nmenu"))
				{
					if (PieMenuItem("SubSub")) { std::cout << "Sub Sub" << std::endl; }
					if (PieMenuItem("SubSub2")) { /*TODO*/ }
					EndPieMenu();
				}
				if (PieMenuItem("TestSub")) { /*TODO*/ }
				if (PieMenuItem("TestSub2")) { /*TODO*/ }
				EndPieMenu();
			}

			EndPiePopup();
		}
	}

	//required to call this at end
	imgui.end();

	if (textureSource.isAllocated())
	{
		textureSource.draw(ofRandom(200), ofRandom(200));
	}
}

void ofApp::renderScene() {
	ofEnableDepthTest();
	pbr.begin(&EditorCam);

	material.baseColor = BaseColor;
	material.roughness = Roughness;
	material.metallic = Metallic;

	material.begin(&pbr);

	ofPushMatrix();
	ofMultMatrix(GizmoTestMatrix);
	for (int i = 0; i < airships_x; ++i)
	{
		for (int j = 0; j < airships_z; ++j)
		{
			ofPushMatrix();
			ofTranslate(ofPoint(650.0f * j, 0.0f, 400.0f * i));
			Airship.drawFaces();
			ofPopMatrix();
		}
	}
	ofPopMatrix();

	//Testing ofxBullet Drawing
	ofPushMatrix();
	ofMultMatrix(BulletBox.getTransformationMatrix());
	ofVec3f BoxSize = BulletBox.getSize();
	ofDrawBox(ofVec3f(0,0,0), BoxSize.x, BoxSize.y, BoxSize.z);
	ofPopMatrix();

	ofPushMatrix();
	ofMultMatrix(BulletGround.getTransformationMatrix());
	ofVec3f GroundSize = BulletGround.getSize();
	ofDrawBox(ofVec3f(0, 0, 0), GroundSize.x, GroundSize.y, GroundSize.z);
	ofPopMatrix();

	material.end();

	pbr.end();

	ofDisableDepthTest();
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key) {

	if (key == 'f') { //TODO: Focus on selected object

		//Move the camera's position the same amount and same direction we move the target vec
		const ofVec3f oldTargetLoc = EditorCam.getTarget().getPosition();
		const ofVec3f newTargetLoc = BulletBox.getPosition();
		const ofVec3f deltaTargetLoc = newTargetLoc - oldTargetLoc;

		EditorCam.setPosition(EditorCam.getPosition() + deltaTargetLoc);
		EditorCam.setTarget(newTargetLoc);
	}
	else if (key == 'w') {
		mCurrentGizmoOperation = ImGuizmo::TRANSLATE;
	}
	else if (key == 'e') {
		mCurrentGizmoOperation = ImGuizmo::ROTATE;
	}
	else if (key == 'r') {
		mCurrentGizmoOperation = ImGuizmo::SCALE;
	}
	else if (key == ' ') {
		bShowPieMenu = true;
	}
	else if (key == '1') {
		PropWindow.bOpen = !PropWindow.bOpen;
	}
	else if (key == '=') {
		ofToggleFullscreen();
	}

	KeyPressMap[key] = true;
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key) 
{
	if (key == ' ') {
		bShowPieMenu = false;
	}

	KeyPressMap[key] = false;
}


void ofApp::mouseScrolled(float x, float y) {

}
//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y) {

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button) {

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button) {
	if (button == OF_MOUSE_BUTTON_RIGHT)
	{
		bShowPieMenu = true;
	}
}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button) {
	
	if (button == OF_MOUSE_BUTTON_RIGHT)
	{
		bShowPieMenu = false;
	}
}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h) {
	ScenePassFBO.allocate(w, h);
	PostProcessEffect.SetSize(w, h);
}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg) {

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo) {

}
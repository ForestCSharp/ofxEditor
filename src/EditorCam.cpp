#include "EditorCam.h"
#include "ofMath.h"
#include "ofUtils.h"
#include "ofApp.h"

// when an ofxEditorCam is moving due to momentum, this keeps it
// from moving forever by assuming small values are zero.
static const float minDifference = 0.1e-5f;

// this is the default on windows os
static const unsigned long doubleclickTime = 200;

//----------------------------------------
ofxEditorCam::ofxEditorCam() 
{
	reset();
}

//----------------------------------------
ofxEditorCam::~ofxEditorCam() {
	disableMouseInput();
}

//----------------------------------------
void ofxEditorCam::update(ofEventArgs & args) {
	
	viewport = getViewport(this->viewport);

	ofVec2f MousePosition;
	ofApp* AppPtr = ofApp::GetAppPtr();
	if (AppPtr != nullptr)
	{
		MousePosition.x = (float)AppPtr->mouseX / (float)viewport.width;
		MousePosition.y = (float)AppPtr->mouseY / (float)viewport.height;
	} 
	MouseDelta = MousePosition - PreviousMousePosition;
	//TODO: Move Mouse Delta to ofApp
	
	uint64_t Milliseconds = ofGetElapsedTimeMillis();
	float Delta = (float)Milliseconds / 1000.0f;

	if (bMouseInputEnabled) 
	{
		if (bDoRotate) 
		{
			updateRotation(Delta);
		}
		else if (bDoPan) 
		{
			UpdatePan(Delta);
		}
	}

	prevPosition = ofCamera::getGlobalPosition();
	prevRot      = ofCamera::getGlobalOrientation();


	PreviousMousePosition = MousePosition;
}

void ofxEditorCam::UpdatePan(const float& Delta)
{
	float SquareDistanceToTarget = getPosition().distance(target.getPosition());
	ofVec3f PanOffset = ofVec3f(getXAxis() * PanSensitivity * -MouseDelta.x) + ofVec3f(getYAxis() * PanSensitivity * MouseDelta.y);
	PanOffset *= SquareDistanceToTarget;
	setPosition(prevPosition + PanOffset * Delta);
}

//----------------------------------------
void ofxEditorCam::updateRotation(const float& Delta) {

	int vFlip;

	if (isVFlipped())
	{
		vFlip = -1;
	}
	else
	{
		vFlip = 1;
	}

	float xRot = vFlip * -MouseDelta.y * VerticalRotationRate * Delta;
	float yRot = -MouseDelta.x * HorizontalRotationRate * Delta;
	float zRot = 0;

	//Turntable Rotation Axis
	ofVec3f YRotationAxis = ofVec3f(0, 1, 0);

	curRot = ofQuaternion(xRot, getXAxis(), yRot, YRotationAxis, zRot, getZAxis());
	setPosition((prevPosition - target.getGlobalPosition())*curRot + target.getGlobalPosition());
	setOrientation(prevRot * curRot);

}

//----------------------------------------
void ofxEditorCam::begin(ofRectangle _viewport) {
	if (!bEventsSet) {
		setEvents(ofEvents());
	}
	viewport = getViewport(_viewport);
	ofCamera::begin(viewport);
}

//----------------------------------------
void ofxEditorCam::reset() {
	target.resetTransform();

	target.setPosition(0, 0, 0);
	lookAt(target);

	resetTransform();

	moveX = 0;
	moveY = 0;
	moveZ = 0;

	bDoPan = false;
	bDoRotate = false;
}

//----------------------------------------
void ofxEditorCam::setTarget(const ofVec3f& targetPoint) {
	target.setPosition(targetPoint);
	lookAt(target);
}

//----------------------------------------
void ofxEditorCam::setTarget(ofNode& targetNode) {
	target = targetNode;
	lookAt(target);
}

//----------------------------------------
ofNode& ofxEditorCam::getTarget() {
	return target;
}

//----------------------------------------
void ofxEditorCam::enableMouseInput() {
	if (!bMouseInputEnabled && events) 
	{
		ofAddListener(events->update, this, &ofxEditorCam::update);
		ofAddListener(events->mousePressed, this, &ofxEditorCam::mousePressed);
		ofAddListener(events->mouseReleased, this, &ofxEditorCam::mouseReleased);
		ofAddListener(events->mouseScrolled, this, &ofxEditorCam::mouseScrolled);
	}

	bMouseInputEnabled = true;
}

//----------------------------------------
void ofxEditorCam::disableMouseInput() {
	if (bMouseInputEnabled && events) 
	{
		ofRemoveListener(events->update, this, &ofxEditorCam::update);
		ofRemoveListener(events->mousePressed, this, &ofxEditorCam::mousePressed);
		ofRemoveListener(events->mouseReleased, this, &ofxEditorCam::mouseReleased);
		ofRemoveListener(events->mouseScrolled, this, &ofxEditorCam::mouseScrolled);
	}

	bMouseInputEnabled = false;
}

//----------------------------------------
void ofxEditorCam::setEvents(ofCoreEvents & _events) 
{
	bool wasMouseInputEnabled = bMouseInputEnabled;
	disableMouseInput();
	events = &_events;
	if (wasMouseInputEnabled) 
	{
		enableMouseInput();
	}
	bEventsSet = true;
}

//----------------------------------------
bool ofxEditorCam::getMouseInputEnabled() 
{
	return bMouseInputEnabled;
}

void ofxEditorCam::mousePressed(ofMouseEventArgs & mouse) {
	ofRectangle viewport = getViewport(this->viewport);
	
	ofApp* AppPtr = ofApp::GetAppPtr();
	assert(AppPtr != nullptr);

	if (viewport.inside(mouse.x, mouse.y)) 
	{
		prevAxisX = getXAxis();
		prevAxisY = getYAxis();
		prevAxisZ = getZAxis();

		if (mouse.button == OF_MOUSE_BUTTON_LEFT && AppPtr->IsKeyPressed(OF_KEY_ALT) && AppPtr->IsKeyPressed(OF_KEY_SHIFT))
		{
			bDoPan = true;
		}
		else if (mouse.button == OF_MOUSE_BUTTON_LEFT && AppPtr->IsKeyPressed(OF_KEY_ALT)) 
		{
			bDoRotate = true;
		}
		else
		{
			bDoRotate = false;
		}
	}
}

void ofxEditorCam::mouseReleased(ofMouseEventArgs & mouse) {
	ofRectangle viewport = getViewport(this->viewport);

	ofVec2f center(viewport.width / 2, viewport.height / 2);
	int vFlip;
	if (isVFlipped()) {
		vFlip = -1;
	}
	else {
		vFlip = 1;
	}

	if (mouse.button == OF_MOUSE_BUTTON_LEFT)
	{
		bDoRotate = false;
		bDoPan = false;
	}
}

void ofxEditorCam::mouseScrolled(ofMouseEventArgs & mouse) {
	ofRectangle viewport = getViewport(this->viewport);
	prevPosition = ofCamera::getGlobalPosition();
	prevAxisZ = getZAxis();
	
	float SquareDistanceToTarget = getPosition().distance(target.getPosition());
	setPosition(getPosition() + getZAxis() * -mouse.scrollY * ZoomSensitivity * SquareDistanceToTarget);
}

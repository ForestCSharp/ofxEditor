#pragma once

#include "ofCamera.h"
#include "ofEvents.h"

class ofxEditorCam : public ofCamera {
public:

	ofxEditorCam();
	~ofxEditorCam();

	virtual void begin(ofRectangle viewport = ofRectangle());

	void reset();

	void setTarget(const ofVec3f& target);
	void setTarget(ofNode& target);
	ofNode& getTarget();

	void setEvents(ofCoreEvents & events);

	void enableMouseInput();
	void disableMouseInput();
	bool getMouseInputEnabled();

	void enableMouseMiddleButton();
	void disableMouseMiddleButton();
	bool getMouseMiddleButtonEnabled();

	ofNode target;

	bool bDoPan = false;
	bool bDoRotate = false;

	bool bMouseInputEnabled = true;

	float moveX;
	float moveY;
	float moveZ;

	float VerticalRotationRate = 15.0f;
	float HorizontalRotationRate = 35.0f;
	float PanSensitivity = 0.05f;
	float ZoomSensitivity = 0.05f;

	ofVec2f PreviousMousePosition;

	//TODO: Move Mouse Delta to ofApp, note we normalized for viewport size
	ofVec2f MouseDelta;

	void update(ofEventArgs & args);
	void updateRotation(const float& Delta);
	void UpdatePan(const float& Delta);

	void mousePressed(ofMouseEventArgs & mouse);
	void mouseReleased(ofMouseEventArgs & mouse);
	void mouseScrolled(ofMouseEventArgs & mouse);

	ofQuaternion curRot;

	ofVec3f prevAxisX;

	ofVec3f prevAxisY;

	ofVec3f prevAxisZ;

	ofVec3f prevPosition;

	ofQuaternion prevRot;

	ofRectangle viewport;

	ofCoreEvents * events = nullptr;
	bool bEventsSet = false;
};
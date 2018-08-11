#pragma once

#include "ofCamera.h"
#include "ofEvents.h"
#include <glm/gtc/quaternion.hpp>

class ofxEditorCam : public ofCamera {
public:

	ofxEditorCam();
	~ofxEditorCam();

	virtual void begin(ofRectangle viewport = ofRectangle());

	void reset();

	void setTarget(const glm::vec3& target);
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

	float VerticalRotationRate = 7.0f;
	float HorizontalRotationRate = 17.0f;
	float PanSensitivity = 0.05f;
	float ZoomSensitivity = 0.05f;

	glm::vec2 PreviousMousePosition;

	//TODO: Move Mouse Delta to ofApp, note we normalized for viewport size
	glm::vec2 MouseDelta;

	void update(ofEventArgs & args);
	void updateRotation(const float& Delta);
	void UpdatePan(const float& Delta);

	void mousePressed(ofMouseEventArgs & mouse);
	void mouseReleased(ofMouseEventArgs & mouse);
	void mouseScrolled(ofMouseEventArgs & mouse);

	glm::quat curRot;

	glm::vec3 prevPosition;

	glm::quat prevRot;

	ofRectangle viewport;

	ofCoreEvents * events = nullptr;
	bool bEventsSet = false;
};
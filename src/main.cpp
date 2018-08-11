
#include "ofApp.h"
#include "ofMain.h"

#include <iostream>

//========================================================================
int main()
{

	ofGLWindowSettings settings;
	settings.setGLVersion( 4, 0 );
	settings.setSize( 1280, 720 );
	
	ofCreateWindow(settings);

	ofRunApp( new ofApp() );
}

#pragma once

#include "ofCamera.h"
#include "GameNode.hpp"

namespace ofxDeferredRenderer
{
	void Setup(int Width, int Height);
	void RenderScene(std::set<ofxGameMesh*> Meshes, std::set<ofxGameLight*> Lights, ofCamera& Camera);

}
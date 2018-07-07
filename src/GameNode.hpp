#pragma once

#include "ofNode.h"
#include "ofLight.h"
#include "ofTexture.h"

//Note: Had to Make "children" public in ofNode.h
class ofxGameNode : public ofNode
{
public:

	virtual void renderChildren()
	{
		for (ofNode* child : GetChildren())
		{
			child->draw();

			//TODO: Handle this dynamic_cast elsewhere (perhaps when adding nodes)
			ofxGameNode* gameChild = dynamic_cast<ofxGameNode*>(child);
			if (gameChild != nullptr)
			{
				//Need to render children outside of draw function so they don't get double matrix updates
				gameChild->renderChildren();
			}
		}
	}
}; //TODO: Inherit from this for all nodes, so they also render their children

class ofxGameMesh : public ofxGameNode
{
public:

	ofxGameMesh(ofMesh& InMesh)
	{
		Mesh = std::move(InMesh);
	}

	virtual void customDraw() override
	{
		Mesh.draw();
	}

	ofMesh Mesh;
	


	//TODO: Move to Material interface (Material owns its GBuffer Shader and binds it / sets up its uniforms)
	ofTexture AlbedoTexture;
	ofFloatColor AlbedoValue = ofFloatColor( .2, .2, .2, 1.0 );

	ofTexture MetallicTexture;
	float MetallicValue = 0.5f;

	ofTexture RoughnessTexture;
	float RoughnessValue = 0.2f;

	ofTexture NormalTexture;
	float NormalIntensity = 1.0f;
	
}; //TODO! Override Set Parent to update Meshes set

class ofxGameLight : public ofLight
{
	
}; //TODO! Override Set Parent to Update Lights set
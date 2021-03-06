//
//  basicShape.h
//  karmaMapper
//
//  Created by Daan de Lange on 22/3/14.
//  Defines common methods for shape handling
//

// todo: move load/save functions to basicShape rather than shapesHandler

#pragma once

#include "shapeFactory.hpp"
#include "ofMain.h"
#include "KMSettings.h"
#include "basicPoint.h"
#include "ofxXmlSettings.h"
#include "ofxGui.h"
#include "ofxGuiExtended.h"

// todo: move vertexShape stuff to it's own class and keep the bare minimum (points, etc)
// todo: rename colors (fgColor,bgColor) and give names like groupColor, primaryColor, secondaryColor, etc.
// todo: add name so we can getShapeByName()

// When a shape exits edit mode, all coordinates are made absolute so they are easy to use for effects. Relative coordinates are easier for manipulation and storage.
// Shape querying and altering happens trough absolute coordinates.
// todo: use a struct instead of ofVec2f

typedef list<basicPoint>& pointListRef; // without the typedef returning this causes compiler errors.

class basicShape {
	
public:
	basicShape(const basicPoint _pos);
	virtual ~basicShape();
	//const bool isShapeRegistered;
	
	// #########
	// MAIN FUNCTIONS
	void initialiseBasicVariables(const basicPoint _pos);
	virtual void sendToGPU();
	
	// #########
	// LOAD & SAVE FUNCTIONS
	virtual bool saveToXML(ofxXmlSettings& xml );
	virtual bool loadFromXML(ofxXmlSettings& xml);
	
	// #########
	// UTILITIES
	virtual void calculateBoundingBox();
	virtual void onShapeModified();
	virtual void onShapeEdited();
	virtual void resetToScene();
	
	// #########
	// BASIC SHAPE GETTERS
	virtual basicShape* getUpcasted();
	string getName() const;
	int getGroupID() const;
	bool isReady() const;
	bool hasFailed() const;
	bool isSelected = false;
	const string getShapeType() const;
	ofRectangle getBoundingBox() const;
	vector<string> getTypes() const;
	bool isType(const string _type) const;
	virtual bool isInside( const ofVec2f _pos, const bool _isPositionAbsolute = true) const;
	basicPoint* getPositionPtr();
	basicPoint* getPositionUnaltered();

	// #########
	// global variables
	static basicShape nullShape;
	
protected:
	
	// basicShape properties
	bool initialized = false;
	bool hasError = false;
	ofRectangle boundingBox; // contains all shapes
#ifdef KM_EDITOR_APP
	ofParameter<int> groupID; // [-1=none, other = groupID]
	ofParameter<string> shapeName;
#else
	int groupID;
	string shapeName;
#endif
	vector<string> myShapeTypes;
	
	basicPoint position; // absolute (other shape data will be relative to this)
	
	
private:

	
	// * * * * * * * * * * * * * * * * * * *
#ifdef KM_EDITOR_APP


public:
	// #########
	// EDITING ESSENTIALS
	virtual void render();
	
	// #########
	// EDITING CONTROL
	bool isInEditMode() const;
	bool switchEditMode();
	virtual bool enableEditMode();
	virtual bool disableEditMode();
	
	// #########
	// EDITING UI CONTROL
	virtual void selectPrevHandle();
	virtual void selectNextHandle();
	virtual bool handleExists(basicPoint* _i);
	virtual void selectHandle(basicPoint* _i);
	virtual void translateActiveHandle(basicPoint _offset);
	
	// #########
	// EDITING UTILITIES
	virtual void applyScale(basicPoint scale);
	
	// #########
	// SETTERS
	bool setPosition(const basicPoint _pos);
	bool setGroupID(const int& _id);
	
	// #########
	// GUI LISTENERS
	virtual void keyPressed(ofKeyEventArgs& e);
	virtual bool interceptMouseClick(ofMouseEventArgs &e);
	void groupIDUpdated(int& val);
	void nameUpdated(string& val);
	
	bool pleaseDeleteMe;
	
protected:
	bool bEditMode;
	
	ofColor fgColor;
	ofColor bgColor;
	static int maxGroupID; // used for color calculation
	int prevMaxGroupID; // used for color calculation
	
	basicPoint* activeHandle = NULL;
	
	// custom HUD GUI elements
	ofRectangle guiToggle;
	ofxPanelExtended shapeGui;
	ofxGuiGroupExtended basicShapeGui;
	bool drawShapeGui = false;
	
	void setColorFromGroupID();
	
private:
	void buildBasicMenu();
	
// endif KM_EDITOR_APP
#endif

};

// GUI translations
#define GUIinfo_GroupID		("Group ID")
#define GUIinfo_ShapeName	("Name")

namespace shape
{
	basicShape* create(const std::string& name, const basicPoint _pos);
	void destroy(const basicShape* comp);
	vector< std::string > getAllShapeTypes();
}

// allow shape registration
#define SHAPE_REGISTER(TYPE, NAME)                                        \
namespace shape {                                                         \
namespace factory {                                                       \
namespace                                                                 \
{                                                                         \
template<class T>                                                         \
class shapeRegistration;                                                  \
\
template<>                                                                \
class shapeRegistration<TYPE>                                             \
{                                                                         \
static const ::shape::factory::RegistryEntry<TYPE>& reg;                  \
};                                                                        \
\
const ::shape::factory::RegistryEntry<TYPE>&                              \
shapeRegistration<TYPE>::reg =                                            \
::shape::factory	::RegistryEntry<TYPE>::Instance(NAME);                \
}}}
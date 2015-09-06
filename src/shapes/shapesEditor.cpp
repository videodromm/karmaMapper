//
//  shapesEditor.cpp
//  karmaMapper
//
//  Created by Daan de Lange on 14/11/2014.
//  Copyright (c) 2014 __MyCompanyName__. All rights reserved.
//

#include "shapesEditor.h"


// - - - - - - - -
// CONSTRUCTORS
// - - - - - - - -
//shapesEditor::shapesEditor( ): scene( NULL ) {
	
//}

shapesEditor::shapesEditor( ) {
#ifdef KM_LOG_INSTANCIATIONS
	ofLogNotice("shapesEditor::shapesEditor()");
#endif
	
	// init variables
	activeShape = NULL;
	enableEditingToggle = false;
	fullScreenToggle = false;;
	editMode = EDIT_MODE_OFF;
	
	buildMenus();
	
	// load image
	background.load("vendome_full.jpg");
}

shapesEditor::~shapesEditor(){
	
	// menu event listeners
	saveButton.removeListener(this, &shapesEditor::showSaveDialog);
	loadButton.removeListener(this, &shapesEditor::showLoadMenu);
	fullScreenToggle.removeListener(this, &shapesEditor::setFullScreen);
	enableEditingToggle.removeListener(this, &shapesEditor::enableShapeEditing );
	
	disableEditing();
	
	//delete gui;
	//delete batchGui;
	
	// todo: save app settings on quit
}

bool shapesEditor::isEditorClass(){
	return true;
}

void shapesEditor::update() {
	// only use computing power if in edit mode.
	if( !isInEditMode() || editMode==EDIT_MODE_RENDER ) return;
	
	// check for shape deletion
	// trick from http://stackoverflow.com/a/8621457/58565
	for(auto s = shapes.rbegin(); s!=shapes.rend(); ){
		if( (*s)->pleaseDeleteMe ){
			if(activeShape == *s) selectShape(NULL);
			s++;
			s= std::list<basicShape*>::reverse_iterator( shapes.erase(s.base()) );
		}
		else s++;
	}
	
	// single edit mode
	if( editMode==EDIT_MODE_SHAPE ){
		
		if( activeShape != NULL ){
			//activeShape->update();
		}
		
	}
	
	/*/ prevent pointhandlers from moving
	if( editMode==EDIT_MODE_BATCH_SELECT ){
		syncMultiSelectionHandlers();
	}
	// update multiPointHandlers
	else if( multiShapesSelection.size()>0){
		
		// did one point move ? (compare to rescale
		for(int i = 0; i < multiPointHandlers.size(); i++){
			ofVec2f newPos = multiPointHandlers[i].getPos();
			ofVec2f oldPos = i==0?ofVec2f(multiShapesBoundingBox.x, multiShapesBoundingBox.y):i==1?ofVec2f(multiShapesBoundingBox.x+multiShapesBoundingBox.width, multiShapesBoundingBox.y):i==2?ofVec2f(multiShapesBoundingBox.x+multiShapesBoundingBox.width, multiShapesBoundingBox.y+multiShapesBoundingBox.height):i==3?ofVec2f(multiShapesBoundingBox.x, multiShapesBoundingBox.y+multiShapesBoundingBox.height):ofVec2f(0);
			
			// movement detected?
			if(newPos!=oldPos){
				
				// remember current Rect
				ofRectangle originalBoundingBox(multiShapesBoundingBox);
				
				if( editMode==EDIT_MODE_BATCH_SCALE ){
					// lock ratio ?
					if( ofGetKeyPressed( OF_KEY_SHIFT ) ){
						ofPolyline line;
						ofPoint direction(multiShapesBoundingBox.width, multiShapesBoundingBox.height);
						if(i%2==1) direction.y*=-1.f;
						
						direction.normalize();
						line.addVertex(oldPos+direction*-50);
						line.addVertex(oldPos+direction*50);
						line.close();
						newPos=line.getClosestPoint(newPos);
					}
					
					ofVec2f diff = newPos-oldPos;
					
					if(ofGetKeyPressed(OF_KEY_ALT)){
						// make diff relative to multiPointHandlers[0]
						if(i==1) diff.x*=-1.f;
						else if(i==2) diff*=-1.f;
						else if(i==3) diff.y*=-1.f;
						
						multiShapesBoundingBox.set( multiShapesBoundingBox.getPosition()+diff, multiShapesBoundingBox.getWidth()-diff.x*2.f, multiShapesBoundingBox.getHeight()-diff.y*2.f );
						
						//break; // dont look at the other points now they have moved too
					}
					else switch (i) {
						case 0:
							multiShapesBoundingBox.set( multiShapesBoundingBox.getPosition()+diff, multiShapesBoundingBox.width-diff.x, multiShapesBoundingBox.height-diff.y );
							break;
						case 1:
							multiShapesBoundingBox.set( multiShapesBoundingBox.getPosition()-ofVec2f(0,-diff.y), multiShapesBoundingBox.width+diff.x, multiShapesBoundingBox.height-diff.y );
							break;
						case 2:
							multiShapesBoundingBox.set( multiShapesBoundingBox.getPosition(), multiShapesBoundingBox.width+diff.x, multiShapesBoundingBox.height+diff.y );
							break;
						case 3:
							multiShapesBoundingBox.set( multiShapesBoundingBox.getPosition()+ofVec2f(diff.x,0), multiShapesBoundingBox.width-diff.x, multiShapesBoundingBox.height+diff.y );
							break;
							
						default:
							break;
					}
					
					// put handlers on new container rect
					syncMultiSelectionHandlers();
					
					// calc transformation variables
					ofVec2f scale(multiShapesBoundingBox.width/originalBoundingBox.width, multiShapesBoundingBox.height/originalBoundingBox.height);
					
					// apply transformation to shapes
					for(list<basicShape*>::iterator it=multiShapesSelection.begin(); it!=multiShapesSelection.end(); it++){
						basicShape* s = (*it);
						
						// move its position according to scale factor
						ofVec2f newShapePos( ofMap(s->getPositionPtr()->x, originalBoundingBox.x, originalBoundingBox.x+originalBoundingBox.width, multiShapesBoundingBox.x, multiShapesBoundingBox.x+multiShapesBoundingBox.width), ofMap(s->getPositionPtr()->y, originalBoundingBox.y, originalBoundingBox.y+originalBoundingBox.height, multiShapesBoundingBox.y, multiShapesBoundingBox.y+multiShapesBoundingBox.height) );
						s->setPosition( newShapePos );
						
						// apply scale to shape
						s->applyScale(scale);
						
					}
					
					// dont check other points
					break;
				}
				
				// simple move tool
				else if( editMode==EDIT_MODE_BATCH_MOVE ){
					ofVec2f diff = newPos-oldPos;
					multiShapesBoundingBox.set( multiShapesBoundingBox.getPosition()+diff, multiShapesBoundingBox.width, multiShapesBoundingBox.height );
					syncMultiSelectionHandlers();
					
					// apply transformation to shapes
					for(list<basicShape*>::iterator it=multiShapesSelection.begin(); it!=multiShapesSelection.end(); it++){
						basicShape* s = (*it);
						
						// just move the shape
						s->setPosition( (*s->getPositionPtr()) + diff );
						
					}
					
					// dont check other points
					break;
				}
			}
		}
	}
	
	/ * / update GUI ?
	if(isInEditMode()){
		guiToggle.setPosition( boundingBox.getTopRight()+5 );
	}*/
}

void shapesEditor::draw() {
	// only use computing power if in edit mode.
	if( !isInEditMode() ) return;
	
	ofSetColor(255);
	background.draw(0,0);
	
	// tmp ofScale(0.5, 0.5);
	// draw shapes (edit mode)
	for(auto it = shapes.begin(); it != shapes.end(); it++){
		if( (*it)->isReady() && *it != activeShape ){
			if( (*it)->isInEditMode() ) (*it)->render();
			else (*it)->sendToGPU();
		}
	}
	
	// draw active shape
	if( activeShape ) activeShape->render();

	// draw rescale tool
	if(editMode!=EDIT_MODE_RENDER && multiShapesSelection.size()>0){
		// highlight selection
		for(auto it = multiShapesSelection.begin(); it != multiShapesSelection.end(); it++){
			if( (*it)->isReady() ){
				ofSetColor(0);
				(*it)->sendToGPU();
			}
		}
		
		// show bounding box
		ofPushStyle();
		ofSetColor(255,50);
		ofDrawRectangle(multiShapesBoundingBox);
		ofSetColor(0);
		ofNoFill();
		ofDrawRectangle(multiShapesBoundingBox);
		ofPopStyle();
		
		// draw handlers
		for(int i=0; i<multiPointHandlers.size(); i++){
			multiPointHandlers[i].draw();
		}
	}
	
	editorGui.draw();
}

// - - - - - - - -
// EDIT MODE TOGGLING
// - - - - - - - -

void shapesEditor::enableEditing(){
	if( !isInEditMode() ) setEditMode(EDIT_MODE_RENDER);
}

void shapesEditor::disableEditing(){
	if( isInEditMode() ) setEditMode( EDIT_MODE_OFF );
}

void shapesEditor::switchEditing(){
	isInEditMode()?disableEditing():enableEditing();
}

bool shapesEditor::isInEditMode() const{
	return !(editMode==EDIT_MODE_OFF);
}

// - - - - - - - -
// EVENT LISTENERS
// - - - - - - - -
void shapesEditor::_draw(ofEventArgs &e){
	draw();
}

void shapesEditor::_update(ofEventArgs &e){
	update();
}

void shapesEditor::guiEvent(ofxUIEventArgs &e){
	string name = e.getName();
	
	// scan for open dropdowns
	
	// interface options
	// - - - - - - - - -
	if(name==GUIToggleFullScreen){
		ofxUIToggle *toggle = (ofxUIToggle *) e.widget;
		ofSetFullscreen(toggle->getValue());
	}
	
	// selection handling
	// - - - - - - - - - -
	else if(name==GUISelectNextShape){
		if( ( (ofxUILabelButton*) e.getButton())->getValue() == true )
			selectNextShape();
	}
	else if(name==GUISelectPrevShape){
		if( ( (ofxUILabelButton*) e.getButton())->getValue() == true )
			selectPrevShape();
	}
	else if(name==GUIDeleteShape){
		if( ( (ofxUILabelButton*) e.getButton())->getValue() == true ){
			// is a shape selected & does it still exist ?
			if(activeShape!=NULL && shapeExists(activeShape) ){
				removeShape(activeShape);
				
				// deselect
				selectShape(NULL);
			}
		}
	}
	else if(name==GUImultiShapesSelection){
		if( ( (ofxUILabelButton*) e.getButton())->getValue() == true ){
			selectShape(NULL);
			setEditMode(EDIT_MODE_BATCH_SELECT);
		}
	}
	
	// FILE SAVING & LOADING
	// - - - - - - - - - - -
	else if(name==GUIConfigFile){
        //selectConfigFileOpen = !selectConfigFileOpen;
		ofxUIDropDownList* ddl = (ofxUIDropDownList*) e.widget;
		
		// update configuration
		vector<string> names = ddl->getSelectedNames();
		// set (or empty) config file
		loadedConfiguration=(names.size()>0)?names[0]:"";
		
		// add new configuration ?
		if(loadedConfiguration=="- new configuration -"){
			loadedConfiguration = ofSystemTextBoxDialog("Enter a name for the new configuration save file.", "MyGeniusNewConfiguration")+".xml";
			ddl->addToggle(loadedConfiguration);
			ddl->clearSelected();
			ddl->activateToggle(loadedConfiguration); // select current item in the list
		}
		
		// update dropdown title
		ddl->setLabelText("File: "+loadedConfiguration);
		ddl->setName(ddl->getName());
		ddl->activateToggle(loadedConfiguration);
	}
	else if(name==GUILoadConfig){
		if( ( (ofxUILabelButton*) e.getButton())->getValue() == true ){
			if(!loadScene(loadedConfiguration)) ofSystemAlertDialog("Failed to load from file: "+loadedConfiguration);
		}
	}
	else if(name==GUISaveConfig){
		if( ( (ofxUILabelButton*) e.getButton())->getValue() == true ){
			// warn if no save file
			if(loadedConfiguration.empty()){
				ofSystemAlertDialog("In order to save, you must first select a configuration file.");
			}
			else saveScene(loadedConfiguration);
		}
	}
	else if(name==GUIAddShape){
		ofxUIDropDownList* ddl = (ofxUIDropDownList*) e.widget;
		
		// update configuration
		vector<string> shapes = ddl->getSelectedNames();
		
		// never remain selected
		ddl->clearSelected();
		
		// set (or empty) config file
		string selected=(shapes.size()>0)?shapes[0]:"";
		
		// no selection ?
		if(selected.empty()) return;
		
		// try to add shape
		bool shapeExists = addShape( selected );
		
	}
	//else cout << "shapesEditor::guiEvent() --> " << name << endl;
}

void shapesEditor::batchGuiEvent(ofxUIEventArgs &e){
	string name = e.getName();
	
	if(name==batchGUISelectAll){
		if( ( (ofxUILabelButton*) e.getButton())->getValue() == true ){
			multiShapesSelection.clear();
			for(auto it=shapes.begin(); it!=shapes.end(); it++){
				multiShapesSelection.push_back( *it );
			}
			updateMultiShapesSelection();
		}
	}
	else if(name==batchGUISelectNone){
		if( ( (ofxUILabelButton*) e.getButton())->getValue() == true ){
			multiShapesSelection.clear();
			updateMultiShapesSelection();
		}
	}
	// return back to main menu
	else if(name==batchGUICancel){
		if( ( (ofxUILabelButton*) e.getButton())->getValue() == true ){
			setEditMode( EDIT_MODE_RENDER );
			multiPointHandlers.clear();
			multiShapesSelection.clear();
			updateMultiShapesSelection();
		}
	}
	else if (name==batchGUIRescaleMode){
		if( ( (ofxUILabelToggle*) e.getButton())->getValue() == true )
			setEditMode( EDIT_MODE_BATCH_SCALE );
		else setEditMode( EDIT_MODE_BATCH_SELECT );
	}
	else if (name==batchGUIMoveMode){
		if( ( (ofxUILabelToggle*) e.getButton())->getValue() == true )
			setEditMode( EDIT_MODE_BATCH_MOVE );
		else setEditMode( EDIT_MODE_BATCH_SELECT );
	}
	else if(name==batchGUIFlipXMode || name==batchGUIFlipYMode){
		if( ( (ofxUILabelToggle*) e.getButton())->getValue() == true){
			basicPoint scale(name==batchGUIFlipXMode?-1:1, name==batchGUIFlipYMode?-1:1);
			
			// mirror the positions
			ofRectangle originalBoundingBox(multiShapesBoundingBox);
			//multiShapesBoundingBox.set(multiShapesBoundingBox.getPosition()*scale);
			multiShapesBoundingBox.scaleFromCenter(scale.x, scale.y);
			
			// apply transformation to shapes
			for(auto it=multiShapesSelection.begin(); it!=multiShapesSelection.end(); it++){
				
				// move its position according to scale factor
				basicPoint newShapePos(
					ofMap((*it)->getPositionPtr()->x, originalBoundingBox.x, originalBoundingBox.x+originalBoundingBox.width, multiShapesBoundingBox.x, multiShapesBoundingBox.x+multiShapesBoundingBox.width),
					ofMap((*it)->getPositionPtr()->y, originalBoundingBox.y, originalBoundingBox.y+originalBoundingBox.height, multiShapesBoundingBox.y, multiShapesBoundingBox.y+multiShapesBoundingBox.height)
				);
				(*it)->setPosition( newShapePos );
				
				// apply scale to shape
				(*it)->applyScale(scale);
			}
			// also prevents them from being moved
			syncMultiSelectionHandlers();
			
			setEditMode( EDIT_MODE_BATCH_SELECT );
		}
		// dont continue flipping forever
		else setEditMode( EDIT_MODE_BATCH_SELECT );
	}
	else cout << "batchGuiEvent: " << name << endl;
}

void shapesEditor::updateMultiShapesSelection(){
	if(multiShapesSelection.size()==0){
		multiShapesBoundingBox.set(0,0,0,0);
		multiPointHandlers.clear();
		return;
	}
	
	// getRectFromShapes
	ofVec2f from(ofGetWidth(),ofGetHeight());
	ofVec2f to(0,0);
	for(auto it=multiShapesSelection.begin(); it!=multiShapesSelection.end(); it++){
		ofRectangle bb = (*it)->getBoundingBox();
		
		if(bb.x<from.x) from.x=bb.x;
		if(bb.y<from.y) from.y=bb.y;
		
		if(bb.x+bb.width > to.x) to.x=bb.x+bb.width;
		if(bb.y+bb.height > to.y) to.y=bb.y+bb.height;
	}
	multiShapesBoundingBox.set(from.x, from.y, to.x-from.x, to.y-from.y);
	
	// update gui nb selected shapes
	// todo
	//batchGui->getWidget(batchGUINbSelected);
	
	syncMultiSelectionHandlers();
}

void shapesEditor::syncMultiSelectionHandlers(){
	
	// create pointHandlers
	if(multiPointHandlers.size()!=4){
		multiPointHandlers.clear();
		multiPointHandlers.resize(4);
		for(int i=0; i<multiPointHandlers.size(); i++){
			multiPointHandlers[i].setEditable( true );
		}
	}
	// setHandlersOnRect
	for(int i=0; i<multiPointHandlers.size(); i++){
		switch (i) {
			case 0:
				multiPointHandlers[i].setPos( basicPoint(multiShapesBoundingBox.x, multiShapesBoundingBox.y) );
				break;
			case 1:
				multiPointHandlers[i].setPos( basicPoint(multiShapesBoundingBox.x+multiShapesBoundingBox.width, multiShapesBoundingBox.y) );
				break;
			case 2:
				multiPointHandlers[i].setPos( basicPoint(multiShapesBoundingBox.x+multiShapesBoundingBox.width, multiShapesBoundingBox.y+multiShapesBoundingBox.height) );
				break;
			case 3:
				multiPointHandlers[i].setPos( basicPoint(multiShapesBoundingBox.x, multiShapesBoundingBox.y+multiShapesBoundingBox.height) );
				break;
			default:
				break;
		}
	}
}


// - - - - - - - -
// CUSTOM FUNCTIONS
// - - - - - - - -

// adds a shape
bool shapesEditor::addShape(string _shapeType){
	// todo: make this shape-specific (check if shape type exists)
	vertexShape* s = new vertexShape();
	
	basicShape* result = insertShape(s);
	if( result == NULL ) return false;
	
	// enable edit mode
	selectShape( static_cast<basicShape*>(result) );
	
	return true;
}

// returns true if shape is unexistent after deletion (even if it didn't exist)
bool shapesEditor::removeShape(basicShape *_s){
	
	if( !removeShape(_s) ) return false;
	
	// deselect ?
	if(activeShape==_s) selectShape(NULL);
	
	// todo
	
	return true;
}

void shapesEditor::selectShape(basicShape* _i){
	
	// disable edit mode on current one
	if( shapeExists(activeShape) ){
		activeShape->disableEditMode();
	}
	
	// select it
	if( shapeExists(_i) ){
		
		// remember
		activeShape = _i;
		
		// enter edit mode
		// todo: even if its a vertexShape, this seems to only call basicShape::enableEditmode();
		_i->enableEditMode();
		
		menuNumSelectedShapes = "1";
	}
	else{
		activeShape = NULL;
		menuNumSelectedShapes = "0";
	}
}

void shapesEditor::selectNextShape(){
	if( getNumShapes()==1 && activeShape!=NULL) selectShape(NULL);
	else if(activeShape==NULL) selectShape( shapes.front() );
	
	else{
		basicShape* next = NULL;
		for(auto it = shapes.begin(); it != shapes.end(); it++){
			if(activeShape == (*it)){
				it++;
				next=(*it);
				break;
			}
		}
		selectShape( next );
	}
}

void shapesEditor::selectPrevShape(){
	if( getNumShapes()==1 && activeShape!=NULL) selectShape(NULL);
	else if(activeShape==NULL) selectShape( shapes.back() );
	
	else{
		basicShape* prev = NULL;
		for(auto it = shapes.rbegin(); it != shapes.rend(); it++){
			if(activeShape == (*it)){
				it++;
				prev=(*it);
				break;
			}
		}
		selectShape( prev );
	}
}

bool shapesEditor::setEditMode(shapesEditMode _mode){
	// edit mode gets disabled
	if( isInEditMode() && _mode==EDIT_MODE_OFF){
		cout << "shapesEditor::setEditMode() OFF" << endl;
		
		selectShape(NULL);
		// unregister editor events
		ofRemoveListener(ofEvents().mousePressed, this, &shapesEditor::_mousePressed);
		
		ofRemoveListener( ofEvents().mousePressed, this, &shapesEditor::_mousePressed );
		ofRemoveListener( ofEvents().draw, this, &shapesEditor::_draw );
		ofRemoveListener( ofEvents().update, this, &shapesEditor::_update );
		
		//ofRemoveListener( batchGui->newGUIEvent, this, &shapesEditor::batchGuiEvent );
		//ofRemoveListener( gui->newGUIEvent, this, &shapesEditor::guiEvent );
		
		
		//gui->disable();
		//batchGui->disable();
	}
	
	// edit mode just got enabled
	else if(!isInEditMode() && _mode==EDIT_MODE_RENDER){
		cout << "shapesEditor::setEditMode() ON" << endl;
		
		selectShape(NULL);// todo: make this remember
		
		// enable listeners
		ofAddListener( ofEvents().mousePressed, this, &shapesEditor::_mousePressed );
		ofAddListener( ofEvents().draw, this, &shapesEditor::_draw );
		ofAddListener( ofEvents().update, this, &shapesEditor::_update );
	}
	
	// sho batch edit GUI ?
	/*else if(_mode==EDIT_MODE_BATCH_SELECT && !batchGui->isEnabled() ){
		gui->disable();
		batchGui->enable();
	}*/
	
	// remember
	editMode = _mode;
	
	// No selection allowed in render mode
	if( editMode==EDIT_MODE_RENDER){
		if(activeShape) selectShape(NULL);
	}
	
	// desselect batchGui elements
	/*if( batchGui!=NULL ){
		if( _mode!=EDIT_MODE_BATCH_MOVE ) ((ofxUILabelToggle*)batchGui->getWidget(batchGUIMoveMode))->setValue(false);
		if( _mode!=EDIT_MODE_BATCH_SCALE ) ((ofxUILabelToggle*)batchGui->getWidget(batchGUIRescaleMode))->setValue(false);
		if( _mode!=EDIT_MODE_BATCH_FLIPX ) ((ofxUILabelToggle*)batchGui->getWidget(batchGUIFlipXMode))->setValue(false);
		if( _mode!=EDIT_MODE_BATCH_FLIPY ) ((ofxUILabelToggle*)batchGui->getWidget(batchGUIFlipYMode))->setValue(false);
	}*/
	
	return (editMode == _mode);
}

// - - - - - - - -
// EVENT LISTENERS
// - - - - - - - -

void shapesEditor::_mousePressed(ofMouseEventArgs &e){
	if( !isInEditMode() ) return;
	
	// ignore editor guiMenu clicks
	if( editorGui.getShape().inside( e.x, e.y) ){
		return;
	}
	
	else if( editMode==EDIT_MODE_SHAPE ){
		
		if( e.button==0 ){
			// notify active shape of click ?
			if( activeShape && activeShape->interceptMouseClick( e ) ){
				return;
			}
			// check if we can select a shape ?
			else {
				for(auto it=shapes.begin(); it!=shapes.end(); it++){
					if( (*it)->isInside( e )){
						selectShape(*it);
					}
				}
			}
		}
	}
	
	// let user select shapes by clicking on them
	else if( editMode==EDIT_MODE_BATCH_SELECT && e.button==0){
		for(auto it=shapes.begin(); it!=shapes.end(); it++){
			if( (*it)->isInside( e )){
				
				// check if already inside ?
				bool found = false;
				for(auto its=multiShapesSelection.begin(); its!=multiShapesSelection.end(); its++){
					// match ?
					if((*its) == (*it)){
						found = true;
						
						// remove from selection
						multiShapesSelection.erase(its);
						
						break;
					}
				}
				
				if(!found) multiShapesSelection.push_back( *it );
					
				updateMultiShapesSelection();
			}
		}
	}
}

void shapesEditor::_keyPressed(ofKeyEventArgs &e){
	if(e.key=='h' || e.key=='H'){
		mouseHidden ? ofShowCursor() : ofHideCursor();
		mouseHidden = !mouseHidden;
		
	}
}


// - - - - - - - -
// MENU EVENT LISTENERS
// - - - - - - - -
void shapesEditor::showLoadMenu(){
	//ofRectangle btn = loadButton.getShape();
	ofFileDialogResult openFileResult= ofSystemLoadDialog("Select the XML file to load...", false, ofToDataPath("saveFiles/"));
	
	if(openFileResult.bSuccess){
		// restrict to saveFiles dir
		ofFile file( ofToDataPath("saveFiles/")+openFileResult.getName() );
		if(file.exists()){
			loadScene( openFileResult.getName() );
		}
	}
}

void shapesEditor::showSaveDialog(){
	//ofRectangle btn = loadButton.getShape();
	ofFileDialogResult openFileResult= ofSystemSaveDialog(loadedConfiguration,"Where shall I save your scene?");
	
	if(openFileResult.bSuccess){
		// only keep the file name, not the path
		saveScene( openFileResult.getName() );
	}
}

void shapesEditor::setFullScreen(bool & _fullScreen){
	ofSetFullscreen( _fullScreen );
}

void shapesEditor::enableShapeEditing(bool &_on){
	if(_on) setEditMode(EDIT_MODE_SHAPE);
	else setEditMode(EDIT_MODE_RENDER);
}

void shapesEditor::buildMenus(){
	// New GUI setup
	editorGui.setup("SHAPES EDITOR");
	//editorGui.setShowHeader(false);
	//editorGui.add( (new ofxLabelExtended())->setup("", "Welcome to the scene editor :)")->setShowLabelName(false) );
	
	editorGui.add( (new ofxGuiSpacer()) );
	
	
	// Load & save
	editorMenu.setup("Scene Editor Menu");
	loadButton.setup( GUILoadConfig );
	loadButton.addListener(this, &shapesEditor::showLoadMenu);
	editorMenu.add(&loadButton);
	
	saveButton.setup( GUISaveConfig );
	saveButton.addListener(this, &shapesEditor::showSaveDialog);
	editorMenu.add( &saveButton );
	
	editorMenu.add( (new ofxGuiSpacer()) );
	
	fullScreenToggle.setup( GUIToggleFullScreen, fullScreenToggle );
	fullScreenToggle.addListener(this, &shapesEditor::setFullScreen);
	editorMenu.add( &fullScreenToggle );
	editorMenu.add( (new ofxGuiSpacer()) );
	
	editorGui.add( &editorMenu );
	
	
	shapesMenu.setup("Shape Editing");
	
	menuNumSelectedShapes.set( GUINbSelectedShapes, "0");
	shapesMenu.add( (new ofxLabel(menuNumSelectedShapes)) );
	
	enableEditingToggle.setup( GUIEnableEditing, enableEditingToggle );
	enableEditingToggle.addListener(this, &shapesEditor::enableShapeEditing );
	
	shapesMenu.add( &enableEditingToggle );
	
	batchModeSelect.setup("Batch Editing",2);
	batchModeSelect.add(new ofxMinimalToggle(ofParameter<bool>("Disabled",false)));
	batchModeSelect.add(new ofxMinimalToggle(ofParameter<bool>("MultiSelect Mode",false)));
	batchModeSelect.add(new ofxMinimalToggle(ofParameter<bool>("Scale",false)));
	batchModeSelect.add(new ofxMinimalToggle(ofParameter<bool>("Move",false)));
	batchModeSelect.allowMultipleActiveToggles(false);
	shapesMenu.add( &batchModeSelect );
	
	editorGui.add( &shapesMenu );
	
	/*
	
	gui->addLabel("New shape", OFX_UI_FONT_LARGE);
	vector<string> items;
	items.push_back("vertexShape"); // todo: auto populate this with all available shapes
	ddl = gui->addDropDownList(GUIAddShape, items);
	ddl->setAllowMultiple(false);
	ddl->setAutoClose(true);
	
	gui->autoSizeToFitWidgets();
	
	ofAddListener(gui->newGUIEvent, this, &shapesEditor::guiEvent);
	
	
	// setup rescale GUI
	textString = "This tool lets you rescale several shapes.";
	batchGui->addTextArea("textarea", textString, OFX_UI_FONT_SMALL);
	textString = "Select the target shapes by clicking on them, then proceed to scaling.";
	batchGui->addTextArea("textarea", textString, OFX_UI_FONT_SMALL);
	textString = "Scale modifier keys: ALT for reflected scaling, SHIFT to lock aspect ratio";
	batchGui->addTextArea("textarea", textString, OFX_UI_FONT_SMALL);
	batchGui->addSpacer();
	
	batchGui->addLabelButton(batchGUISelectAll, false);
	batchGui->addLabelButton(batchGUISelectNone, false);
	
	/* / generate list of all shapes
	 int i = 0;
	 for(list<basicShape*>::iterator it=shapes.begin(); it!=shapes.end(); it++){
	 batchGui->addToggle((*it)->shapeType+"[" + ofToString(i)+"]" + ofToString(shapes.size()), false);
	 i++;
	 }* /
	
	batchGui->addSpacer();
	batchGui->addLabel("Actions to permorm:", OFX_UI_FONT_MEDIUM);
	batchGui->addLabelToggle(batchGUIRescaleMode, false);
	batchGui->addLabelToggle(batchGUIMoveMode, false);
	batchGui->addLabelToggle(batchGUIFlipXMode, false);
	batchGui->addLabelToggle(batchGUIFlipYMode, false);
	
	batchGui->addSpacer();
	batchGui->addLabelButton(batchGUICancel, false);
	
	batchGui->autoSizeToFitWidgets();
	batchGui->disable();
	
	ofAddListener(batchGui->newGUIEvent, this, &shapesEditor::batchGuiEvent);*/
}

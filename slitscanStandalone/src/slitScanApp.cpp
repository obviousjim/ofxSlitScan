/**
 * 
 * The MIT License
 * 
 * Copyright (c) 2010, 2011 james george
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 * slitScanApp
 * 
 * an example openFrameworks appliation showing how to use the ofxSlitScan add on
 */

#include "slitScanApp.h"

#define WIDTH 640
#define HEIGHT 480

void slitScanApp::setup(){
	
	// This makes relative paths work in C++ in Xcode by changing directory to the Resources folder inside the .app bundle
	// this way you can include your images in the copy phase of the project and don't have to rely on a data/ folder for distribution
	
//	//TODO Need a XPlatform solution for this
//	#ifdef TARGET_OSX
//	ofDisableDataPath();
//    CFBundleRef mainBundle = CFBundleGetMainBundle();
//    CFURLRef resourcesURL = CFBundleCopyResourcesDirectoryURL(mainBundle);
//    char path[PATH_MAX];
//    CFURLGetFileSystemRepresentation(resourcesURL, TRUE, (UInt8 *)path, PATH_MAX);
//    CFRelease(resourcesURL);
//    chdir(path);
//	#endif
//	
	ofSetVerticalSync(true);
	ofSetLogLevel(OF_LOG_VERBOSE);
		
	capacity = 20;
	
	sampleMapStrings.push_back("maps/left_to_right.png");
	sampleMapStrings.push_back("maps/right_to_left.png");
	sampleMapStrings.push_back("maps/up_to_down.png");
	sampleMapStrings.push_back("maps/down_to_up.png");
	sampleMapStrings.push_back("maps/hard_noise.png");
	sampleMapStrings.push_back("maps/soft_noise.png");
	sampleMapStrings.push_back("maps/random_grid.png");
	sampleMapStrings.push_back("maps/video_delay.png");
	
	for(int i = 0; i < sampleMapStrings.size(); i++){
		ofImage* map = new ofImage();
		map->allocate(WIDTH, HEIGHT, OF_IMAGE_GRAYSCALE);
		map->loadImage(sampleMapStrings[i]);
		sampleMaps.push_back( map );
	}
		
    colorImg.allocate(WIDTH,HEIGHT, OF_IMAGE_COLOR);
	previewImage.allocate(WIDTH, HEIGHT, OF_IMAGE_COLOR);
	warpImage.allocate(WIDTH, HEIGHT, OF_IMAGE_COLOR);
	customMap.allocate(WIDTH, HEIGHT, OF_IMAGE_GRAYSCALE);
	
	currentSampleMapIndex = 0;
	
	isDraggingLeft = false;
	isDraggingRight = false;
	isDraggingCapacity = false;
	
	isFullScreen = false;
	isPaused = false;
	
	leftClamp = 0.0f;
	rightClamp = 1.0f;
		
	changeCapacity();
	initLiveVideo();
	
	warp.setBlending(true);
	warp.setDelayMap(*(sampleMaps[0]));
	
	//load buttons
	loadYourOwn.loadImage("images/loadyourown.png");
	buttonLiveOn.loadImage("images/live_on.png");
	buttonMovieOn.loadImage("images/movie_on.png");
	buttonPauseOn.loadImage("images/pause_on.png");
	buttonPauseOff.loadImage("images/pause_off.png");
	buttonBlendingOn.loadImage("images/blending_on.png");
	buttonBlendingOff.loadImage("images/blending_off.png");
	buttonFullscreenOn.loadImage("images/fullscreen_on.png");
	buttonFullscreenOff.loadImage("images/fullscreen_off.png");
	
	leftHandle.loadImage("images/handle_left.png");
	rightHandle.loadImage("images/handle_right.png");

	frameCapacityBar.loadImage("images/capacity_full.png");
	frameCapacityText.loadImage("images/frame_capacity.png");
	
	bottomImage.loadImage("images/bottom_banner.png");
}

void slitScanApp::initLiveVideo(){
	if(!useLiveVideo){
		useLiveVideo = true;		
		vidPlayer.close();
	}
		
	vidGrabber.initGrabber(WIDTH,HEIGHT);	

	vidGrabber.listDevices();
	vidGrabber.setDeviceID(10);

}

void slitScanApp::initMovie(){
	ofFileDialogResult r = ofSystemLoadDialog("Open Movie", false);
	
	string file = r.getPath();
	if (r.bSuccess && file != "") {
		if(useLiveVideo){
			useLiveVideo = false;
			vidGrabber.close();
		}
		vidPlayer.loadMovie(file);
		vidPlayer.setSpeed(1);
		movieImg.allocate(vidPlayer.width, vidPlayer.height, OF_IMAGE_COLOR);
		vidPlayer.play();	
		if(vidPlayer.width != WIDTH || vidPlayer.height != HEIGHT){
			movieImage.allocate(vidPlayer.width, vidPlayer.height, OF_IMAGE_COLOR);
		}
	}
}

void slitScanApp::update(){
    bool bNewFrame = false;	
	if(useLiveVideo){
		vidGrabber.grabFrame();
		bNewFrame = vidGrabber.isFrameNew();
	}
	else{
        vidPlayer.idleMovie();
		bNewFrame = vidPlayer.isFrameNew();
	}

	if (bNewFrame && !isPaused){
		if(useLiveVideo){
            colorImg.setFromPixels(vidGrabber.getPixels(), WIDTH,HEIGHT, OF_IMAGE_COLOR);
	    }
		else{
			unsigned char* inPixels;
			if(vidPlayer.width == WIDTH || vidPlayer.height == HEIGHT){
				inPixels = vidPlayer.getPixels();
			}
			else {
				movieImage.setFromPixels(vidPlayer.getPixels(), vidPlayer.width,vidPlayer.height, OF_IMAGE_COLOR);
				ofImage resized = movieImage;
				resized.resize(WIDTH, HEIGHT);
				inPixels = resized.getPixels();
			}
			colorImg.setFromPixels(inPixels, WIDTH, HEIGHT, OF_IMAGE_COLOR);
		}
		
		//each frame, call addImage to ofxSlitScan.
		warp.addImage(colorImg);
	}
}


#define FILM_STRIP_Y 100

#define MAX_CAPACITY 500

#define HANDLE_WIDTH 10
#define FRAME_PADDING 10
#define BUTTON_PADDING 4

#define DRAW_WIDTH (ofGetWidth())
#define MAP_THUMB_WIDTH 134
#define MAP_THUMB_HEIGHT 96
#define SELECTOR_HEIGHT (MAP_THUMB_HEIGHT+FRAME_PADDING*2)
#define VIEWPORT_HEIGHT (HEIGHT + FRAME_PADDING*2)
#define VIEWPORT_WIDTH (WIDTH + FRAME_PADDING*2)

#define HUD_BUTTON_WIDTH 125
#define HUD_BUTTON_HEIGHT 30

#define CLAMP_HANDLE_WIDTH 30

#define CAPACITY_SLIDER_HEIGHT 30
#define BOTTOM_LABEL_HEIGHT 50

#define COLOR_YELLOW_HIGHLIGHT 0xfbb661
#define COLOR_YELLOW 0xac9966
#define COLOR_GREEN_LIGHT 0x5c7b6a
#define COLOR_GREEN_DARK 0x27676d
#define COLOR_DARK_GREY 0x38393f

void slitScanApp::draw(){
	
	if(!useLiveVideo){
		//weird video player bug... may not need this any more
		vidPlayer.play();
	}
	
	ofPushStyle();{
		ofSetHexColor(COLOR_GREEN_DARK);
		ofRect(0, 0, ofGetWidth(), ofGetHeight());
	}ofPopStyle();
	
	if(isFullScreen){
		warp.getOutputImage()->draw(0, 0, ofGetWidth(), ofGetHeight());
		return;
	}
	
	//Draw top selector
	ofPushStyle(); ofPushMatrix(); {
		
		ofFill();
		ofSetHexColor(COLOR_GREEN_DARK);
		ofRect(0, 0, DRAW_WIDTH, SELECTOR_HEIGHT);
		
		ofSetHexColor(0xffffff);
		for(int i = 0; i <= sampleMaps.size(); i++){
			//if it's selected, draw a selection highlight
			if(i == currentSampleMapIndex){
				ofPushStyle();{
					ofSetHexColor(COLOR_GREEN_LIGHT);
					ofRect(FRAME_PADDING/2 + i*(MAP_THUMB_WIDTH+FRAME_PADDING), 
						   FRAME_PADDING/2, 
						   MAP_THUMB_WIDTH+FRAME_PADDING, 
						   MAP_THUMB_HEIGHT+FRAME_PADDING);
					
					ofNoFill();
					ofSetHexColor(COLOR_YELLOW_HIGHLIGHT);
					ofRect(FRAME_PADDING/2 + i*(MAP_THUMB_WIDTH+FRAME_PADDING), 
						   FRAME_PADDING/2, 
						   MAP_THUMB_WIDTH+FRAME_PADDING, 
						   MAP_THUMB_HEIGHT+FRAME_PADDING);
					
				} ofPopStyle();
			}
			
			if(i < sampleMaps.size()){
				//draw the map
				sampleMaps[i]->draw(FRAME_PADDING + i*(MAP_THUMB_WIDTH+FRAME_PADDING), 
								FRAME_PADDING, 
								MAP_THUMB_WIDTH, 
								MAP_THUMB_HEIGHT );
			}
			else{
				if(currentSampleMapIndex == i){
					customMap.draw(FRAME_PADDING + i*(MAP_THUMB_WIDTH+FRAME_PADDING), 
								   FRAME_PADDING, 
								   MAP_THUMB_WIDTH, 
								   MAP_THUMB_HEIGHT);
				}
				else{
					ofSetHexColor(0xFFFFFF);
					loadYourOwn.draw(FRAME_PADDING + i*(MAP_THUMB_WIDTH+FRAME_PADDING), FRAME_PADDING);				
				}
			}
		}
		
	} ofPopMatrix(); ofPopStyle();	
	
	
	ofTranslate(0, SELECTOR_HEIGHT);

	//Draw view ports
	ofPushStyle(); ofPushMatrix(); {
		
		ofFill();
		ofSetHexColor(COLOR_YELLOW);
		ofRect(0, 0, WIDTH+(FRAME_PADDING*2), HEIGHT+(FRAME_PADDING*2));
		
		ofSetHexColor(0xffffff);
		warp.getOutputImage()->draw(FRAME_PADDING, FRAME_PADDING);		
		
		//if we are hovering, draw the overlay
		//normalize coordinates for (0,0) at top of the overlay
		int viewMouseX = mouseX - FRAME_PADDING*2;
		int viewMouseY = mouseY - (FRAME_PADDING*2 + SELECTOR_HEIGHT); 
		if(viewMouseX > 0 && viewMouseX < WIDTH && viewMouseY > 0 && viewMouseY < HEIGHT){

			ofEnableAlphaBlending();
						
			//movie / live video selector
			ofPushMatrix(); ofPushStyle();{
				ofTranslate(FRAME_PADDING*2, FRAME_PADDING*2);
				
				if(useLiveVideo){
					buttonLiveOn.draw(0,0);
				}
				else {
					buttonMovieOn.draw(0, 0);
				}

			} ofPopMatrix(); ofPopStyle();

			//frame blender
			ofPushMatrix(); ofPushStyle(); {
				ofTranslate(WIDTH -  HUD_BUTTON_WIDTH, FRAME_PADDING*2);
				
				if(warp.isBlending()){
					buttonBlendingOn.draw(0,0);
				}
				else{
					buttonBlendingOff.draw(0, 0);				
				}
				
			} ofPopMatrix(); ofPopStyle();
			
			
			//pause button
			ofPushMatrix(); ofPushStyle();{
				
				ofTranslate(FRAME_PADDING*2, HEIGHT - ( FRAME_PADDING+HUD_BUTTON_HEIGHT));
								
				if(isPaused){
					buttonPauseOn.draw(0, 0);
				}
				else{
					buttonPauseOff.draw(0, 0);
				}
				
			} ofPopMatrix(); ofPopStyle();
			
			//full screen button
			ofPushMatrix(); ofPushStyle();{
				
				ofTranslate(WIDTH - HUD_BUTTON_WIDTH, HEIGHT - ( FRAME_PADDING+HUD_BUTTON_HEIGHT));
				
				buttonFullscreenOff.draw(0,0);
				
			} ofPopMatrix(); ofPopStyle();
		}
		
		ofTranslate(WIDTH+(FRAME_PADDING*2), 0);
		ofSetHexColor(COLOR_GREEN_DARK);
		ofRect(0, 0, WIDTH+(FRAME_PADDING*2), HEIGHT+(FRAME_PADDING*2));		
		
		ofSetHexColor(0xffffff);
		warp.getDelayMap()->draw(FRAME_PADDING, FRAME_PADDING, WIDTH, HEIGHT);
		
	} ofPopMatrix(); ofPopStyle();	
	
	ofTranslate(0, HEIGHT+FRAME_PADDING*2);

	//draw filmstrip
	ofPushStyle(); ofPushMatrix(); {
		
		ofFill();
		ofSetHexColor(COLOR_YELLOW);		
		ofRect(0, 0, DRAW_WIDTH, heightPerFrame+FRAME_PADDING*4);
		
		ofTranslate(FRAME_PADDING+CLAMP_HANDLE_WIDTH, FRAME_PADDING);
		
		ofSetHexColor(0xffffff);		
		int frameStep = warp.getCapacity() / framesToShow;
		int fameIndex = 0;
		unsigned char* pixels = previewImage.getPixels();
		for(int i = 0; i < framesToShow; i ++){
			warp.pixelsForFrame(warp.getCapacity() - (frameStep*i) - 1, pixels);
			previewImage.setFromPixels(pixels, WIDTH, HEIGHT, OF_IMAGE_COLOR);
			previewImage.draw(widthPerFrame*i, FRAME_PADDING, widthPerFrame, heightPerFrame);
		}
		
		float leftClampX = leftClamp * filmStripWidth;
		float rightClampX = rightClamp * filmStripWidth;

		ofEnableAlphaBlending();
		ofSetColor(0xac, 0x99, 0x66, 200);
		ofFill();
		
		//block out left side
		ofRect(0, 0, leftClampX, FRAME_PADDING+heightPerFrame);
		
		//block out right side
		ofRect(rightClampX, 0, filmStripWidth - rightClampX, heightPerFrame+FRAME_PADDING);
		
		//handles 
		ofSetHexColor(0xFFFFFF);
		leftHandle.draw(leftClampX - 1, 0);
		rightHandle.draw(rightClampX - CLAMP_HANDLE_WIDTH, FRAME_PADDING);
		
		ofDisableAlphaBlending();
		
	} ofPopMatrix(); ofPopStyle();	
	
	
	//draw capacity slider
	ofTranslate(0, heightPerFrame + FRAME_PADDING*3);

	ofPushStyle(); ofPushMatrix(); {
		
		ofFill();
		ofSetHexColor(COLOR_YELLOW);		
		ofRect(0, 0, DRAW_WIDTH, CAPACITY_SLIDER_HEIGHT + FRAME_PADDING*2);
		
		ofTranslate(FRAME_PADDING, FRAME_PADDING);
		
		float capacitySliderWidth = DRAW_WIDTH - FRAME_PADDING*2;
		
		ofSetHexColor(0xFFFFFF);
		frameCapacityBar.draw(0,0);
		
		ofFill();
		ofSetHexColor(COLOR_YELLOW);		
		ofRect(capacitySliderWidth * (1.0 * capacity / MAX_CAPACITY), 1, 
			   capacitySliderWidth * (1.0 - 1.0 * capacity / MAX_CAPACITY)-1 , CAPACITY_SLIDER_HEIGHT-2);
		
		ofSetHexColor(0xFFFFFF);
		ofEnableAlphaBlending();
		frameCapacityText.draw(capacitySliderWidth/2 - frameCapacityText.width/2, CAPACITY_SLIDER_HEIGHT/2.0 - frameCapacityText.height/2);
		ofDisableAlphaBlending();
		
	} ofPopMatrix(); ofPopStyle();

	//draw bottom label
	ofTranslate(0, CAPACITY_SLIDER_HEIGHT + 22);
	bottomImage.draw(0,0);	
}

void slitScanApp::changeCapacity()
{
	if(warp.isSetup()){
		warp.setCapacity(capacity);
	}
	else{
		warp.setup(WIDTH, HEIGHT, capacity, OF_IMAGE_COLOR);
		warp.setDelayMap(*sampleMaps[currentSampleMapIndex]);
		
	}
	warp.setTimeDelayAndWidth(int( leftClamp * warp.getCapacity() + .5), 
							  int( (rightClamp - leftClamp) * warp.getCapacity() + .5) );
	
	framesToShow = MIN(warp.getCapacity(), 19);
	heightPerFrame = 49;
	widthPerFrame  = 1.0 * WIDTH / HEIGHT * heightPerFrame;
	filmStripWidth = widthPerFrame*framesToShow;
}

void slitScanApp::keyPressed(int key){
	//hot keys
	// b - blending
	// p or space bar pause
	// f - fullscreen
	// m - load movie
	// l - live video
	// v - video settings
	// numbers 1 - 9 select maps	
	if(key == 'b'){
		warp.toggleBlending();
	}
	else if(key == 'p' || key == ' '){
		isPaused = !isPaused;
	}
	else if(key == 'f'){
		isFullScreen = !isFullScreen;
		ofSetFullscreen(isFullScreen);
	}
	else if(key == 'm'){
		initMovie();
	}
	else if(key == 'l'){
		initLiveVideo();
	}
	else if(key == 'v'){
		vidGrabber.videoSettings();
	}
	else if(key == '1'){
		warp.setDelayMap(*(sampleMaps[currentSampleMapIndex = 0])); 
	}
	else if(key == '2'){
		warp.setDelayMap(*(sampleMaps[currentSampleMapIndex = 1]));
	}
	else if(key == '3'){
		warp.setDelayMap(*(sampleMaps[currentSampleMapIndex = 2]));
	}
	else if(key == '4'){
		warp.setDelayMap(*(sampleMaps[currentSampleMapIndex = 3]));
	}
	else if(key == '5'){
		warp.setDelayMap(*(sampleMaps[currentSampleMapIndex = 4]));
	}
	else if(key == '6'){
		warp.setDelayMap(*(sampleMaps[currentSampleMapIndex = 5]));
	}
	else if(key == '7'){
		warp.setDelayMap(*(sampleMaps[currentSampleMapIndex = 6]));
	}
	else if(key == '8'){
		warp.setDelayMap(*(sampleMaps[currentSampleMapIndex = 7]));
	}
	else if(key == '9'){
		if(loadCustomMapIndex()){
			currentSampleMapIndex = 8;
		}
	}
}

void slitScanApp::mouseDragged(int x, int y, int button){
	if(isDraggingLeft){
		leftClamp = CLAMP( 1.0f * (x - FRAME_PADDING-CLAMP_HANDLE_WIDTH) / filmStripWidth, 0.0f, rightClamp);
	}
	else if(isDraggingRight){
		rightClamp = CLAMP(1.0f * (x - FRAME_PADDING-CLAMP_HANDLE_WIDTH) / filmStripWidth, leftClamp, 1.0f); 
	}
	else if(isDraggingWidth){
		float difference = dragOffset - (x - CLAMP_HANDLE_WIDTH) / filmStripWidth;
		rightClamp = CLAMP(rightClampAnchor - difference, 0.0, 1.0);
		leftClamp =  CLAMP(leftClampAnchor - difference, 0.0, 1.0);
	}
	else if(isDraggingCapacity){
		capacity = getCapacitySliderPercent(x)*MAX_CAPACITY; 
	}

	warp.setTimeDelayAndWidth(int( (leftClamp * warp.getCapacity() + .5)),
							  int( (rightClamp - leftClamp) * warp.getCapacity() + .5));
	
	//redraw based on actual discrete values
	leftClamp  =  1.0 * warp.getTimeDelay() / warp.getCapacity();
	rightClamp = (1.0 * warp.getTimeDelay() + warp.getTimeWidth() ) / warp.getCapacity();
}

void slitScanApp::mousePressed(int x, int y, int button){
	
	testClickedSampleIndex(x,y);
	
	//HUD controls
	testClickedFrameBlend(x, y);
	testClickedLiveVideo(x, y);
	testClickedLoadMovie(x, y);
	testClickedFullscreen(x, y);
	testClickedPause(x, y);
	
	//scrubbing film strip
	testClickedLeftClamp(x, y);
	testClickedRightClamp(x, y);
	testClickedFilmStrip(x, y);
	
	//changing capacity
	testClickedCapacitySlider(x, y);
	
	testClickedBanner(x,y);
}

void slitScanApp::testClickedSampleIndex(int x, int y)
{
	int selectedIndex = getSelectedSampleMapIndex(x, y);
	if(selectedIndex == sampleMaps.size() && loadCustomMapIndex()){
		currentSampleMapIndex = selectedIndex;	
	}
	else if( selectedIndex >= 0 && selectedIndex != currentSampleMapIndex && selectedIndex < sampleMaps.size()) {
		warp.setDelayMap(*(sampleMaps[selectedIndex]));
		currentSampleMapIndex = selectedIndex;	
	}
}

bool slitScanApp::loadCustomMapIndex()
{
	ofFileDialogResult r = ofSystemLoadDialog("Open Warp Map", false);
	
	string mapfile = r.getPath();
	if(!r.bSuccess || mapfile == ""){
		return false;
	}
	
	customMap.loadImage(mapfile);
	if(customMap.width != WIDTH || customMap.height != HEIGHT){
		customMap.resize(WIDTH, HEIGHT);
	}
	
	warp.setDelayMap(customMap);	
	return true;
}

int slitScanApp::getSelectedSampleMapIndex(int x, int y){
	y -= FRAME_PADDING;
	x -= FRAME_PADDING;
	if( y >  0 && y < MAP_THUMB_HEIGHT && x > 0 && x < DRAW_WIDTH){
		return (int)ofMap(x, 0, sampleMaps.size()*(MAP_THUMB_WIDTH+FRAME_PADDING), 0, sampleMaps.size(), false);
	}
	return -1;
}

void slitScanApp::testClickedFrameBlend(int x, int y)
{
	x -= (VIEWPORT_WIDTH - HUD_BUTTON_WIDTH - FRAME_PADDING*2);
	y -= (FRAME_PADDING*2 + SELECTOR_HEIGHT);
	if( y > 0 && y < HUD_BUTTON_HEIGHT && x > 0 && x < HUD_BUTTON_WIDTH){
		warp.toggleBlending();
	}
}

void slitScanApp::testClickedPause(int x, int y)
{
	x -=  FRAME_PADDING*2;
	y -= (SELECTOR_HEIGHT + VIEWPORT_HEIGHT - FRAME_PADDING*3 - HUD_BUTTON_HEIGHT);
	if( y > 0 && y < HUD_BUTTON_HEIGHT && x > 0 && x < HUD_BUTTON_WIDTH){
		isPaused = !isPaused;
	}	
}

void slitScanApp::testClickedFullscreen(int x, int y)
{
	x -= (VIEWPORT_WIDTH - HUD_BUTTON_WIDTH - FRAME_PADDING*2);
	y -= (SELECTOR_HEIGHT + VIEWPORT_HEIGHT - FRAME_PADDING*3 - HUD_BUTTON_HEIGHT);
	if( y > 0 && y < HUD_BUTTON_HEIGHT && x > 0 && x < HUD_BUTTON_WIDTH){
		ofSetFullscreen(true);
		isFullScreen = true;
	}	
}

void slitScanApp::testClickedCapacitySlider(int x, int y)
{
	y -= (FRAME_PADDING + SELECTOR_HEIGHT +  //selector
		  HEIGHT + FRAME_PADDING +		 //viewport
		  heightPerFrame + FRAME_PADDING*4); //filmstrip
	if( y > 0 && y < CAPACITY_SLIDER_HEIGHT){
		isDraggingCapacity = true;
		capacity = getCapacitySliderPercent(x)*MAX_CAPACITY; 
	}
}		 

float slitScanApp::getCapacitySliderPercent(int x)
{
	return ofMap(x-FRAME_PADDING, 0, DRAW_WIDTH-FRAME_PADDING*2, 0, 1.0, true);
}

void slitScanApp::testClickedLeftClamp(int x, int y)
{
	y -= (SELECTOR_HEIGHT + VIEWPORT_HEIGHT + FRAME_PADDING);
	x -= FRAME_PADDING;
	if( x > 0 && x < DRAW_WIDTH && y > 0 && y < FRAME_PADDING ) {
		isDraggingLeft = true;
	}
}

void slitScanApp::testClickedRightClamp(int x, int y)
{
	y -= (FRAME_PADDING*2 + SELECTOR_HEIGHT + VIEWPORT_HEIGHT + heightPerFrame);
	x -= FRAME_PADDING;
	if( x > 0 && x < DRAW_WIDTH && y > 0 && y < FRAME_PADDING){
		isDraggingRight = true;
	}
}

void slitScanApp::testClickedFilmStrip(int x, int y)
{
	y -= (FRAME_PADDING*2 + SELECTOR_HEIGHT + VIEWPORT_HEIGHT );
	x -= CLAMP_HANDLE_WIDTH;
	if( x > 0 && x < filmStripWidth && y > 0 && y < heightPerFrame ) {
		//find the center
		dragOffset = x / filmStripWidth;
		leftClampAnchor = leftClamp;
		rightClampAnchor = rightClamp;
		isDraggingWidth = true;
	}	
}

void slitScanApp::testClickedLiveVideo(int x, int y)
{
	x -=  FRAME_PADDING*2;
	y -= (FRAME_PADDING*2 + SELECTOR_HEIGHT);
	if(!useLiveVideo && x > 0 && x < HUD_BUTTON_WIDTH / 2 && y > 0 && y < HUD_BUTTON_HEIGHT){
		initLiveVideo();
	}
}

void slitScanApp::testClickedLoadMovie(int x, int y)
{
	x -=  FRAME_PADDING*2 + HUD_BUTTON_WIDTH/2;
	y -= (FRAME_PADDING*2 + SELECTOR_HEIGHT);
	if(useLiveVideo && x > 0 && x < HUD_BUTTON_WIDTH / 2 && y > 0 && y < HUD_BUTTON_HEIGHT){
		initMovie();
	}
}

void slitScanApp::testClickedBanner(int x, int y)
{
	if(y > ofGetHeight() - bottomImage.getHeight()){
		ofLaunchBrowser("http://www.jamesgeorge.org/");
	}
}


void slitScanApp::mouseReleased(int x, int y, int button)
{
	isDraggingLeft = false;
	isDraggingRight = false;
	isDraggingWidth = false;
	if(isDraggingCapacity){
		changeCapacity();
		isDraggingCapacity = false;
	}
}

void slitScanApp::mouseMoved(int x, int y){}
void slitScanApp::windowResized(int x, int y){}
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

#ifndef _SLITSCAN_APP
#define _SLITSCAN_APP

#include "ofMain.h"
#include "ofxSlitScan.h"

class slitScanApp : public ofBaseApp {

  public:

	void setup();
	void update();
	void draw();

	void keyPressed(int key);
	void mouseMoved(int x, int y );
	void mouseDragged(int x, int y, int button);
	void mousePressed(int x, int y, int button);
	void mouseReleased(int x, int y, int button);
	void windowResized(int w, int h);

	ofVideoGrabber	vidGrabber;
	ofVideoPlayer	vidPlayer;
	
	ofImage	colorImg;
	ofImage movieImg;
	ofImage	warpImage;
	ofImage previewImage;
	ofImage movieImage;
	
	ofxSlitScan warp;
	int capacity;

	void changeCapacity();
	
	//sample map selector
	vector<string> sampleMapStrings;
	vector<ofImage*> sampleMaps;
	ofImage customMap;
	
	int currentSampleMapIndex;
	
	//input selection
	bool useLiveVideo;
	void initLiveVideo();
	void initMovie();
	bool isFullScreen;
	bool isPaused;
	
	//filmstrip widget
	float rightClamp;
	float leftClamp;
	
	int framesToShow;
	float heightPerFrame;
	float widthPerFrame;
	float filmStripWidth;
	
	bool isDraggingLeft;
	bool isDraggingRight;
	bool isDraggingWidth;
	float dragOffset;
	float leftClampAnchor;
	float rightClampAnchor;
	bool isDraggingCapacity;
	
	//interactivity functions
	int getSelectedSampleMapIndex(int x, int y);
	void testClickedSampleIndex(int x, int y);
	bool loadCustomMapIndex();
	void testClickedFrameBlend(int x, int y);
	void testClickedFullscreen(int x, int y);
	void testClickedPause(int x, int y);
	void testClickedCapacitySlider(int x, int y);
	void testClickedLeftClamp(int x, int y);
	void testClickedRightClamp(int x, int y);
	void testClickedFilmStrip(int x, int y);	
	void testClickedLiveVideo(int x, int y);
	void testClickedLoadMovie(int x, int y);
	void testClickedBanner(int x, int y);
	
	float getCapacitySliderPercent(int x);
	float getClampPercent(int x);
	
	//interface images
	ofImage loadYourOwn;
	ofImage buttonLiveOn;
	ofImage buttonMovieOn;
	ofImage buttonPauseOn;
	ofImage buttonPauseOff;
	ofImage buttonBlendingOn;
	ofImage buttonBlendingOff;
	ofImage buttonFullscreenOn;
	ofImage buttonFullscreenOff;
	
	ofImage leftHandle;
	ofImage rightHandle;
	ofImage frameCapacityText;
	ofImage frameCapacityBar;
	ofImage bottomImage;
	

};

#endif

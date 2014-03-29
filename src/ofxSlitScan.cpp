/**
 * 
 * The MIT License
 * 
 * Copyright (c) 2010, 2011 James George http://www.jamesgeorge.org
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
 * ofxSlitScan.h
 *
 * ofxSlitScan is a simple utility for creating slit scan
 * effects and video time delays. It works by keeping a 
 * rolling buffer of video frames that can be sampled
 * from arbitrarly via a warp map 
 *
 * For inspiring works using this effect, check out Golan Levin's page
 * on Slit Scan artworks
 *
 * http://www.flong.com/texts/lists/slit_scan/
 *
 *
 * Usage:
 *
 * First call set up with the width and height that matches the video you want to process
 * after calling setup, use any image to set the delay map 
 * then with every incoming frame call addImage();
 * Calling getOutputImage will return a reference to the warped / delayed video frame
 */

#include "ofxSlitScan.h"

#define BYTES_PER_PIXEL 3

//converts from an index (0, capacity) to the appropriate fraem in the rolling buffer
static inline int frame_index(int framepointer, int index, int capacity){ 
	framepointer += index;
    if(framepointer < capacity) {
        return framepointer;
    }
    return framepointer - capacity;
}

ofxSlitScan::ofxSlitScan()
:buffersAllocated(false) {
}

void ofxSlitScan::setup(int w, int h, int _capacity) {
    switch (BYTES_PER_PIXEL) {
		case 1:{
			type = OF_IMAGE_GRAYSCALE;
		}break;
		case 3:{
			type = OF_IMAGE_COLOR;
		}break;
		case 4:{
			type = OF_IMAGE_COLOR_ALPHA;
		}break;
		default:{
			ofLog(OF_LOG_ERROR, "ofxSlitScan Error -- Invalid image type");
			return;
		}break;
	}
    
	//clean up if reallocating
	if(buffersAllocated){
		free(delayMapPixels);
		for(int i = 0; i < capacity; i++){
			free(buffer[i]);
		}
		free(buffer);
		buffersAllocated = false;
	}
	
	width = w;
	height = h;
	capacity = _capacity;
	framepointer = 0;
	blend = false;
	timeDelay = 0;
	timeWidth = capacity;
	bytesPerFrame = width*height*BYTES_PER_PIXEL;
	delayMapPixels = (float*)calloc(w*h, sizeof(float));
	buffer = (unsigned char**)calloc(capacity, sizeof(unsigned char*));
	for(int i = 0; i < capacity; i++){
		buffer[i] = (unsigned char*)calloc(bytesPerFrame, sizeof(unsigned char));
	}
	outputImage.allocate(w, h, type);
	delayMapImage.allocate(w, h, OF_IMAGE_GRAYSCALE);
	buffersAllocated = true;
	outputIsDirty = true;
	delayMapIsDirty = true;
}

bool ofxSlitScan::isSetup(){
	return buffersAllocated;
}

void ofxSlitScan::setCapacity(int _capacity){
	if(_capacity <= 0){
		_capacity = 1;
	}
	
	if(_capacity == capacity){
		return;
	}
	
	//the new capacity is bigger
	if (capacity < _capacity) {
		buffer = (unsigned char**)realloc(buffer, _capacity*sizeof(unsigned char*));
		for(int i = capacity; i < _capacity; i++){
			buffer[i] = (unsigned char*)calloc(bytesPerFrame, sizeof(unsigned char));
		}
	}
	//the new capacity is smaller
	else {
		for( int i = _capacity; i < capacity; i++){
			free(buffer[i]);
		}
		buffer = (unsigned char**)realloc(buffer, _capacity*sizeof(unsigned char*));
		framepointer %= _capacity;
	}
	capacity = _capacity;
	outputIsDirty = true;
}

void ofxSlitScan::setDelayMap(unsigned char* pix, ofImageType type){
	switch (type) {
		case OF_IMAGE_COLOR:{
			for(int i = 0; i < width*height; i++){
				//RGB 0 - 255 ==> YUV 0.0 - 1.0
				delayMapPixels[i] = (0.299*pix[i*3] + 0.587*pix[i*3+1] + 0.114*pix[i*3+2]) / 255.0; 
			}				
		}break;
			
		case OF_IMAGE_COLOR_ALPHA:{
			for(int i = 0; i < width*height; i++){
				//RGBA 0 - 255 ==> YUV 0.0 - 1.0
				delayMapPixels[i] = (0.299*pix[i*4] + 0.587*pix[i*4+1] + 0.114*pix[i*4+2]) / 255.0;
			}				
		}break;
			
		case OF_IMAGE_GRAYSCALE:{
			for(int i = 0; i < width*height; i++){
				delayMapPixels[i] = pix[i] / 255.0;
			}	
		}break;
			
		default:{
			ofLog(OF_LOG_ERROR, "ofxSlitScan -- unsupported image map type");
		}break;
	}
    
	delayMapIsDirty = true;
	outputIsDirty = true; 
}

void ofxSlitScan::setDelayMap(float* mappix){
	//assumed monochrome float image
	for(int i = 0; i < width*height; i++){
		delayMapPixels[i] = mappix[i];
	}
	delayMapIsDirty = true;
	outputIsDirty = true; 
}

void ofxSlitScan::setDelayMap(ofBaseHasPixels& map){
    setDelayMap(map.getPixelsRef());
}

void ofxSlitScan::setDelayMap(ofPixels& map){
	if(map.getWidth() != width || map.getHeight() != height){
		ofLog(OF_LOG_ERROR,"ofxSlitScan Error -- Map dimensions do not match image dimensions. given %fx%f, need %dx%d\n", map.getWidth(), map.getHeight(), width, height);
		return;
	}
	setDelayMap(map.getPixels(), map.getImageType());
}

void ofxSlitScan::setBlending(bool _blend){
	blend = _blend;
	outputIsDirty = true;
}

void ofxSlitScan::toggleBlending(){
	blend = !blend;
	outputIsDirty = true;
}

void ofxSlitScan::addImage(unsigned char* image){
	
	//write the image into the buffer
	memcpy(buffer[framepointer], image, bytesPerFrame*sizeof(unsigned char));
	
	//increment the framepointer
	framepointer = ( (framepointer + 1) % capacity );	
	
	outputIsDirty = true;	
}
void ofxSlitScan::addImage(ofBaseHasPixels& image){
    addImage(image.getPixelsRef());
}

void ofxSlitScan::addImage(ofPixels& image){
	if(image.getImageType() != type){
		ofLog(OF_LOG_ERROR, "ofxSlitScan -- adding image of the wrong type");
		return;
	}
	addImage( image.getPixels() );
}

ofImage& ofxSlitScan::getOutputImage(){
	if(outputIsDirty){
		//calculate the new distorted image
		unsigned char* writebuffer = outputImage.getPixels();
		unsigned char* outbuffer = writebuffer;
		
		int x, y, offset, lower_offset, upper_offset, pixelIndex;
		float precise, alpha, invalpha;	
		int mapMin = capacity - timeDelay - timeWidth;// (time_delay + time_width);
		int mapMax = capacity - 1 - timeDelay;// - time_delay;
        int mapRange = mapMax - mapMin;
        int n = width * height;
        pixelIndex = 0;
        
		if(blend){
			for(int i = 0; i < n; i++) {
                //find pixel point in local reference
                precise = delayMapPixels[i] * mapRange + mapMin;
                //cast it to an integer
                offset = int(precise);
                
                //calculate alpha
                alpha = precise - offset;
                invalpha = 1 - alpha;
                
                //convert to framepointer reference point
                lower_offset = frame_index(framepointer, offset, capacity);
                upper_offset = frame_index(framepointer, offset+1, capacity);
                
                //get buffers
                unsigned char *a = buffer[lower_offset] + pixelIndex;
                unsigned char *b = buffer[upper_offset] + pixelIndex;
                
                //interpolate and set values
                for(int c = 0; c < BYTES_PER_PIXEL; c++) {
                    *outbuffer++ = (a[c]*invalpha)+(b[c]*alpha);
                }
                pixelIndex += BYTES_PER_PIXEL;
            }
		}
		else{
            pixelIndex = 0;
			for(int i = 0; i < n; i++) {
                int index = delayMapPixels[i] * mapRange + mapMin;
                index = frame_index(framepointer, index, capacity);
                // faster than memcpy because the compiler can optimize it
                for(int c = 0; c < BYTES_PER_PIXEL; c++) {
                    *outbuffer++ = buffer[index][pixelIndex + c];
                }
                pixelIndex += BYTES_PER_PIXEL;
			}
		}
		outputImage.setFromPixels(writebuffer, width, height, type);
		outputIsDirty = false;
	}

	return outputImage;
}

ofImage& ofxSlitScan::getDelayMap(){
	if(delayMapIsDirty){
		unsigned char* pix = delayMapImage.getPixels();
		for(int i = 0; i < width*height; i++){
			pix[i] = char(delayMapPixels[i]*255);
		}
		delayMapImage.setFromPixels(pix, width, height, OF_IMAGE_GRAYSCALE);
		delayMapIsDirty = false;
	}
	return delayMapImage;
}

int ofxSlitScan::getWidth(){
	return width;
}

int ofxSlitScan::getHeight(){
	return height;
}

void ofxSlitScan::pixelsForFrame(int num, unsigned char* outbuf){
	memcpy(outbuf, buffer[frame_index(framepointer, num, capacity)], bytesPerFrame*sizeof(unsigned char));
}

void ofxSlitScan::setTimeDelayAndWidth(int _timeDelay, int _timeWidth){
	timeDelay = ofClamp(_timeDelay, 0, capacity-1);
	timeWidth = ofClamp(_timeWidth, 1, capacity);
	if(timeDelay + timeWidth > capacity){
		ofLog(OF_LOG_ERROR, "ofxSlitScan -- Invalid time delay and width specified, adds to %d with a capacity of %d", (timeDelay+timeWidth), capacity);
		timeDelay = 0;
		timeWidth = capacity;
	}
	outputIsDirty = true;	
}

void ofxSlitScan::setTimeDelay(int _timeDelay){
	timeDelay = ofClamp(_timeDelay, 0, capacity - timeWidth - 1);
	outputIsDirty = true;
}

void ofxSlitScan::setTimeWidth(int _timeWidth){
	timeWidth = ofClamp(_timeWidth, 1, capacity - timeDelay);
	outputIsDirty = true;
}

int ofxSlitScan::getCapacity(){
	return capacity;
}

int ofxSlitScan::getTimeDelay(){
	return timeDelay;
}

int ofxSlitScan::getTimeWidth(){
	return timeWidth;
}

ofImageType ofxSlitScan::getType(){
	return type;
}

bool ofxSlitScan::isBlending(){	
	return blend;
}


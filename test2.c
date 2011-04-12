#include <stdio.h>
#include "cv.h"
#include "highgui.h"
 
int main(int argc, char **argv) {
    CvCapture *capture = 0;
	IplImage
		*prevFrame = 0
		, *thisFrame = 0
		, *deltaFrame = 0
		;

	int key = 0;
 
    /* initialize camera */
    capture = cvCaptureFromCAM(0);
 
    /* always check */
    if (!capture) {
        fprintf( stderr, "Cannot open initialize webcam!\n" );
        return 1;
    }
 
    /* create a window for the video */
    cvNamedWindow("result", CV_WINDOW_AUTOSIZE);

	thisFrame = cvQueryFrame(capture);
	prevFrame = cvCloneImage(thisFrame);
	deltaFrame = cvCloneImage(thisFrame);

	switch (thisFrame->depth) {
		case IPL_DEPTH_8U:
			printf("8U\n");
			break;

		case IPL_DEPTH_8S:
			printf("8S\n");
			break;

		case IPL_DEPTH_16U:
			printf("16U\n");
			break;

		case IPL_DEPTH_16S:
			printf("16S\n");
			break;

		case IPL_DEPTH_32S:
			printf("32S\n");
			break;

		case IPL_DEPTH_32F:
			printf("32F\n");
			break;

		case IPL_DEPTH_64F:
			printf("64F\n");
			break;

	}
 
    while (key != 'q') {
        /* get a frame */
        thisFrame = cvQueryFrame(capture);
 
        /* always check */
        if (!thisFrame) break;

		/* Compute the delta */
		if (thisFrame->imageSize != prevFrame->imageSize) {
			break;
		}

		for (int i = 0; i < thisFrame->imageSize; ++i) {
			deltaFrame->imageData[i] = thisFrame->imageData[i] - prevFrame->imageData[i] + 128;
		}
       
        /* display current frame */
        cvShowImage("result", deltaFrame);
 
        /* exit if user press 'q' */
        key = cvWaitKey(1);

		cvCopy(thisFrame, prevFrame);
    }
 
    /* free memory */
    cvDestroyWindow("result");
    cvReleaseCapture(&capture);
 
    return 0;
}

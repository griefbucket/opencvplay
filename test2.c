#include <stdio.h>
#include <unistd.h>

#include <zlib.h>

#include "cv.h"
#include "highgui.h"

#define DELTA_ACC_THRESHOLD 10

unsigned int def(unsigned char *in, size_t inLen) {
	size_t CHUNK = 4096;
	unsigned char out[4096];
	unsigned int written = 0;
	z_stream strm;

	strm.zalloc = Z_NULL;
	strm.zfree = Z_NULL;
	strm.opaque = Z_NULL;

	deflateInit(&strm, 6);

	strm.avail_in = inLen;
	strm.next_in = in;

	do {
		strm.avail_out = CHUNK;
		strm.next_out = out;
		deflate(&strm, Z_FINISH);
		written += CHUNK - strm.avail_out;
	}
	while (strm.avail_out == 0);

	return written;
}
 
int main(int argc, char **argv) {
    CvCapture *capture = 0;
	IplImage
		*prevFrame = 0
		, *thisFrame = 0
		, *deltaFrame = 0
		, *rebuiltFrame = 0
		;

	int key = 0;
 
    /* initialize camera */
    capture = cvCaptureFromCAM(0);
 
    /* always check */
    if (!capture) {
        fprintf(stderr, "Cannot open initialize webcam!\n");
        return 1;
    }
 
    /* create a window for the video */
    cvNamedWindow("result", CV_WINDOW_AUTOSIZE);

	thisFrame = cvQueryFrame(capture);
	prevFrame = cvCloneImage(thisFrame);
	deltaFrame = cvCloneImage(thisFrame);
	rebuiltFrame = cvCloneImage(thisFrame);

	int *allDeltaData = (int*)calloc(sizeof(int), thisFrame->imageSize);

	if (thisFrame->depth != IPL_DEPTH_8U) {
		fprintf(stderr, "Unsupported frame format");
		return 1;
	}

	if (thisFrame->dataOrder != IPL_DATA_ORDER_PIXEL) {
		fprintf(stderr, "Must use IPL_DATA_ORDER_PIXEL");
		return 1;
	}

	/* Zero out the rebuilt data! */
	for (int i = 0; i < rebuiltFrame->imageSize; ++i) {
		rebuiltFrame->imageData[i] = 0;
		prevFrame->imageData[i] = 0;
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

		unsigned char 
			*thisData = (unsigned char*) thisFrame->imageData
			, *prevData = (unsigned char*) prevFrame->imageData
			, *rebuiltData = (unsigned char*) rebuiltFrame->imageData
			, *deltaData = (unsigned char*) deltaFrame->imageData
			;

		/* Compute the exact deltas */
		for (int i = 0; i < thisFrame->imageSize; ++i) {
			allDeltaData[i] += (int)thisData[i] - (int)prevData[i];
		}

		/* Then write the actual deltas */
		for (int i = 0; i < thisFrame->imageSize; ++i) {
			int val = allDeltaData[i];

			if (val > DELTA_ACC_THRESHOLD) {
				int f = val < 127 ? val : 127;
				deltaData[i] = 127 - (unsigned char)f;
				allDeltaData[i] -= f;
			}
			else if (val < -DELTA_ACC_THRESHOLD) {
				int f = val > -127 ? val : -127;
				deltaData[i] = 127 - (unsigned char)f;
				allDeltaData[i] -= f;
			}
			else {
				deltaData[i] = 127;
			}
		}

		/* And rebuild the image from the deltas */
		for (int i = 0; i < thisFrame->imageSize; ++i) {
			int val = (int)rebuiltData[i] + 127 - (int)deltaData[i];

			if (val < 0) val = 0;
			if (val > 255) val = 255;

			rebuiltData[i] = (unsigned char)val;
		}
       
        /* display current frame */
        cvShowImage("result", rebuiltFrame);
 
        /* exit if user press 'q' */
        key = cvWaitKey(1);

		cvCopy(thisFrame, prevFrame);

		/* encode the thing hur hur */
		unsigned int size = def((unsigned char*)deltaFrame->imageData, deltaFrame->imageSize);
		fprintf(stderr, "%d\n", size);
    }
 
    /* free memory */
    cvDestroyWindow("result");
    cvReleaseCapture(&capture);
 
    return 0;
}

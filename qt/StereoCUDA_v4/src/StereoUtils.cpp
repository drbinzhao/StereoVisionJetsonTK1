/*
 * StereoUtils.cpp
 *
 *  Created on: Dec 16, 2015
 *      Author: nicolasrosa
 */

/* Libraries */
#include "inc/StereoUtils.h"

/* Constructor and Destructor */
StereoUtils::Timer::Timer(){}
StereoUtils::Timer::Timer::~Timer(){}


void StereoUtils::Timer::startClock(){
    clockInitial = getTickCount();
}


void StereoUtils::Timer::stopClock(){
    clockFinal = getTickCount();
}


void StereoUtils::Timer::calculateFPS(){
    d = clockFinal - clockInitial;	//Ticks Difference
    f = getTickFrequency();			//CPU Ticks per Second
    fps = f/d;						//Frames per Second
}

void StereoUtils::Timer::calculateElapsedTime(){
    d = clockFinal - clockInitial;	// Ticks Difference
    f = getTickFrequency();			// CPU Ticks per Second
    elapsedTime = d/f;				// Elapsed Time in Seconds
}

void StereoUtils::Timer::printElapsedTime(){
    calculateElapsedTime();
    cout << "t: " << elapsedTime << endl;
}


void StereoUtils::Timer::printFPS(){
    calculateFPS();
    cout << "FPS: " << fps << endl;
}


double StereoUtils::Timer::getFPS() const{
    return fps;
}


void StereoUtils::Extras::writeMatToFile(cv::Mat& m, const char* filename){
    ofstream fout(filename);

    if(!fout){
        cout<<"File Not Opened"<<endl;  return;
    }

    for(int i=0; i<m.rows; i++){
        for(int j=0; j<m.cols; j++){
            fout<<m.at<float>(i,j)<<"\t";
        }
        fout<<endl;
    }

    fout.close();
}


void StereoUtils::Resizer::resizeFrames(Mat* frame1,Mat* frame2,Size resolution){
    if(frame1->cols != 0 || !frame2->cols != 0){
        resize(*frame1, *frame1, resolution, 0, 0, INTER_CUBIC);
        resize(*frame2, *frame2, resolution, 0, 0, INTER_CUBIC);
    }
}


void StereoUtils::Resizer::changeResolution(VideoCapture* capL,VideoCapture* capR,Size resolution){
    capL->set(CV_CAP_PROP_FRAME_WIDTH, resolution.width);
    capL->set(CV_CAP_PROP_FRAME_HEIGHT,resolution.height);
    capR->set(CV_CAP_PROP_FRAME_WIDTH, resolution.width);
    capR->set(CV_CAP_PROP_FRAME_HEIGHT,resolution.height);

    cout << "Camera 1 Resolution: " << capL->get(CV_CAP_PROP_FRAME_WIDTH) << "x" << capL->get(CV_CAP_PROP_FRAME_HEIGHT) << endl;
    cout << "Camera 2 Resolution: " << capR->get(CV_CAP_PROP_FRAME_WIDTH) << "x" << capR->get(CV_CAP_PROP_FRAME_HEIGHT) << endl;

}


void StereoUtils::Extras::contrast_and_brightness(Mat &left,Mat &right,float alpha,float beta){
    //Contrast and Brightness. Do the operation: new_image(i,j) = alpha*image(i,j) + beta
    for( int y = 0; y < left.rows; y++ ){
        for( int x = 0; x < left.cols; x++ ){
            for( int c = 0; c < 3; c++ ){
                left .at<Vec3b>(y,x)[c] = saturate_cast<uchar>( alpha*( left .at<Vec3b>(y,x)[c] ) + beta );
                right.at<Vec3b>(y,x)[c] = saturate_cast<uchar>( alpha*( right.at<Vec3b>(y,x)[c] ) + beta );
            }
        }
    }
}


void StereoUtils::Extras::calculateHist(Mat &src,const string histName){
    // Establish the number of bins
    int histSize = 256;

    // Set the ranges ( for B,G,R) )
    float range[] = { 0, 256 } ;
    const float* histRange = { range };

    bool uniform = true; bool accumulate = false;

    // Draw the histograms for GreyScale
    int hist_w = 512; int hist_h = 400;
    int bin_w = cvRound( (double) hist_w/histSize );

    Mat histImage( hist_h, hist_w, CV_8UC3, Scalar( 0,0,0) );

    // 8-bits unsigned, NO. OF CHANNELS=1
    if(src.type()==CV_8UC1){
        Mat grey_hist;

        /// Compute the histograms:
        calcHist( &src, 1, 0, Mat(), grey_hist, 1, &histSize, &histRange, uniform, accumulate );

        Mat histImage( hist_h, hist_w, CV_8UC3, Scalar( 0,0,0) );

        // Normalize the result to [ 0, histImage.rows ]
        normalize(grey_hist, grey_hist, 0, histImage.rows, NORM_MINMAX, -1, Mat() );

        // Draw for each channel
        for( int i = 1; i < histSize; i++ ){
          line( histImage, Point( bin_w*(i-1), hist_h - cvRound(grey_hist.at<float>(i-1)) ) ,
                           Point( bin_w*(i), hist_h - cvRound(grey_hist.at<float>(i)) ),
                           Scalar( 255, 255, 255), 2, 8, 0  );
        }

        // Display
        namedWindow(histName, WINDOW_AUTOSIZE );
        imshow(histName, histImage );
    }

    // 8-bits unsigned, NO. OF CHANNELS=3
    if(src.type()==CV_8UC3){
        // Separate the image in 3 places ( B, G and R )
        vector<Mat> bgr_planes;
        split( src, bgr_planes );

        Mat b_hist, g_hist, r_hist;

        /// Compute the histograms:
        calcHist( &bgr_planes[0], 1, 0, Mat(), b_hist, 1, &histSize, &histRange, uniform, accumulate );
        calcHist( &bgr_planes[1], 1, 0, Mat(), g_hist, 1, &histSize, &histRange, uniform, accumulate );
        calcHist( &bgr_planes[2], 1, 0, Mat(), r_hist, 1, &histSize, &histRange, uniform, accumulate );

        // Normalize the result to [ 0, histImage.rows ]
        normalize(b_hist, b_hist, 0, histImage.rows, NORM_MINMAX, -1, Mat() );
        normalize(g_hist, g_hist, 0, histImage.rows, NORM_MINMAX, -1, Mat() );
        normalize(r_hist, r_hist, 0, histImage.rows, NORM_MINMAX, -1, Mat() );

        // Draw for each channel
        for( int i = 1; i < histSize; i++ ){
          line( histImage, Point( bin_w*(i-1), hist_h - cvRound(b_hist.at<float>(i-1)) ) ,
                           Point( bin_w*(i), hist_h - cvRound(b_hist.at<float>(i)) ),
                           Scalar( 255, 0, 0), 2, 8, 0  );
          line( histImage, Point( bin_w*(i-1), hist_h - cvRound(g_hist.at<float>(i-1)) ) ,
                           Point( bin_w*(i), hist_h - cvRound(g_hist.at<float>(i)) ),
                           Scalar( 0, 255, 0), 2, 8, 0  );
          line( histImage, Point( bin_w*(i-1), hist_h - cvRound(r_hist.at<float>(i-1)) ) ,
                           Point( bin_w*(i), hist_h - cvRound(r_hist.at<float>(i)) ),
                           Scalar( 0, 0, 255), 2, 8, 0  );
        }

        // Display
        namedWindow(histName, WINDOW_AUTOSIZE );
        imshow(histName, histImage );
    }
}


string StereoUtils::Extras::intToString(int number){
    stringstream ss;
    ss << number;
    return ss.str();
}

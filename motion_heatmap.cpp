#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/videoio/videoio.hpp>
#include <opencv2/objdetect/objdetect.hpp>
#include <opencv2/imgcodecs/imgcodecs.hpp>
#include <opencv2/video.hpp>
#include <iostream>
#include <ctime>
#include "opencv2/video/background_segm.hpp"

using namespace std;
using namespace cv;

void main()
{
	//Capturing video
	int option;
	string vid_file;
	VideoCapture cap;
	int nFrames;
	cout<<"Select your option for input"<<endl<<" Camera -> 1 \n Video-File -> 2 \n";
	cin>>option;
	if(option == 1)
	cap.open(0);
	else if(option == 2)
	{
		cout<<"Enter video file name (with extension): "; // input video should be on same worksapce folder
		cin>>vid_file;
		cap.open(vid_file);
		nFrames = cap.get(CV_CAP_PROP_FRAME_COUNT);
	}
	else
		cout<<"enter valid option !"<<endl;

	if(!cap.isOpened())
	{
		cout << "Error opening video stream or file" << endl;
		exit(0);
	}

	// Default resolution of the frame is obtained.The default resolution is system dependent.
	int frame_width = cap.get(CV_CAP_PROP_FRAME_WIDTH) , frame_height = cap.get(CV_CAP_PROP_FRAME_HEIGHT);

	//Video saving APIs
	VideoWriter video2("heatmap_video.avi",CV_FOURCC('M','J','P','G'),10, Size(frame_width,frame_height));

	//begsegmentation pointer initialised
	Ptr<BackgroundSubtractorMOG2> background_segmentor_object_file = createBackgroundSubtractorMOG2();
	int first_iteration_indicator=1,i=0,c=1;
	Mat gray,accum_image,first_frame,frame,frame_dup,fgbgmask,threshold_image,color_image,result_overlay,accum_image_duplicate,color_image1,duplicate_final,result_overlay_video,final_img;
	while(cap.isOpened())
	{
		cout << "No. of Iterations completed :" << i << endl;

		if(c==(nFrames-1))
		{

			break;
		}

		cap.read(frame);
		if (first_iteration_indicator == 1)
		{
			//Saving the first frame to overlap all the accumilated colored segmentation's on this frame
			first_frame = frame.clone();
			//Converting to Grayscale
			cvtColor(frame,gray, COLOR_BGR2GRAY);
			//accum_image frame dimension initialisation
			Mat accum_image = Mat(frame_height,frame_width, CV_64F, double(0));
			first_iteration_indicator=0;
		}
		else
		{
			frame_dup = frame.clone();
			//Converting to Grayscale
			cvtColor(frame,gray, COLOR_BGR2GRAY);
			//Remove the background
			background_segmentor_object_file ->apply(gray,fgbgmask);
			//Thresholding the image
			int thres=2,maxValue=2;
			threshold(fgbgmask,threshold_image,thres,maxValue,CV_THRESH_BINARY);
			//Adding to the accumilated image
			accum_image = threshold_image+accum_image;
			//Saving the accumilated image onto a duplicate frame to plot a live heatmap
			accum_image_duplicate = accum_image;
			cv::applyColorMap(accum_image_duplicate, color_image1, COLORMAP_HOT);
			addWeighted(frame, 0.5, color_image1, 0.5,0.0, result_overlay_video);
			//Heatmap video generation
			video2.write(result_overlay_video);
		}
				//window to display message
				char msg[100];
				Mat pop_up = Mat::zeros(250, 750, CV_8UC3);
				sprintf(msg," Motion Heatmap Generating..");
				putText(pop_up,msg,Point2f(100,100),  FONT_HERSHEY_SIMPLEX,1,Scalar(255,0,0),2,true);
				sprintf(msg," Press q to quit !!!");
				putText(pop_up,msg,Point2f(100,200),  FONT_HERSHEY_SIMPLEX,1,Scalar(0,0,255),2,true);
				imshow("NOTE !!!", pop_up);

				int key = (waitKey(30) & 0xFF);
				if(key == 'q'||key == 'Q')
				{
					cout<<"Exiting at iteration : "<<i<<endl;
					break;
				}
		i++,c++;
		frame.release();
	}
	//Adding all accumilated frames to the first frame
	cv::applyColorMap(accum_image, color_image, COLORMAP_HOT);
	double alpha = 0.5; double beta;
	beta = ( 1.0 - alpha );
	//addWeighted( src1, alpha, src2, beta, 0.0, dst);
	addWeighted( first_frame, alpha, color_image, beta,0.0, result_overlay);
	//	imshow("RESULT_OVERLAY",result_overlay);
	imwrite( "result_overlay_final.jpg",result_overlay);

	// When everything done, release the video capture and write object
	cout<<" Heatmap video & Result_overlay_image are saved on workspace Folder"<<endl;
	cap.release();
	// Closes all the windows
	destroyAllWindows();

}

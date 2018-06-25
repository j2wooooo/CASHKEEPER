// ���� ��ũ : https://docs.opencv.org/2.4/doc/tutorials/objdetect/cascade_classifier/cascade_classifier.html
// ���� ��ũ �״�� �ϸ�
// capture = cvCaptureFromCAM(-1); ���� 
// capture�� Mat�����̸� �ȵǰ� IplImage* �����̿�����-> �ٸ��� cam �о���°ɷ� �����ϰ���

#include "/home/nvidia/opencv/modules/objdetect/include/opencv2/objdetect.hpp"
//#include "/home/nvidia/opencv/modules/highgui/include/opencv2/highgui.hpp"
#include "/home/nvidia/opencv/modules/imgproc/include/opencv2/imgproc/imgproc.hpp"
#include "/home/nvidia/opencv/include/opencv2/opencv.hpp"
#include "/home/nvidia/NVIDIA_CUDA-8.0_Samples/6_Advanced/interval/boost/config/no_tr1/cmath.hpp" 
// I added it, cause when I compile this code ther was an  error "undefined reference to symbol 'lrint@@GLIBC_2.17' //lib..... 

#include <iostream>
#include <stdio.h>

using namespace std;
using namespace cv;

/** Function Headers */
void detectAndDisplay(Mat frame);

/** Global variables */
String face_cascade_name = "/home/nvidia/opencv/data/haarcascades_cuda/haarcascade_frontalface_alt.xml";
String eyes_cascade_name = "/home/nvidia/opencv/data/haarcascades_cuda/haarcascade_eye_tree_eyeglasses.xml";
String mouth_cascade_name = "/home/nvidia/opencv/data/haarcascades_cuda/haarcascade_mcs_mouth.xml";
CascadeClassifier face_cascade;
CascadeClassifier eyes_cascade;
CascadeClassifier mouth_cascade;
string window_name = "Capture - Face detection";
RNG rng(12345);

std::string get_tegra_pipeline(int width, int height, int fps) {
	return "nvcamerasrc ! video/x-raw(memory:NVMM), width=(int)" + std::to_string(width) + ", height=(int)" 
		+ std::to_string(height) + ", format=(string)I420, framerate=(fraction)" + std::to_string(fps) 
		+ "/1 ! nvvidconv flip-method=0 ! video/x-raw, format=(string)BGRx ! videoconvert ! video/x-raw, format=(string)BGR ! appsink";
}

/** @function main */
int main(int argc, const char** argv)
{
	//CvCapture* capture;

	/* ķ �ҷ����� */
	// Options
	int WIDTH = 1920;
	int HEIGHT = 1080;
	int FPS = 30;

	// Define the gstream pipeline
	std::string pipeline = get_tegra_pipeline(WIDTH, HEIGHT, FPS);
	std::cout << "Using pipeline: \n\t" << pipeline << "\n";

	// Create OpenCV capture object. ensure it works.
	cv::VideoCapture capture(pipeline, cv::CAP_GSTREAMER);
	if (!capture.isOpened()) {
		std::cout << "Connection failed";
		return -1;
	}

	Mat frame;

	//-- 1. Load the cascades
	if (!face_cascade.load(face_cascade_name)) { printf("--(!)Error loading\n"); return -1; };
	if (!eyes_cascade.load(eyes_cascade_name)) { printf("--(!)Error loading\n"); return -1; };
	if (!mouth_cascade.load(mouth_cascade_name)) { printf("--(!)Error loading\n"); return -1; };

	//-- 2. Read the video stream
	//capture = cvCaptureFromCAM(-1);
	while(1)
	{
		//frame = cvQueryFrame(capture);
		capture >> frame;

		//-- 3. Apply the classifier to the frame
		if (!frame.empty())
		{
			detectAndDisplay(frame);
		}
		else
		{
			printf(" --(!) No captured frame -- Break!"); break;
		}

		int c = waitKey(10);
		if ((char)c == 'c') { break; }
	}

	return 0;
}

/** @function detectAndDisplay */
void detectAndDisplay(Mat frame)
{
	std::vector<Rect> faces;
	Mat frame_gray;

	cvtColor(frame, frame_gray, CV_BGR2GRAY);
	equalizeHist(frame_gray, frame_gray);

	//-- Detect faces
	face_cascade.detectMultiScale(frame_gray, faces, 1.1, 2, CASCADE_FIND_BIGGEST_OBJECT | CV_HAAR_SCALE_IMAGE, Size(30, 30));

	for (size_t i = 0; i < faces.size(); i++)
	{
		Point center(faces[i].x + faces[i].width*0.5, faces[i].y + faces[i].height*0.5);
		ellipse(frame, center, Size(faces[i].width*0.5, faces[i].height*0.5), 0, 0, 360, Scalar(255, 0, 255), 4, 8, 0);

		Mat faceROI = frame_gray(faces[i]);
		std::vector<Rect> eyes;
		std::vector<Rect> mouth;

		//-- In each face, detect eyes
		eyes_cascade.detectMultiScale(faceROI, eyes, 1.1, 3, 0 | CV_HAAR_SCALE_IMAGE, Size(30, 30));
		mouth_cascade.detectMultiScale(faceROI, mouth, 1.1, 5, 0 | CV_HAAR_SCALE_IMAGE , Size(80,80)); 

		for (size_t j = 0; j < eyes.size(); j++)
		{
			Point center(faces[i].x + eyes[j].x + eyes[j].width*0.5, faces[i].y + eyes[j].y + eyes[j].height*0.5);
			int radius = cvRound((eyes[j].width + eyes[j].height)*0.25);
			circle(frame, center, radius, Scalar(255, 0, 0), 4, 8, 0);
		}

		for(size_t k=0; k<mouth.size(); k++)
		{

			//Point center(faces[i].x + mouth[k].x + eyes[k].width*0.5, faces[i].y + mouth[k].y + mouth[k].height*0.5);
			//int radius = cvRound((mouth[k].width + mouth[k].height)*0.25);
			//circle(frame, center, radius, Scalar(255,255,255),4,8,0);

			Point pt1(mouth[0].x+faces[i].x , mouth[0].y+faces[i].y);
        		Point pt2(pt1.x+mouth[0].width, pt1.y+mouth[0].height);
        		rectangle(frame, pt1,pt2,Scalar(255,255,255),4,8,0);
		}
	}
	//-- Show what you got
	imshow(window_name, frame);
}



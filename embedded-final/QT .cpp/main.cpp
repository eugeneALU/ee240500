#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <termios.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <stdbool.h>
#include <limits.h>
#include <sys/select.h>
#include <iostream>
#include <math.h>
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/objdetect/objdetect.hpp"
#include "opencv2/core/core.hpp"
#include "opencv.hpp"
#include "opencv2/core/core_c.h"

#define left 4
#define right 6
#define forward 8
#define back 2
#define stop 5

using namespace cv;
using namespace std;

int xbee (char);
void clean_buffer ();


int main()
{
    Mat frame,skinImg, nframe,thresholdImg;
    double largest_area;
    int largest_contour_index;
    int dir=stop;
    int serial_d;
    speed_t speed;
    struct termios soptions, soptions_org;

    serial_d = open("/dev/ttyUSB0", O_RDWR | O_NOCTTY);
    if (serial_d == -1) cout<<"error";

    // ----- Begin of setup serial ports
    tcgetattr(serial_d, &soptions_org);
    tcgetattr(serial_d, &soptions);

    speed = B9600; // Speed options: B19200, B38400, B57600, B115200
    cfsetispeed(&soptions, speed);
    cfsetospeed(&soptions, speed);

    // Enable the reciver and set local mode...
    soptions.c_cflag |= ( CLOCAL | CREAD );
    // Setting Parity Checking (8N1)
    soptions.c_cflag &= ~PARENB;
    soptions.c_cflag &= ~CSTOPB;
    soptions.c_cflag &= ~CSIZE;
    soptions.c_cflag |= CS8;

    // Local setting
    //soptions.c_lflag = (ICANON | ECHO | ECHOE); //canonical
    soptions.c_lflag =  ~(ICANON | ECHO | ECHOE | ISIG); //noncanonical

    // Input setting
    //soptions.c_iflag |= (IXON | IXOFF | IXANY); //software flow control
    soptions.c_iflag |= (INPCK | ISTRIP);
    soptions.c_iflag = IGNPAR;

    // Output setting
    soptions.c_oflag = 0;
    soptions.c_oflag &= ~OPOST;

    // Read options
    soptions.c_cc[VTIME] = 0;
    soptions.c_cc[VMIN] = 1; //transfer at least 1 character or block

    // Apply setting
    tcsetattr(serial_d, TCSANOW, &soptions);
    // ----- End of setup serial ports

    VideoCapture cap(0);

    if (!cap.isOpened()) cout<<"open failed";

    for (;;)
    {
        vector<vector<Point> > contours;
        vector<Vec4i> hierarchy;

        if (!cap.isOpened()) cout<<"open failed";
        cap >> frame;

        blur(frame, frame, Size(3, 3));

        cvtColor(frame, nframe, CV_BGR2YCrCb);
        inRange(nframe,Scalar(80, 135, 85), cv::Scalar(255, 180, 135), skinImg);
        erode(skinImg, skinImg, Mat(), Point(-1, -1), 3);
        dilate(skinImg, skinImg, Mat(), Point(-1, -1), 3);
        threshold(skinImg, thresholdImg,0, 150, CV_THRESH_BINARY);
        findContours (thresholdImg, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE , Point());

        largest_area=0;
        largest_contour_index=0;
        for( int i = 0; i< contours.size(); ++i ) // iterate through each contour.
        {
            double a=contourArea( contours[i],false);  //  Find the area of contour
            if(a>largest_area){
                largest_area=a;
                largest_contour_index=i;                //Store the index of largest contour
            }
        }

        drawContours( frame, contours,largest_contour_index,  Scalar (255,0,255), 1, 8, hierarchy); // Draw the largest contour using previously stored index.


        vector<vector<Point> >hull(contours.size());
        vector<vector<int> >hulli(contours.size());
        vector<vector<Vec4i> > defects(contours.size());
        for (int i = 0; i < contours.size(); ++i)
        {
            convexHull(contours[largest_contour_index], hull[largest_contour_index], false);
            convexHull(contours[largest_contour_index], hulli[largest_contour_index], false);
            if(hulli[largest_contour_index].size() > 3 ){
                convexityDefects(contours[largest_contour_index],hulli[largest_contour_index],defects[i]);
            }
        }


        for (int i =0; i < defects.size(); ++i)
        {
            for(const Vec4i& v : defects[i])
            {
                float depth = v[3] / 256;
                if (depth > 5 ) {
                    int startidx = v[0]; Point start(contours[largest_contour_index][startidx]);
                    int endidx = v[1]; Point end(contours[largest_contour_index][endidx]);
                    int faridx = v[2]; Point far(contours[largest_contour_index][faridx]);
                    double x = sqrt(pow((end.x - start.x),2) + pow((end.y - start.y),2));
                    double y = sqrt(pow((far.x - start.x),2) + pow((far.y - start.y),2));
                    double z = sqrt(pow((end.x - far.x),2) + pow((end.y - far.y),2));
                    double angle = acos(((pow(y,2) + pow(z,2) - pow(x,2))/(2*y*z)) * 57);
                    if (angle <= 90){
                        circle(frame,far,4, Scalar(255, 255, 255), 2);
                    }
                    circle(frame,  start, 4, Scalar(0, 0, 255), 2);

                    if(start.x > 170 && start.x < 220 ){
                        if(start.y >115 && start.y < 215){
                            dir = left;
                            break;
                        }
                    }
                    else if(start.x > 295 && start.x < 345 ){
                        if(start.y >40 && start.y < 90){
                            dir = forward;
                            break;
                        }
                        else if(start.y >290 && start.y < 340 ){
                            dir = back;
                            break;
                        }
                    }
                    else if(start.x > 420 && start.x < 470 ){
                        if(start.y >165 && start.y < 215){
                            dir = right;
                            break;
                        }
                    }
                    else {dir=stop;}
                }
            }
            if (dir!=stop){break;}

        }


        rectangle(frame, Point(170, 165), Point(220,215), Scalar(0, 0, 255), 4, 8, 0);//left
        rectangle(frame, Point(295,40), Point(345, 90), Scalar(255, 0, 0),  4, 8, 0);//forward
        rectangle(frame, Point(420, 165), Point(470,215), Scalar(0, 0, 255),  4, 8, 0);//right
        rectangle(frame, Point(295,290), Point(345,340), Scalar(255, 0, 0),  4, 8, 0);//back
        if (dir == forward) {
            putText(frame,"forward",Point (10,400), CV_FONT_HERSHEY_COMPLEX, 4,Scalar(0,255,0),2,CV_AA);
            xbee('f');

        }
        else if (dir == back) {
            putText(frame,"back",Point (10,400), CV_FONT_HERSHEY_COMPLEX, 4,Scalar(0,255,0),2,CV_AA);
            xbee('b');

        }
        else if (dir == right) {
            putText(frame,"right",Point (10,400), CV_FONT_HERSHEY_COMPLEX, 4,Scalar(0,255,0),2,CV_AA);
            xbee('r');

        }
        else if (dir == left) {
            putText(frame,"left",Point (10,400), CV_FONT_HERSHEY_COMPLEX, 4,Scalar(0,255,0),2,CV_AA);
            xbee('l');

        }
        else {
            putText(frame,"stop",Point (10,400), CV_FONT_HERSHEY_COMPLEX, 4,Scalar(0,255,0),2,CV_AA);
            xbee('s');
        }


        imshow("final", frame );
        waitKey(50);

    }
    cap.release();
    return 0;
}

#define BUFLEN (255)
#define ZDEBUG (1)

void clean_buffer () {
    fflush(stdin);
}

int xbee (char com) {
    int serial_d;
    char command;
    int sent_c;

    serial_d = open("/dev/ttyUSB0", O_RDWR | O_NOCTTY);

    printf("Enter command ('q' to exit): ");
    command = com;
    clean_buffer();
    printf("Sending command '%c'...\n", command);
    sent_c = write(serial_d, &command, 1); // Send command
    tcdrain(serial_d);
    return 0;
}


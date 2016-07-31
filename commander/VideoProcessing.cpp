#include "VideoProcessing.h"

#include "opencv2/core/core.hpp"
#include "opencv2/imgproc/imgproc.hpp"

using namespace cv;


CVideoProcessing::CVideoProcessing()
{
}


void CVideoProcessing::process (void * data, unsigned int height, unsigned int width)
{
    Mat cam (height,width, CV_8UC3, data);
    Mat camgray;
    Mat edges;

    Mat req (cam.rows,cam.cols,CV_8UC1);
    Mat geq (cam.rows,cam.cols,CV_8UC1);
    Mat beq (cam.rows,cam.cols,CV_8UC1);
    Mat cameq [] = {req, geq, beq};
    int from_toeq [] = {0,0 , 1,1 , 2,2};

    mixChannels(&cam,1,cameq,3, from_toeq, 3);

    //equalizeHist(req,req);
    //equalizeHist(geq,geq);
    //equalizeHist(beq,beq);

    mixChannels(cameq,3,&cam,1, from_toeq, 3);

    cvtColor(cam, camgray, CV_RGB2GRAY);

    Mat imgHsv;
    cvtColor (cam,imgHsv, CV_RGB2HSV);




    int from_to[] = {0, 0, 1 ,1};
    Mat imgHs (cam.size(), CV_8UC2);
    mixChannels(&imgHsv,1,&imgHs,1, from_to, 2);
    Rect gndRoi;
    gndRoi.height = 50;
    gndRoi.width = 50;
    gndRoi.x = (640/2) - (gndRoi.width /2);
    gndRoi.y = (480 - gndRoi.height);

    Mat gndSamp (imgHs,gndRoi);


    uint8_t minh = 255;
    uint8_t mins = 255;
    uint8_t maxh = 0;
    uint8_t maxs = 0;

    //uint8_t * data = gndSamp.data();
    for (int i = 0; i < gndSamp.rows; i++)
    {
        uint8_t * data = &gndSamp.data[i*gndSamp.step];
        for (int j = 0; j < gndSamp.cols; j++)
        {
            size_t sz = gndSamp.elemSize();
            uint8_t h = data[j*gndSamp.elemSize()];
            uint8_t s = data[j*gndSamp.elemSize()+1];
            if (h > maxh)
            {
                maxh = h;
            }
            if (s > maxs)
            {
                maxs = s;
            }
            if (h < minh)
            {
                minh = h;
            }
            if (s < mins)
            {
                mins = s;
            }
        }
    }





    inRange(imgHs,Scalar(minh,mins),Scalar(maxh,maxs),edges);

  #if 0


    Canny(camgray,edges,50,100);

    inRange(camgray,Scalar(0,0,0,0),Scalar(100,100,100,255),edges);
    Mat str_el =getStructuringElement(MORPH_ELLIPSE, Size(7,7));
    //morphologyEx(edges,edges,MORPH_OPEN,str_el);
    //morphologyEx(edges,edges,MORPH_CLOSE,str_el);


    vector<vector<Point> > contours;
    vector<Vec4i> hierarchy;
    findContours( edges, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0, 0) );
    vector<RotatedRect> r_rect;
    float area = 0;
    float new_area;
    float trackidx = 0;
    for (int j = 0; j < contours.size(); j++)
    {
        r_rect.push_back (minAreaRect(contours[j]));
        Point2f vertices[4];
        r_rect[j].points(vertices);
        new_area = r_rect[j].size.width * r_rect[j].size.height;
        if (new_area > area)
        {
            trackidx = j;
            area = new_area;
        }

        for (int i = 0; i < 4; i++)
        {
            line (cam,vertices[i],vertices[(i+1)%4],Scalar(0,254,0));
        }
    }
#endif


    Mat edgescol(edges.rows,edges.cols,CV_8UC3);
    cvtColor(edges,edgescol, CV_GRAY2RGB);

    for (int i = 0; i < edgescol.rows; i++)
    {
        for (int j = 0; j < edgescol.cols; j++)
        {
            if (edgescol.at<Vec3b>(i,j)!= Vec3b(0,0,0))
            {
                edgescol.at<Vec3b>(i,j) = Vec3b(255,0,0);
            }
        }
    }

    cam = cam * 0.5 + edgescol * 0.5;

    rectangle(cam,gndRoi,Scalar(0,254,0));

    std::stringstream ss;
    ss << "min: " << (int)minh << " / " << (int)mins << "   max: " << (int)maxh << " / " << (int)maxs;
    putText(cam, ss.str(), Point(10, 60), FONT_HERSHEY_PLAIN, 1.0, CV_RGB(0,0,255), 1.0);
}

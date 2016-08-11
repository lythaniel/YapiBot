/*
 * ImageProcessing.cpp
 *
 * Copyright (C) 2016 Cyrille Potereau
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */

#include "ImageProcessing.h"
#include "Camera.h"
#include "VideoStreamer.h"
#include "Thread.h"
#include "Graphics.h"
#include "opencv2/core/core.hpp"
#include "opencv2/imgproc/imgproc.hpp"



using namespace cv;

CImageProcessing::CImageProcessing() :
m_pCamera(NULL),
m_pVideoStream (NULL),
m_Width (0),
m_Height (0),
m_Framerate (0),
m_pVideoProcThread(NULL),
m_OutputFrameBuffer(NULL)
{
}

void CImageProcessing::init(uint32_t width, uint32_t height,uint32_t framerate)
{
	m_Width = width;
	m_Height = height;
	m_Framerate = framerate;
	m_OutputFrameBuffer = new uint8_t [m_Width*m_Height*4]; //ARGB buffer. 4bytes per pixel.
	m_pCamera = StartCamera(m_Width, m_Height,m_Framerate,1,true);
	m_pVideoStream = new CVideoStreamer(m_Width, m_Height, m_Framerate);
	m_pVideoProcThread = new CThread (NULL);
	m_pVideoProcThread->regThreadProcess(this,&CImageProcessing::VideoProcThread);
	m_pVideoProcThread->start();

	//init graphics and the camera
	//InitGraphics();

}

CImageProcessing::~CImageProcessing()
{
	 if (m_pVideoProcThread != NULL)
	 {
		 m_pVideoProcThread->kill();
		 delete m_pVideoProcThread;
	 }
	 if(m_OutputFrameBuffer != NULL)
	 {
		 delete m_OutputFrameBuffer;
	 }
	 if(m_pCamera != NULL)
	 {
		 StopCamera();
	 }
	 if (m_pVideoStream)
	 {
		delete m_pVideoStream;
	 }
}


void CImageProcessing::VideoProcThread (void *)
{
	//create 4 texture
	//GfxTexture textures;
	//textures.Create(MAIN_TEXTURE_WIDTH,MAIN_TEXTURE_HEIGHT);
	const void  * frameptr;
	uint32_t framesz;
	uint8_t * framedata;

	while(1)
	{

		//lock the chosen frame buffer, and copy it directly into the corresponding open gl texture
		if (m_pCamera != NULL)
		{
			if(m_pCamera->BeginReadFrame(0,(const void  * *)&framedata,(int32_t *)&framesz))
			{
				//printf ("Camera frame readed\n");
	#if 0
				//if doing argb conversion the frame data will be exactly the right size so just set directly
				//Mat img (frame_sz,CV_8UC1,frame_data);
				Mat img (MAIN_TEXTURE_HEIGHT, MAIN_TEXTURE_WIDTH, CV_8UC4, (void*) frame_data);
				Mat imgresize;
				resize(img, imgresize, Size(), 0.25, 0.25);
				Mat imggray;
				Mat edges;
				cvtColor(imgresize, imggray, CV_RGBA2GRAY);

				//Canny(imggray,edges,50,100);
				inRange(imgresize,Scalar(0,0,0,0),Scalar(100,100,100,255),edges);
				Mat str_el =getStructuringElement(MORPH_ELLIPSE, Size(7,7));
				morphologyEx(edges,edges,MORPH_OPEN,str_el);
				morphologyEx(edges,edges,MORPH_CLOSE,str_el);

				//cvtColor(edges, imgresize,  CV_GRAY2RGBA,4);
				//resize(imgresize, img, img.size(), 4, 4);
				vector<vector<Point> > contours;
				vector<Vec4i> hierarchy;
				findContours( edges, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0, 0) );
				vector<RotatedRect> r_rect;
				float area = 0;
				float new_area;
				float trackidx = 0;
				for (int32_t j = 0; j < contours.size(); j++)
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
					for (int32_t i = 0; i < 4; i++)
					{
						vertices[i].x =vertices[i].x * 4;
						vertices[i].y =vertices[i].y * 4;
					}
					for (int32_t i = 0; i < 4; i++)
					{
						line (img,vertices[i],vertices[(i+1)%4],Scalar(0,254,0));
					}
				}
				std::string str ("No track detected ...");
				std::stringstream ss;
				Point2f pos;
				double angle;
				if (r_rect.size() > 0)
				{	//double angle;
					//int32_t pos;
					Point2f vertices[4];
					r_rect[trackidx].points(vertices);
					Point2f corner_low_0, corner_low_1;
					if (vertices[0].y > vertices[1].y)
					{
						corner_low_0 = vertices[0];
						corner_low_1 = vertices[1];
					}
					else
					{
						corner_low_0 = vertices[1];
						corner_low_1 = vertices[0];
					}
					if (vertices[2].y > corner_low_0.y)
					{
						corner_low_1 = corner_low_0;
						corner_low_0 = vertices[2];
					}
					else if (vertices[2].y > corner_low_1.y)
					{
						corner_low_1 = vertices[2];
					}
					if (vertices[3].y > corner_low_0.y)
					{
						corner_low_1 = corner_low_0;
						corner_low_0 = vertices[3];
					}
					else if (vertices[3].y > corner_low_1.y)
					{
						corner_low_1 = vertices[3];
					}
					pos.x = (corner_low_0.x + corner_low_1.x)/2;
					pos.y = (corner_low_0.y + corner_low_1.y)/2;
					angle = r_rect[trackidx].angle;
					if (r_rect[trackidx].size.height < r_rect[trackidx].size.width)
					{
						angle += 90;
					}
					ss << "pos=" << (pos.x-80) << " a=" << angle << " h=" << r_rect[trackidx].size.height << " w=" << r_rect[trackidx].size.width;
					str = ss.str();
				}

				putText(img, str, Point(10, 30), FONT_HERSHEY_PLAIN, 1.0, CV_RGB(0,0,255), 1.0);
				pos.x = pos.x * 4;
				pos.y = pos.y * 4;
				circle (img,pos,10,Scalar(254,254,254),1);
				Point2f trackdir;
				angle = angle / 180 * CV_PI;
				trackdir.x = sin(angle) * 240;
				trackdir.y = -cos(angle) * 240;
				pos.y = 0;
				pos.x = (pos.x - 320) * 0.2f;
				if (abs(pos.x) < 50) pos.x = 0;
				/*if (abs (pos.x < 160))
				pos.x = 0;
				pos.x = pos.x *2;*/
				trackdir.x = trackdir.x * 1.0f;
				Point2f dir = pos + trackdir;
				std::stringstream ss2;

				line (img,Point(320,240),Point(320+dir.x,240+dir.y),Scalar(0,254,0));

				ss2 << "dir: x=" << dir.x << " / y=" << dir.y;
				str = ss2.str();
				putText(img, str, Point(10, 60), FONT_HERSHEY_PLAIN, 1.0, CV_RGB(0,0,255), 1.0);
				//motors_move (dir.x * 10,dir.y);
				#endif
				memcpy(m_OutputFrameBuffer,framedata,framesz);
				//textures[texidx].SetPixels(tmpbuff);

				//write(frame_data, 1, frame_sz, stdout);
				m_pCamera->EndReadFrame(0);
				m_pVideoStream->streamFrame ((uint8_t*)framedata,framesz);

			}


			//begin frame, draw the texture then end frame (the bit of maths just fits the image to the screen while maintaining aspect ratio)
			/*BeginFrame();
			float aspect_ratio = float(MAIN_TEXTURE_WIDTH)/float(MAIN_TEXTURE_HEIGHT);
			float screen_aspect_ratio = 1680.f/1050.f;
			DrawTextureRect(&textures[texidx],-aspect_ratio/screen_aspect_ratio,1.f,aspect_ratio/screen_aspect_ratio,-1.f);
			//DrawTextureRect(&textures[texidx],-1.f,-1.f,1.f,1.f);
			EndFrame();*/
		}
	}

}

void CImageProcessing::setParameter (YapiBotParam_t param, int8_t * buffer, uint32_t size)
{
	if ((param & PARAM_MASK) == CAMERA_PARAM)
	{
		m_pCamera->setParameter(param,buffer,size);
	}
	else
	{

	}

}
void CImageProcessing::getParameter (YapiBotParam_t param)
{
	if ((param & PARAM_MASK) == CAMERA_PARAM)
	{
		m_pCamera->getParameter(param);
	}
	else
	{

	}

}


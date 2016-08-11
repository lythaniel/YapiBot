/*
 * Camera.h
 *
 * Copyright (C) 2016 Cyrille Potereau
 *
 * based on original work from Chris Cummings:
 * http://robotblogging.blogspot.fr/2013/10/an-efficient-and-simple-c-api-for.html
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */

#pragma once

#include "mmalincludes.h"
#include "CameraControl.h"
#include "YapiBotCmd.h"

class CCamera;

class CCameraOutput
{
public:
	int						Width;
	int						Height;
	MMAL_COMPONENT_T*		ResizerComponent;
	MMAL_CONNECTION_T*		Connection;
	MMAL_BUFFER_HEADER_T*	LockedBuffer;
	MMAL_POOL_T*			BufferPool;
	MMAL_QUEUE_T*			OutputQueue;
	MMAL_PORT_T*			BufferPort;

	CCameraOutput();
	~CCameraOutput();
	bool Init(int width, int height, MMAL_COMPONENT_T* input_component, int input_port_idx, bool do_argb_conversion);
	void Release();
	void OnVideoBufferCallback(MMAL_PORT_T *port, MMAL_BUFFER_HEADER_T *buffer);
	static void VideoBufferCallback(MMAL_PORT_T *port, MMAL_BUFFER_HEADER_T *buffer);
	int ReadFrame(void* buffer, int buffer_size);
	bool BeginReadFrame(const void** out_buffer, int * out_buffer_size);
	void EndReadFrame();
	MMAL_POOL_T* EnablePortCallbackAndCreateBufferPool(MMAL_PORT_T* port, MMAL_PORT_BH_CB_T cb, int buffer_count);
	MMAL_COMPONENT_T* CreateResizeComponentAndSetupPorts(MMAL_PORT_T* video_output_port, bool do_argb_conversion);



};

class CCamera
{
public:

	int ReadFrame(int level, void* buffer, int buffer_size);
	bool BeginReadFrame(int level, const void * * out_buffer, int * out_buffer_size);
	void EndReadFrame(int level);
	MMAL_COMPONENT_T* getCameraComponent() {return CameraComponent;}
	MMAL_COMPONENT_T* getSplitterComponent () {return SplitterComponent;}

	void setParameter (YapiBotParam_t param, char * buffer, unsigned int size);
	void getParameter (YapiBotParam_t param);

private:
	CCamera();
	~CCamera();

	bool Init(int width, int height, int framerate, int num_levels, bool do_argb_conversion);
	void Release();
	MMAL_COMPONENT_T* CreateCameraComponentAndSetupPorts();
	MMAL_COMPONENT_T* CreateSplitterComponentAndSetupPorts(MMAL_PORT_T* video_ouput_port);

	void OnCameraControlCallback(MMAL_PORT_T *port, MMAL_BUFFER_HEADER_T *buffer);
	static void CameraControlCallback(MMAL_PORT_T *port, MMAL_BUFFER_HEADER_T *buffer);

	int							Width;
	int							Height;
	int							FrameRate;
	RASPICAM_CAMERA_PARAMETERS	CameraParameters;
	MMAL_COMPONENT_T*			CameraComponent;    
	MMAL_COMPONENT_T*			SplitterComponent;
	MMAL_CONNECTION_T*			VidToSplitConn;
	CCameraOutput*				Outputs[4];

	int							m_Saturation;
	int							m_Contrast;
	int							m_Brightness;
	int							m_Sharpness;
	int							m_Iso;



	friend CCamera* StartCamera(int width, int height, int framerate, int num_levels, bool do_argb_conversion);
	friend void StopCamera();
};

CCamera* StartCamera(int width, int height, int framerate, int num_levels, bool do_argb_conversion=true);
void StopCamera();

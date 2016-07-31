#pragma once

#include "mmalincludes.h"
#include "CallBack.h"


class CEncoder
{
public:
	CEncoder (int width, int height, int framerate);
	~CEncoder ();

	static void converter_input_callback(MMAL_PORT_T *port, MMAL_BUFFER_HEADER_T *buffer);
	static void encoder_output_callback(MMAL_PORT_T *port, MMAL_BUFFER_HEADER_T *buffer);
	static void encoder_control_callback(MMAL_PORT_T *port, MMAL_BUFFER_HEADER_T *buffer);
	static  MMAL_BOOL_T buffer_header_cb	(MMAL_BUFFER_HEADER_T *header, void *userdata);

	bool encode (uint8_t* data, uint32_t size);
	uint8_t* getSPS (uint32_t *size);
	uint8_t* getPPS (uint32_t *size);

	DECLARE_REG_FUNCTION_CB_2(regFrameEncodedCb, m_pFrameEncodedCb)

private:
	int						m_Width;
	int						m_Height;
	int						m_FrameRate;

	MMAL_COMPONENT_T * m_pEncComp;  // Pointer to the encoder component
	MMAL_COMPONENT_T * m_pConvComp; // Pointer to the converter component

	MMAL_POOL_T * m_pEncPoolOut;	// Pointer to the pool of buffers used by encoder output port
	MMAL_POOL_T * m_pEncPoolCtrl;	// Pointer to the pool of buffers used by encoder output port
	MMAL_POOL_T * m_pConvPoolIn;	// Pointer to the pool of buffers used by encoder output port

	MMAL_PORT_T * m_pEncInPort;  	//encoder_input;
	MMAL_PORT_T * m_pEncOutPort; 	//encoder_output = NULL;
	MMAL_PORT_T * m_pConvInPort;    //encoder_input;
	MMAL_PORT_T * m_pConvOutPort;   //encoder_output = NULL;

	MMAL_CONNECTION_T * m_pConn;

	void onConverterInputCallback(MMAL_PORT_T *port, MMAL_BUFFER_HEADER_T *buffer);
	void onEncoderOutputCallback(MMAL_PORT_T *port, MMAL_BUFFER_HEADER_T *buffer);
	void onEncoderControlCallback(MMAL_PORT_T *port, MMAL_BUFFER_HEADER_T *buffer);

	MMAL_STATUS_T ConnectConvToEnc(void);
	MMAL_STATUS_T CreateConverter(void);
	MMAL_STATUS_T CreateEncoder(void);

	void DestroyComponents(void);

	uint8_t* m_pSPSBuffer;
	uint32_t m_SPSSize;

	uint8_t* m_pPPSBuffer;
	uint32_t m_PPSSize;

	Callback2base<uint8_t*,uint32_t> * m_pFrameEncodedCb;


};


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string>

#include "Encoder.h"

#include "mmalincludes.h"
#define ENCODER_BITRATE (1024*1024) //1Mbps

CEncoder::CEncoder (int width, int height, int framerate) :
m_Width (width),
m_Height (height),
m_FrameRate (framerate)
{
	m_pEncComp = NULL;
	m_pConvComp = NULL;

	m_pEncPoolOut = NULL;
	m_pEncPoolCtrl = NULL;
	m_pConvPoolIn = NULL;

	m_pEncInPort = NULL;
	m_pEncOutPort = NULL;
	m_pConvInPort = NULL;
	m_pConvOutPort = NULL;

	m_pSPSBuffer = NULL;
	m_SPSSize = 0;

	m_pPPSBuffer = NULL;
	m_PPSSize = 0;

	m_pConn = NULL;

	m_pFrameEncodedCb = NULL;

	if (CreateEncoder() != MMAL_SUCCESS)
	{
		printf("Failed to create Encoder !\n");
		DestroyComponents();
		return;
	}
	if (CreateConverter() != MMAL_SUCCESS)
	{
		printf("Failed to create Encoder !\n");
		DestroyComponents();
		return;
	}
	if (ConnectConvToEnc() != MMAL_SUCCESS)
	{
		printf("Failed to create Encoder !\n");
		DestroyComponents();
		return;
	}
	printf("Encoder init done !\n");
}

CEncoder::~CEncoder()
{

	DestroyComponents();
	if (m_pSPSBuffer != NULL)
	{
		delete [] m_pSPSBuffer;
	}
	if (m_pPPSBuffer != NULL)
	{
		delete [] m_pPPSBuffer;
	}
	if (m_pFrameEncodedCb != NULL)
	{
		delete [] m_pFrameEncodedCb;
	}
}


bool CEncoder::encode (uint8_t* data, uint32_t size)
{
	bool ret = false;
	MMAL_BUFFER_HEADER_T * buffer;

	if ((buffer = mmal_queue_get(m_pConvPoolIn->queue)) != NULL)
	{
		mmal_buffer_header_pre_release_cb_set (buffer,buffer_header_cb, NULL);
		mmal_buffer_header_mem_lock(buffer);
		if (size <= buffer->alloc_size)
		{
			memcpy(buffer->data,data,size);
		}
		//buffer->data = (uint8_t*)data;
		buffer->length = size;
		mmal_buffer_header_mem_unlock(buffer);
		if (mmal_port_send_buffer(m_pConvInPort, buffer)== MMAL_SUCCESS)
		{
			ret = true;
		}
	}
	return ret;
}


MMAL_STATUS_T CEncoder::ConnectConvToEnc (void)
{
	MMAL_STATUS_T status;
	//create and enable a connection between the video output and the resizer input
	status = mmal_connection_create(&m_pConn, m_pConvComp->output[0], m_pEncComp->input[0], MMAL_CONNECTION_FLAG_TUNNELLING | MMAL_CONNECTION_FLAG_ALLOCATION_ON_INPUT);
	if (status != MMAL_SUCCESS)
	{
		printf("Failed to create connection\n");
	}
	status = mmal_connection_enable(m_pConn);
	if (status != MMAL_SUCCESS)
	{
	printf("Failed to enable connection\n");
	}
	m_pConvInPort->userdata = (struct MMAL_PORT_USERDATA_T *)this;
	m_pEncOutPort->userdata = (struct MMAL_PORT_USERDATA_T *)this;
	status = mmal_port_enable(m_pConvInPort, converter_input_callback);
	status = mmal_port_enable(m_pEncOutPort, encoder_output_callback);

	int num = mmal_queue_length(m_pEncPoolOut->queue);

	for (int q=0;q<num;q++)
	{
		MMAL_BUFFER_HEADER_T *buffer = mmal_queue_get(m_pEncPoolOut->queue);

		if (!buffer)
		{
			printf("Unable to get a required buffer %d from pool queue", q);
		}
		mmal_buffer_header_pre_release_cb_set (buffer,buffer_header_cb, NULL);

		if (mmal_port_send_buffer(m_pEncComp->output[0], buffer)!= MMAL_SUCCESS)
		{
			printf("Unable to send a buffer to encoder output port (%d)", q);
		}
	}

	return status;
}


MMAL_STATUS_T CEncoder::CreateConverter (void)
{
	MMAL_ES_FORMAT_T *format;
	MMAL_STATUS_T status;

	//create the camera component
	status = mmal_component_create("vc.ril.resize", &m_pConvComp);
	if (status != MMAL_SUCCESS)
	{
		printf("Failed to create converter component\n");
	}

	//check we have output ports
	if (m_pConvComp->output_num != 1 || m_pConvComp->input_num != 1)
	{
		printf("converter doesn't have correct ports");
	}

	//get the ports
	m_pConvInPort = m_pConvComp->input[0];
	m_pConvOutPort = m_pConvComp->output[0];


	m_pConvInPort->buffer_size = m_pEncInPort->buffer_size_recommended;
	if (m_pConvInPort->buffer_size < m_pEncInPort->buffer_size_min)
		m_pConvInPort->buffer_size = m_pEncInPort->buffer_size_min;

	m_pConvInPort->buffer_num = m_pEncInPort->buffer_num_recommended;

	if (m_pConvInPort->buffer_num < m_pEncInPort->buffer_num_min)
		m_pConvInPort->buffer_num = m_pEncInPort->buffer_num_min;

	m_pConvOutPort->buffer_size = m_pConvInPort->buffer_size;
	m_pConvOutPort->buffer_num = m_pConvInPort->buffer_num;


	m_pConvOutPort->format->encoding = MMAL_ENCODING_I420;
	m_pConvOutPort->format->encoding_variant = MMAL_ENCODING_I420;

	m_pConvOutPort->format->es->video.width = m_Width;
	m_pConvOutPort->format->es->video.height = m_Height;


	m_pConvOutPort->format->es->video.crop.x = 0;
	m_pConvOutPort->format->es->video.crop.y = 0;
	m_pConvOutPort->format->es->video.crop.width = m_Width;
	m_pConvOutPort->format->es->video.crop.height = m_Height;


	m_pConvOutPort->format->es->video.frame_rate.num = m_FrameRate*65536;
	m_pConvOutPort->format->es->video.frame_rate.den = 65536;

	status = mmal_port_format_commit(m_pConvOutPort);
	if (status != MMAL_SUCCESS)
	{
		printf("Couldn't set resizer input port format : error %d", status);
	}

	mmal_format_copy(m_pConvInPort->format,m_pConvOutPort->format);

	m_pConvInPort->format->encoding = MMAL_ENCODING_RGBA;
	m_pConvInPort->format->encoding_variant = MMAL_ENCODING_RGBA;

	status = mmal_port_format_commit(m_pConvInPort);
	if (status != MMAL_SUCCESS)
	{
		printf("Couldn't set converter output port format : error %d", status);
	}

	// Enable component
	status = mmal_component_enable(m_pConvComp);

	if (status != MMAL_SUCCESS)
	{
		printf("Unable to enable video converter component");
	}

	/* Create pool of buffer headers for the output port to consume */
	m_pConvPoolIn = mmal_port_pool_create(m_pConvInPort, m_pConvInPort->buffer_num, m_pConvInPort->buffer_size);

	if (!m_pConvPoolIn)
	{
		printf("Failed to create buffer header pool for converter input port %s", m_pConvComp->name);
	}


	// Enable the component. Components will only process data when they are enabled.
	status = mmal_component_enable(m_pConvComp);


	printf("Converter component creation done\n");

	return status;
	}


MMAL_STATUS_T CEncoder::CreateEncoder()
{
	MMAL_STATUS_T status;

	status = mmal_component_create(MMAL_COMPONENT_DEFAULT_VIDEO_ENCODER, &m_pEncComp);

	if (status != MMAL_SUCCESS)
	{
		printf("Unable to create video encoder component");
	}

	if (!m_pEncComp->input_num || !m_pEncComp->output_num)
	{
		status = MMAL_ENOSYS;
		printf("Video encoder doesn't have input/output ports");
	}


	m_pEncInPort = m_pEncComp->input[0];
	m_pEncOutPort = m_pEncComp->output[0];

	m_pEncInPort->buffer_size = m_pEncInPort->buffer_size_recommended;

	if (m_pEncInPort->buffer_size < m_pEncInPort->buffer_size_min)
		m_pEncInPort->buffer_size = m_pEncInPort->buffer_size_min;

	m_pEncInPort->buffer_num = m_pEncInPort->buffer_num_recommended;

	if (m_pEncInPort->buffer_num < m_pEncInPort->buffer_num_min)
		m_pEncInPort->buffer_num = m_pEncInPort->buffer_num_min;



	// Only supporting H264 at the moment
	m_pEncOutPort->format->encoding = MMAL_ENCODING_H264;

	m_pEncOutPort->format->bitrate = ENCODER_BITRATE;

	m_pEncOutPort->buffer_size = m_pEncOutPort->buffer_size_recommended;

	if (m_pEncOutPort->buffer_size < m_pEncOutPort->buffer_size_min)
		m_pEncOutPort->buffer_size = m_pEncOutPort->buffer_size_min;

	m_pEncOutPort->buffer_num = 10;//m_pEncOutPort->buffer_num_recommended;

	if (m_pEncOutPort->buffer_num < m_pEncOutPort->buffer_num_min)
		m_pEncOutPort->buffer_num = m_pEncOutPort->buffer_num_min;

	// We need to set the frame rate on output to 0, to ensure it gets
	// updated correctly from the input framerate when port connected

	m_pEncOutPort->format->es->video.frame_rate.num = 0;//1638400;
	m_pEncOutPort->format->es->video.frame_rate.den = 1;//65536;

	// Commit the port changes to the output port
	status = mmal_port_format_commit(m_pEncOutPort);

	if (status != MMAL_SUCCESS)
	{
		printf("Unable to set format on video encoder output port");
	}

	// Set the rate control parameter
	if (0)
	{
		MMAL_PARAMETER_VIDEO_RATECONTROL_T param = {{ MMAL_PARAMETER_RATECONTROL, sizeof(param)}, MMAL_VIDEO_RATECONTROL_DEFAULT};
		status = mmal_port_parameter_set(m_pEncOutPort, &param.hdr);
		if (status != MMAL_SUCCESS)
		{
			printf("Unable to set ratecontrol");
		}
	}

	/*if (state->intraperiod)
	{
	MMAL_PARAMETER_UINT32_T param = {{ MMAL_PARAMETER_INTRAPERIOD, sizeof(param)}, state->intraperiod};
	status = mmal_port_parameter_set(m_pEncOutPort, &param.hdr);
	if (status != MMAL_SUCCESS)
	{
	vcos_log_error("Unable to set intraperiod");
	goto error;
	}
	}*/

	/*if (state->quantisationParameter)
	{
	MMAL_PARAMETER_UINT32_T param = {{ MMAL_PARAMETER_VIDEO_ENCODE_INITIAL_QUANT, sizeof(param)}, state->quantisationParameter};
	status = mmal_port_parameter_set(m_pEncOutPort, &param.hdr);
	if (status != MMAL_SUCCESS)
	{
	vcos_log_error("Unable to set initial QP");
	goto error;
	}

	MMAL_PARAMETER_UINT32_T param2 = {{ MMAL_PARAMETER_VIDEO_ENCODE_MIN_QUANT, sizeof(param)}, state->quantisationParameter};
	status = mmal_port_parameter_set(m_pEncOutPort, &param2.hdr);
	if (status != MMAL_SUCCESS)
	{
	vcos_log_error("Unable to set min QP");
	goto error;
	}

	MMAL_PARAMETER_UINT32_T param3 = {{ MMAL_PARAMETER_VIDEO_ENCODE_MAX_QUANT, sizeof(param)}, state->quantisationParameter};
	status = mmal_port_parameter_set(m_pEncOutPort, &param3.hdr);
	if (status != MMAL_SUCCESS)
	{
	vcos_log_error("Unable to set max QP");
	goto error;
	}

	}*/


	MMAL_PARAMETER_VIDEO_PROFILE_T param;
	param.hdr.id = MMAL_PARAMETER_PROFILE;
	param.hdr.size = sizeof(param);

	param.profile[0].profile = MMAL_VIDEO_PROFILE_H264_HIGH;//state->profile;
	param.profile[0].level = MMAL_VIDEO_LEVEL_H264_4; // This is the only value supported

	status = mmal_port_parameter_set(m_pEncOutPort, &param.hdr);
	if (status != MMAL_SUCCESS)
	{
		printf("Unable to set H264 profile");
	}


	if (mmal_port_parameter_set_boolean(m_pEncInPort, MMAL_PARAMETER_VIDEO_IMMUTABLE_INPUT, 1 /*state->immutableInput*/) != MMAL_SUCCESS)
	{
		printf("Unable to set immutable input flag");
		// Continue rather than abort..
	}

	//set INLINE HEADER flag to generate SPS and PPS for every IDR if requested
	if (mmal_port_parameter_set_boolean(m_pEncOutPort, MMAL_PARAMETER_VIDEO_ENCODE_INLINE_HEADER, 0 /*state->bInlineHeaders*/) != MMAL_SUCCESS)
	{
		printf("failed to set INLINE HEADER FLAG parameters");
		// Continue rather than abort..
	}

	// Enable component
	status = mmal_component_enable(m_pEncComp);

	if (status != MMAL_SUCCESS)
	{
		printf("Unable to enable video encoder component");
	}

	/* Create pool of buffer headers for the output port to consume */
	m_pEncPoolOut = mmal_port_pool_create(m_pEncOutPort, m_pEncOutPort->buffer_num, m_pEncOutPort->buffer_size);

	if (!m_pEncPoolOut)
	{
		printf("Failed to create buffer header pool for encoder output port %s", m_pEncOutPort->name);
	}

	/* Create pool of buffer headers for the output port to consume */
	m_pEncPoolCtrl = mmal_port_pool_create(m_pEncComp->control, m_pEncComp->control->buffer_num, m_pEncComp->control->buffer_size);

	if (!m_pEncPoolCtrl)
	{
		printf("Failed to create buffer header pool for encoder control port %s", m_pEncOutPort->name);
	}
	m_pEncComp->control->userdata = (struct MMAL_PORT_USERDATA_T *)this;
	status = mmal_port_enable(m_pEncComp->control, encoder_control_callback);

	// Enable the component. Components will only process data when they are enabled.
	status = mmal_component_enable(m_pEncComp);

	printf("Encoder component creation done\n");

	return status;
}

void CEncoder::DestroyComponents(void)
{
	// Get rid of any port buffers first
	if (m_pConvPoolIn)
	{
		mmal_port_pool_destroy(m_pConvComp->input[0], m_pConvPoolIn);
	}
	if (m_pEncPoolOut)
	{
		mmal_port_pool_destroy(m_pEncComp->output[0], m_pEncPoolOut);
	}

	if (m_pConvComp)
	{
		mmal_component_destroy(m_pConvComp);
		m_pConvComp = NULL;
	}
	if (m_pEncComp)
	{
		mmal_component_destroy(m_pEncComp);
		m_pEncComp = NULL;
	}
}


void CEncoder::onEncoderControlCallback(MMAL_PORT_T *port, MMAL_BUFFER_HEADER_T *buffer)
{
   if (buffer->cmd == MMAL_EVENT_PARAMETER_CHANGED)
   {
   }
   else
   {
      //printf("Received unexpected camera control callback event, 0x%08x", buffer->cmd);
   }

   mmal_buffer_header_release(buffer);
}


void CEncoder::onConverterInputCallback(MMAL_PORT_T *port, MMAL_BUFFER_HEADER_T *buffer)
{
	// The decoder is done with the data, just recycle the buffer header into its pool
	 mmal_buffer_header_release(buffer);
}


void CEncoder::onEncoderOutputCallback(MMAL_PORT_T *port, MMAL_BUFFER_HEADER_T *buffer)
{
	MMAL_BUFFER_HEADER_T *new_buffer;
	MMAL_STATUS_T status;
	int bytes_written;

	if (buffer->length)
	{
		mmal_buffer_header_mem_lock(buffer);
		if (buffer->flags && MMAL_BUFFER_HEADER_FLAG_CONFIG)
		{
			if (buffer->length > 4)
			{
				if ((buffer->data[4]&0x1f)==0x07) //SPS Info
				{
					if (m_pSPSBuffer == NULL)
					{
						m_SPSSize = buffer->length;
						m_pSPSBuffer = new uint8_t [m_SPSSize];
						memcpy (m_pSPSBuffer,buffer->data,m_SPSSize);
					}

				}
				else if ((buffer->data[4]&0x1f)==0x08) //PPS Info
				{
					if (m_pPPSBuffer == NULL)
					{
						m_PPSSize = buffer->length;
						m_pPPSBuffer = new uint8_t [m_PPSSize];
						memcpy (m_pPPSBuffer,buffer->data,m_PPSSize);
					}
				}
			}
		}

		//bytes_written = fwrite(buffer->data, 1, buffer->length, stdout);
		if (m_pFrameEncodedCb != NULL)
		{
			m_pFrameEncodedCb->trigger((uint8_t*)buffer->data, (uint32_t)buffer->length);
		}

		mmal_buffer_header_mem_unlock(buffer);

		/*if (bytes_written != buffer->length)
		{
			//printf("Failed to write buffer data (%d from %d)- aborting", bytes_written, buffer->length);
			//pData->abort = 1;
		}*/
	}

	// release buffer back to the pool
	mmal_buffer_header_release(buffer);

	// and send one back to the port (if still open)
	if (port->is_enabled)
	{
		new_buffer = mmal_queue_get(m_pEncPoolOut->queue);
		if (new_buffer)
		{
		status = mmal_port_send_buffer(port, new_buffer);
		}
	}
}

 void CEncoder::encoder_control_callback(MMAL_PORT_T *port, MMAL_BUFFER_HEADER_T *buffer)
{
	((CEncoder *)port->userdata)->onEncoderControlCallback(port,buffer);
}


void CEncoder::converter_input_callback(MMAL_PORT_T *port, MMAL_BUFFER_HEADER_T *buffer)
{
	((CEncoder *)port->userdata)->onConverterInputCallback(port,buffer);
}


void CEncoder::encoder_output_callback(MMAL_PORT_T *port, MMAL_BUFFER_HEADER_T *buffer)
{
	((CEncoder *)port->userdata)->onEncoderOutputCallback(port,buffer);
}

MMAL_BOOL_T CEncoder::buffer_header_cb	(MMAL_BUFFER_HEADER_T *header, void *userdata)
{
	return false;
}

uint8_t* CEncoder::getSPS (uint32_t *size)
{
	*size = m_SPSSize;
	return m_pSPSBuffer;
}

uint8_t* CEncoder::getPPS (uint32_t *size)
{
	*size = m_PPSSize;
	return m_pPPSBuffer;
}


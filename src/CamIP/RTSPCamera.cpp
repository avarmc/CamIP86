/*
 * copyright (c) 2015 Elsys Corp.
 *
 * This file is part of CamIP.
 *
 * CamIP is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * CamIP is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with CamIP; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */ 

#include "StdAfx.h"
#include "RTSPCamera.h"
#include <stdio.h>
#include <stdlib.h>
#include <direct.h>

#define __STDC_CONSTANT_MACROS
#define __STDC_LIMIT_MACROS


extern "C" {
#include <libavdevice/avdevice.h>
#include <libswscale/swscale.h>
#include <libavutil/avstring.h>
//#include <libavformat/rtsp.h>
}


enum RTSPLowerTransport {
    RTSP_LOWER_TRANSPORT_UDP = 0,           /**< UDP/unicast */
    RTSP_LOWER_TRANSPORT_TCP = 1,           /**< TCP; interleaved in RTSP */
    RTSP_LOWER_TRANSPORT_UDP_MULTICAST = 2, /**< UDP/multicast */
    RTSP_LOWER_TRANSPORT_NB,
    RTSP_LOWER_TRANSPORT_HTTP = 8,          /**< HTTP tunneled - not a proper
                                                 transport mode as such,
                                                 only for use via AVOptions */
    RTSP_LOWER_TRANSPORT_CUSTOM = 16,       /**< Custom IO - not a public
                                                 option for lower_transport_mask,
                                                 but set in the SDP demuxer based
                                                 on a flag. */
};


enum RTSPTransport {
    RTSP_TRANSPORT_RTP, /**< Standards-compliant RTP */
    RTSP_TRANSPORT_RDT, /**< Realmedia Data Transport */
    RTSP_TRANSPORT_RAW, /**< Raw data (over UDP) */
    RTSP_TRANSPORT_NB
};

/**
 * Transport mode for the RTSP data. This may be plain, or
 * tunneled, which is done over HTTP.
 */
enum RTSPControlTransport {
    RTSP_MODE_PLAIN,   /**< Normal RTSP */
    RTSP_MODE_TUNNEL   /**< RTSP over HTTP (tunneling) */
};


enum RTSPClientState {
    RTSP_STATE_IDLE,    /**< not initialized */
    RTSP_STATE_STREAMING, /**< initialized and sending/receiving data */
    RTSP_STATE_PAUSED,  /**< initialized, but not receiving data */
    RTSP_STATE_SEEKING, /**< initialized, requesting a seek */
};

/**
 * Identify particular servers that require special handling, such as
 * standards-incompliant "Transport:" lines in the SETUP request.
 */
enum RTSPServerType {
    RTSP_SERVER_RTP,  /**< Standards-compliant RTP-server */
    RTSP_SERVER_REAL, /**< Realmedia-style server */
    RTSP_SERVER_WMS,  /**< Windows Media server */
    RTSP_SERVER_NB
};


/**
 * Authentication types, ordered from weakest to strongest.
 */
typedef enum HTTPAuthType {
    HTTP_AUTH_NONE = 0,    /**< No authentication specified */
    HTTP_AUTH_BASIC,       /**< HTTP 1.0 Basic auth from RFC 1945
                             *  (also in RFC 2617) */
    HTTP_AUTH_DIGEST,      /**< HTTP 1.1 Digest auth from RFC 2617 */
} HTTPAuthType;

typedef struct DigestParams {
    char nonce[300];       /**< Server specified nonce */
    char algorithm[10];    /**< Server specified digest algorithm */
    char qop[30];          /**< Quality of protection, containing the one
                             *  that we've chosen to use, from the
                             *  alternatives that the server offered. */
    char opaque[300];      /**< A server-specified string that should be
                             *  included in authentication responses, not
                             *  included in the actual digest calculation. */
    char stale[10];        /**< The server indicated that the auth was ok,
                             * but needs to be redone with a new, non-stale
                             * nonce. */
    int nc;                /**< Nonce count, the number of earlier replies
                             *  where this particular nonce has been used. */
} DigestParams;

/**
 * HTTP Authentication state structure. Must be zero-initialized
 * before used with the functions below.
 */
typedef struct HTTPAuthState {
    /**
     * The currently chosen auth type.
     */
    HTTPAuthType auth_type;
    /**
     * Authentication realm
     */
    char realm[200];
    /**
     * The parameters specifiec to digest authentication.
     */
    DigestParams digest_params;
    /**
     * Auth ok, but needs to be resent with a new nonce.
     */
    int stale;
} HTTPAuthState;

typedef struct RTSPState {
    const void *class1;             /**< Class for private options. */
    void *rtsp_hd; /* RTSP TCP connection handle */

    /** number of items in the 'rtsp_streams' variable */
    int nb_rtsp_streams;

    void **rtsp_streams; /**< streams in this session */

    /** indicator of whether we are currently receiving data from the
     * server. Basically this isn't more than a simple cache of the
     * last PLAY/PAUSE command sent to the server, to make sure we don't
     * send 2x the same unexpectedly or commands in the wrong state. */
    enum RTSPClientState state;

    /** the seek value requested when calling av_seek_frame(). This value
     * is subsequently used as part of the "Range" parameter when emitting
     * the RTSP PLAY command. If we are currently playing, this command is
     * called instantly. If we are currently paused, this command is called
     * whenever we resume playback. Either way, the value is only used once,
     * see rtsp_read_play() and rtsp_read_seek(). */
    int64_t seek_timestamp;

    int seq;                          /**< RTSP command sequence number */

    /** copy of RTSPMessageHeader->session_id, i.e. the server-provided session
     * identifier that the client should re-transmit in each RTSP command */
    char session_id[512];

    /** copy of RTSPMessageHeader->timeout, i.e. the time (in seconds) that
     * the server will go without traffic on the RTSP/TCP line before it
     * closes the connection. */
    int timeout;

    /** timestamp of the last RTSP command that we sent to the RTSP server.
     * This is used to calculate when to send dummy commands to keep the
     * connection alive, in conjunction with timeout. */
    int64_t last_cmd_time;

    /** the negotiated data/packet transport protocol; e.g. RTP or RDT */
    enum RTSPTransport transport;

    /** the negotiated network layer transport protocol; e.g. TCP or UDP
     * uni-/multicast */
    enum RTSPLowerTransport lower_transport;

    /** brand of server that we're talking to; e.g. WMS, REAL or other.
     * Detected based on the value of RTSPMessageHeader->server or the presence
     * of RTSPMessageHeader->real_challenge */
    enum RTSPServerType server_type;

    /** the "RealChallenge1:" field from the server */
    char real_challenge[64];

    /** plaintext authorization line (username:password) */
    char auth[128];

    /** authentication state */
    HTTPAuthState auth_state;

    /** The last reply of the server to a RTSP command */
    char last_reply[2048]; /* XXX: allocate ? */

    /** RTSPStream->transport_priv of the last stream that we read a
     * packet from */
    void *cur_transport_priv;

    /** The following are used for Real stream selection */
    //@{
    /** whether we need to send a "SET_PARAMETER Subscribe:" command */
    int need_subscription;

    /** stream setup during the last frame read. This is used to detect if
     * we need to subscribe or unsubscribe to any new streams. */
    void *real_setup_cache;

    /** current stream setup. This is a temporary buffer used to compare
     * current setup to previous frame setup. */
    void *real_setup;

    /** the last value of the "SET_PARAMETER Subscribe:" RTSP command.
     * this is used to send the same "Unsubscribe:" if stream setup changed,
     * before sending a new "Subscribe:" command. */
    char last_subscription[1024];
    //@}

    /** The following are used for RTP/ASF streams */
    //@{
    /** ASF demuxer context for the embedded ASF stream from WMS servers */
    void *asf_ctx;

    /** cache for position of the asf demuxer, since we load a new
     * data packet in the bytecontext for each incoming RTSP packet. */
    uint64_t asf_pb_pos;
    //@}

    /** some MS RTSP streams contain a URL in the SDP that we need to use
     * for all subsequent RTSP requests, rather than the input URI; in
     * other cases, this is a copy of AVFormatContext->filename. */
    char control_uri[1024];

    /** The following are used for parsing raw mpegts in udp */
    //@{
    void *ts;
    int recvbuf_pos;
    int recvbuf_len;
    //@}

    /** Additional output handle, used when input and output are done
     * separately, eg for HTTP tunneling. */
    void *rtsp_hd_out;

    /** RTSP transport mode, such as plain or tunneled. */
    enum RTSPControlTransport control_transport;

    /* Number of RTCP BYE packets the RTSP session has received.
     * An EOF is propagated back if nb_byes == nb_streams.
     * This is reset after a seek. */
    int nb_byes;

    /** Reusable buffer for receiving packets */
    uint8_t* recvbuf;

    /**
     * A mask with all requested transport methods
     */
    int lower_transport_mask;

    /**
     * The number of returned packets
     */
    uint64_t packets;

    /**
     * Polling array for udp
     */
    void *p;

    /**
     * Whether the server supports the GET_PARAMETER method.
     */
    int get_parameter_supported;

    /**
     * Do not begin to play the stream immediately.
     */
    int initial_pause;

    /**
     * Option flags for the chained RTP muxer.
     */
    int rtp_muxer_flags;

    /** Whether the server accepts the x-Dynamic-Rate header */
    int accept_dynamic_rate;

    /**
     * Various option flags for the RTSP muxer/demuxer.
     */
    int rtsp_flags;

    /**
     * Mask of all requested media types
     */
    int media_type_mask;

    /**
     * Minimum and maximum local UDP ports.
     */
    int rtp_port_min, rtp_port_max;

    /**
     * Timeout to wait for incoming connections.
     */
    int initial_timeout;

    /**
     * timeout of socket i/o operations.
     */
    int stimeout;

    /**
     * Size of RTP packet reordering queue.
     */
    int reordering_queue_size;

    /**
     * User-Agent string
     */
    char *user_agent;

    char default_lang[4];
} RTSPState;


class CRTSPCamera_data
{
public:
	CRTSPCamera_data()
	{
		pFormatCtx = 0;
		pCodecCtx = 0;
		pCodec = 0;
		pFrame = 0;
		pFrameRGB = 0;
		f_desw = 0;
		videoStream = -1;
		buffer = 0;
		numBytes = 0;
		pBFH = 0;
		pBIH = 0;
		format_opts = 0;
	}
	~CRTSPCamera_data()
	{
		if(pCodecCtx)
			avcodec_close(pCodecCtx);
		if(pFrame)
			av_free(pFrame);
		if(pFrameRGB)
			av_free(pFrameRGB);
		if(pFormatCtx)
			avformat_close_input(&pFormatCtx);
		if(pBFH)
			av_free(pBFH);
		if(format_opts)
			av_dict_free(&format_opts);

	}
public:
	AVDictionary *format_opts;
	AVCodecContext  *pCodecCtx;
	AVFormatContext *pFormatCtx;
	AVCodec * pCodec;
	AVFrame *pFrame, *pFrameRGB;
	int videoStream;

	int				f_desw;

	BITMAPFILEHEADER *pBFH;
	BITMAPINFOHEADER *pBIH;
	uint8_t *buffer;
    int numBytes;
};

CRTSPCamera::CRTSPCamera(CIPCameraCallback *pc)
{

	pCallback = pc;
	pData = 0;
	hThread = 0;
	bStop = false;
	nFrame = 0;

	urlBase[0] = 0;

	tFrame	= GetTickCount()-60000;

	av_register_all();
	avdevice_register_all();
	avcodec_register_all();
	avformat_network_init();
}

CRTSPCamera::~CRTSPCamera(void)
{
	Disconnect();

}


bool CRTSPCamera::Connect(LPCTSTR url)
{
	Disconnect();

	char fmt[256];

	if(url)
		wcscpy_s(urlBase,1024,url);
	
	tFrame	= GetTickCount();
	
	pData = new CRTSPCamera_data();

	AVInputFormat *iformat = NULL;
	
	pData->pFormatCtx = avformat_alloc_context();

	if(CString(urlBase).Left(4).CompareNoCase( L"http") == 0 )
	{
		av_dict_set(&pData->format_opts, "input_format", "mjpeg", 0);
		iformat = av_find_input_format("mjpeg");
	} else
	{
	}

	av_dict_set(&pData->format_opts, "timeout", "-1", 0);
	av_dict_set(&pData->format_opts, "stimeout", "-1", 0);

	if(avformat_open_input(&pData->pFormatCtx,CStringA(urlBase),iformat,&pData->format_opts) != 0) 
	{
		Disconnect();
		return false;
	}
	
	if(avformat_find_stream_info(pData->pFormatCtx,0) < 0)   
	{
		Disconnect();
		return false;
	}
    av_dump_format(pData->pFormatCtx, 0, CStringA(urlBase), 0);
  
    for(UINT i=0; i < pData->pFormatCtx->nb_streams; i++)
    {
        if(pData->pFormatCtx->streams[i]->codec->coder_type==AVMEDIA_TYPE_VIDEO)
        {
            pData->videoStream = i;
            break;
        }
    }

    if(pData->videoStream == -1)
	{
		Disconnect();
		return false;
	}

    pData->pCodecCtx = pData->pFormatCtx->streams[pData->videoStream]->codec;
 
    pData->pCodec = avcodec_find_decoder(pData->pCodecCtx->codec_id);
    if(pData->pCodec==NULL)
	{//codec not found
		Disconnect();
		return false;
	}
 
    if(avcodec_open2(pData->pCodecCtx,pData->pCodec,NULL) < 0) 
	{
		Disconnect();
		return false;
	}

    pData->pFrame    = av_frame_alloc();
    pData->pFrameRGB = av_frame_alloc();
 
 
    AVPixelFormat  pFormat = AV_PIX_FMT_BGR24;
    pData->numBytes = avpicture_get_size(pFormat,pData->pCodecCtx->width,pData->pCodecCtx->height) ; //AV_PIX_FMT_RGB24
	int nb2 = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + pData->numBytes*sizeof(uint8_t);
	pData->pBFH = (BITMAPFILEHEADER *) av_malloc(nb2);
	ZeroMemory(pData->pBFH,nb2);

	pData->pBIH = (BITMAPINFOHEADER*)(pData->pBFH+1);
	pData->buffer = (uint8_t*)(pData->pBIH+1);

	pData->pBFH->bfType = *(WORD*)"BM";
	pData->pBFH->bfSize = nb2;
	pData->pBFH->bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

	pData->pBIH->biSize = sizeof(BITMAPINFOHEADER);
	pData->pBIH->biBitCount = 24;
	pData->pBIH->biPlanes = 1;
	pData->pBIH->biWidth = pData->pCodecCtx->width;
	pData->pBIH->biHeight = pData->pCodecCtx->height;


    avpicture_fill((AVPicture *) pData->pFrameRGB,pData->buffer,pFormat,pData->pCodecCtx->width,pData->pCodecCtx->height);
 
	return true;
}

void CRTSPCamera::Disconnect(void)
{
	if(pData)
		delete pData;
	pData = 0;
}


bool CRTSPCamera::IsConnected(void)
{
	return !!pData;
}

bool CRTSPCamera::IsStarted(void)
{
	return !!hThread;
}

bool CRTSPCamera::Start(LPCTSTR url)
{
	Stop();

	if(url)
		wcscpy_s(urlBase,1024,url);

	hThread = CreateThread(0,1024*1024*8,ThreadProc,this,0,0);

	SyncDelay();
	
	return true;
}

void CRTSPCamera::SyncDelay(DWORD delay)
{
	DWORD nf = nFrame;
	DWORD t0 = GetTickCount();

	while( nFrame == nf && ( GetTickCount()-t0 ) < delay )
		Sleep(10);
}

void CRTSPCamera::Stop(void)
{
	bStop = true;
	while(hThread)
		Sleep(10);
	
	Disconnect();
	bStop = false;
	nFrame = 0;

}

DWORD CRTSPCamera::ThreadProc(LPVOID p)
{
	CRTSPCamera *pThis = (CRTSPCamera*)p;
	pThis->ThreadLocal();
	return 0;
}

void CRTSPCamera::ThreadLocal()
{
    while(!bStop )
    {

		if(Connect())
		{
			GetFrames();
		}

		Disconnect();

		if(!bStop)
			Sleep(500);
	}

	CloseHandle(hThread);
	hThread = 0;
}

void CRTSPCamera::GetFrames()
{

	int res;
    int frameFinished;
    AVPacket packet;
    while(!bStop )
    {
		RTSPState *rt = (RTSPState *)pData->pCodecCtx->priv_data;
		if( rt->timeout > 30 || rt->timeout <= 0)
			rt->timeout = 30;

		if(res = av_read_frame(pData->pFormatCtx,&packet)<0)
			break;
         if(packet.stream_index == pData->videoStream){
 
            avcodec_decode_video2(pData->pCodecCtx,pData->pFrame,&frameFinished,&packet);
 
            if(frameFinished){
 
#ifdef _DEBUG
				ZeroMemory(pData->buffer,pData->numBytes);
#endif
                struct SwsContext * img_convert_ctx;
				int w = pData->pCodecCtx->width;
				int h = pData->pCodecCtx->height;
				int w4 = (w+3)&(~3);
				int bw = pData->pFrameRGB->linesize[0];
				int bw4 = w4*3;

                img_convert_ctx = sws_getCachedContext(NULL,w, h, pData->pCodecCtx->pix_fmt,  
					pData->pCodecCtx->width, pData->pCodecCtx->height, AV_PIX_FMT_BGR24, SWS_BICUBIC, NULL, NULL,NULL);
                int r2 = sws_scale(img_convert_ctx, ((AVPicture*)pData->pFrame)->data, ((AVPicture*)pData->pFrame)->linesize, 0, pData->pCodecCtx->height, ((AVPicture *)pData->pFrameRGB)->data, ((AVPicture *)pData->pFrameRGB)->linesize);
 
				if(r2 != -1)
				{
					nFrame ++;
					tFrame	= GetTickCount();
					if(pCallback)
						pCallback->OnImage(pData->pFrameRGB->data[0],w,h);
				}

                av_free_packet(&packet);
                sws_freeContext(img_convert_ctx);
            }
 
        }
 
    }

	av_free_packet(&packet);

}




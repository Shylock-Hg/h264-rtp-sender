  /* The oRTP library is an RTP (Realtime Transport Protocol - rfc3550) stack.
  Copyright (C) 2001  Simon MORLAT simon.morlat@linphone.org

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include <ortp/ortp.h>
#include <signal.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>

#ifndef _WIN32 
#include <sys/types.h>
#include <sys/time.h>
#include <stdio.h>
#endif

#include "h264.h"

/*
#define Y_PLOAD_TYPE 96 //H.264
#define MAX_RTP_PKT_LENGTH 1400
#define DefaultTimestampIncrement 3600 //(90000/25)
uint32_t g_userts=0;

int  rtpSend(RtpSession *session, char  *buffer,  int  len)
{  
    int  sendBytes = 0; 
    int status;       
    uint32_t valid_len=len-4;
    unsigned char NALU=buffer[4];
     
    //printf("send len=%d\n",len);
 
    //如果数据小于MAX_RTP_PKT_LENGTH字节，直接发送：单一NAL单元模式
    if(valid_len <= MAX_RTP_PKT_LENGTH)
    {
        sendBytes = rtp_session_send_with_ts(session,
                                             &buffer[4],
                                             valid_len,
                                             g_userts);
        return sendBytes;
    }
    else if (valid_len > MAX_RTP_PKT_LENGTH)
    {
        //切分为很多个包发送，每个包前要对头进行处理，如第一个包
        valid_len -= 1;
        int k=0,l=0;
        k=valid_len/MAX_RTP_PKT_LENGTH;
        l=valid_len%MAX_RTP_PKT_LENGTH;
        int t=0;
        int pos=5;
        if(l!=0)
        {
            k=k+1;
        }
        while(t<k)//||(t==k&&l>0))
        {
            if(t<(k-1))//(t<k&&l!=0)||(t<(k-1))&&(l==0))//(0==t)||(t<k&&0!=l))
            {
                buffer[pos-2]=(NALU & 0x60)|28;
                buffer[pos-1]=(NALU & 0x1f);
                if(0==t)
                {
                    buffer[pos-1]|=0x80;
                }
                sendBytes = rtp_session_send_with_ts(session,
                                                     &buffer[pos-2],
                                                     MAX_RTP_PKT_LENGTH+2,
                                                     g_userts);
                t++;
                pos+=MAX_RTP_PKT_LENGTH;
            }
            else //if((k==t&&l>0)||((t==k-1)&&l==0))
            {
                int iSendLen;
                if(l>0)
                {
                    iSendLen=valid_len-t*MAX_RTP_PKT_LENGTH;
                }
                else
                    iSendLen=MAX_RTP_PKT_LENGTH;
                buffer[pos-2]=(NALU & 0x60)|28;
                buffer[pos-1]=(NALU & 0x1f);
                buffer[pos-1]|=0x40;
                sendBytes = rtp_session_send_with_ts(session,
                                                     &buffer[pos-2],
                                                     iSendLen+2,
                                                     g_userts);
                t++;
            }
        }
    }
 
    g_userts += DefaultTimestampIncrement;//timestamp increase
    return  len;
}
*/

int runcond=1;

void stophandler(int signum)
{
	runcond=0;
}

static const char *help="usage: rtpsend	filename dest_ip4addr dest_port [ --with-clockslide <value> ] [ --with-jitter <milliseconds>]\n";

int main(int argc, char *argv[])
{
	RtpSession *session;
	//unsigned char buffer[160];
	int i;
	//FILE *infile;
	char *ssrc;
	//uint32_t user_ts=0;
	int clockslide=0;
	int jitter=0;
	if (argc<4){
		printf("%s", help);
		return -1;
	}
	for(i=4;i<argc;i++){
		if (strcmp(argv[i],"--with-clockslide")==0){
			i++;
			if (i>=argc) {
				printf("%s", help);
				return -1;
			}
			clockslide=atoi(argv[i]);
			ortp_message("Using clockslide of %i milisecond every 50 packets.",clockslide);
		}else if (strcmp(argv[i],"--with-jitter")==0){
			ortp_message("Jitter will be added to outgoing stream.");
			i++;
			if (i>=argc) {
				printf("%s", help);
				return -1;
			}
			jitter=atoi(argv[i]);
		}
	}
	
	ortp_init();
	ortp_scheduler_init();
	ortp_set_log_level_mask(ORTP_MESSAGE|ORTP_WARNING|ORTP_ERROR);
	session=rtp_session_new(RTP_SESSION_SENDONLY);	
	
	rtp_session_set_scheduling_mode(session,1);
	rtp_session_set_blocking_mode(session,1);
	//rtp_session_set_connected_mode(session,TRUE);
	rtp_session_set_remote_addr(session,argv[2],atoi(argv[3]));
	rtp_session_set_payload_type(session,96);
	
	ssrc=getenv("SSRC");
	if (ssrc!=NULL) {
		printf("using SSRC=%i.\n",atoi(ssrc));
		rtp_session_set_ssrc(session,atoi(ssrc));
	}
		

/*
	#ifndef _WIN32
	infile=fopen(argv[1],"r");
	#else
	infile=fopen(argv[1],"rb");
	#endif

	if (infile==NULL) {
		perror("Cannot open file");
		return -1;
	}
*/

	signal(SIGINT,stophandler);

#if 0

	while( ((i=fread(buffer,1,160,infile))>0) && (runcond) )
	{
		rtp_session_send_with_ts(session,buffer,i,user_ts);
		user_ts+=160;
		if (clockslide!=0 && user_ts%(160*50)==0){
			ortp_message("Clock sliding of %i miliseconds now",clockslide);
			rtp_session_make_time_distorsion(session,clockslide);
		}
		/*this will simulate a burst of late packets */
		if (jitter && (user_ts%(8000)==0)) {
			struct timespec pausetime, remtime;
			ortp_message("Simulating late packets now (%i milliseconds)",jitter);
			pausetime.tv_sec=jitter/1000;
			pausetime.tv_nsec=(jitter%1000)*1000000;
			while(nanosleep(&pausetime,&remtime)==-1 && errno==EINTR){
				pausetime=remtime;
			}
		}
	}

#endif

	NALU_HEADER		*nalu_hdr;
	FU_INDICATOR	        *fu_ind;
	FU_HEADER		*fu_hdr;
	char sendbuf[1500];
	char* nalu_payload;
	unsigned int timestamp_increse=3600,ts_current=0;
	int status = 0;

#define ddd
	//< open h264 file
	FILE * bits = open_h264_file(argv[1]);

	NALU_t *n;
	n = nalu_alloc(8000000);//为结构体nalu_t及其成员buf分配空间。返回值为指向nalu_t存储空间的指针

	bool start=false;
	while(!feof(bits))
	{
		int size=get_next_nalu(bits, n);//每执行一次，文件的指针指向本次找到的NALU的末尾，下一个位置即为下个NALU的起始码0x000001
		if(size<4)
		{
			printf("get nul error!\n");
			continue;
		}
		nalu_log(n);
		if(!start)
		{
			if(n->nal_unit_type==5||n->nal_unit_type==6||
					n->nal_unit_type==7||n->nal_unit_type==1)
			{
				printf("begin\n");
				start=true;
			}
		}
		//将编码数据写入文件t
		//fwrite(pNals[i].p_payload, 1, pNals[i].i_payload, pFile);
		//发送编码文件
#if 1
		//	当一个NALU小于MAX_RTP_PKT_LENGTH字节的时候，采用一个单RTP包发送
		if(n->len<=MAX_RTP_PKT_LENGTH)
		{
			//printf("ddd0\n");
			//session.SetDefaultMark(false);
			//设置NALU HEADER,并将这个HEADER填入(char*)sendbuf[12]
			nalu_hdr =(NALU_HEADER*)&sendbuf[0]; //将(char*)sendbuf[12]的地址赋给nalu_hdr，之后对nalu_hdr的写入就将写入(char*)sendbuf中；
			nalu_hdr->F=n->forbidden_bit;
			nalu_hdr->NRI=n->nal_reference_idc>>5;//有效数据在n->nal_reference_idc的第6，7位，需要右移5位才能将其值赋给nalu_hdr->NRI。
			nalu_hdr->TYPE=n->nal_unit_type;

			nalu_payload=&sendbuf[1];//同理将(char*)sendbuf[13]赋给nalu_payload
			memcpy(nalu_payload,n->buf+1,n->len-1);//去掉nalu头的nalu剩余内容写入(char*)sendbuf[13]开始的字符串。
			//ts_current=ts_current+timestamp_increse;

			//status = session.SendPacket((void *)(char*)sendbuf,n->len);
			if(n->nal_unit_type==1 || n->nal_unit_type==5)
			{
				rtp_session_send_with_ts(session,(uint8_t*)sendbuf,n->len,ts_current);
				//printf("0\n");
				//status = session.SendPacket((void *)(char*)sendbuf,n->len,96,true,3600);
			}
			else
			{
				rtp_session_send_with_ts(session,(uint8_t*)sendbuf,n->len,ts_current);
				continue;
				//printf("1\n");
				//status = session.SendPacket((void *)(char*)sendbuf,n->len,96,true,0);
					 //如果是6,7类型的包，不应该延时；之前有停顿，原因这在这
			}
			//发送RTP格式数据包并指定负载类型为96
			if (status < 0)
			{
				//std::cerr << RTPGetErrorString(status) << std::endl;
				exit(-1);
			}

		}
		else if(n->len>MAX_RTP_PKT_LENGTH)
		{
			//得到该nalu需要用多少长度为MAX_RTP_PKT_LENGTH字节的RTP包来发送
			int k=0,l=0;
			k=n->len/MAX_RTP_PKT_LENGTH;//需要k个MAX_RTP_PKT_LENGTH字节的RTP包
			l=n->len%MAX_RTP_PKT_LENGTH;//最后一个RTP包的需要装载的字节数
			int t=0;//用于指示当前发送的是第几个分片RTP包
			//ts_current=ts_current+timestamp_increse;
			while(t<=k)
			{

				if(!t)//发送一个需要分片的NALU的第一个分片，置FU HEADER的S位
				{
					//printf("dddd1");
					memset((char*)sendbuf,0,1500);
					//session.SetDefaultMark(false);
					//设置FU INDICATOR,并将这个HEADER填入(char*)sendbuf[12]
					fu_ind =(FU_INDICATOR*)&sendbuf[0]; //将(char*)sendbuf[12]的地址赋给fu_ind，之后对fu_ind的写入就将写入(char*)sendbuf中；
					fu_ind->F=n->forbidden_bit;
					fu_ind->NRI=n->nal_reference_idc>>5;
					fu_ind->TYPE=28;

					//设置FU HEADER,并将这个HEADER填入(char*)sendbuf[13]
					fu_hdr =(FU_HEADER*)&sendbuf[1];
					fu_hdr->E=0;
					fu_hdr->R=0;
					fu_hdr->S=1;
					fu_hdr->TYPE=n->nal_unit_type;


					nalu_payload=&sendbuf[2];//同理将(char*)sendbuf[14]赋给nalu_payload
					memcpy(nalu_payload,n->buf+1,MAX_RTP_PKT_LENGTH);//去掉NALU头

					//status = session.SendPacket((void *)(char*)sendbuf,MAX_RTP_PKT_LENGTH+2);
					//status = session.SendPacket((void *)(char*)sendbuf,MAX_RTP_PKT_LENGTH+2,96,false,0);
					rtp_session_send_with_ts(session,(uint8_t*)sendbuf,MAX_RTP_PKT_LENGTH+2,ts_current);
					//printf("2\n");
					if (status < 0)
					{
						//std::cerr << RTPGetErrorString(status) << std::endl;
						exit(-1);
					}
					t++;
				}
				//发送一个需要分片的NALU的非第一个分片，清零FU HEADER的S位，如果该分片是该NALU的最后一个分片，置FU HEADER的E位
				else if(k==t)//发送的是最后一个分片，注意最后一个分片的长度可能超过MAX_RTP_PKT_LENGTH字节（当l>1386时）。
				{
					//printf("dddd3\n");
					memset((char*)sendbuf,0,1500);
					//session.SetDefaultMark(true);
					//设置FU INDICATOR,并将这个HEADER填入(char*)sendbuf[12]
					fu_ind =(FU_INDICATOR*)&sendbuf[0]; //将(char*)sendbuf[12]的地址赋给fu_ind，之后对fu_ind的写入就将写入(char*)sendbuf中；
					fu_ind->F=n->forbidden_bit;
					fu_ind->NRI=n->nal_reference_idc>>5;
					fu_ind->TYPE=28;

					//设置FU HEADER,并将这个HEADER填入(char*)sendbuf[13]
					fu_hdr =(FU_HEADER*)&sendbuf[1];
					fu_hdr->R=0;
					fu_hdr->S=0;
					fu_hdr->TYPE=n->nal_unit_type;
					fu_hdr->E=1;
					nalu_payload=&sendbuf[2];//同理将(char*)sendbuf[14]赋给nalu_payload
					memcpy(nalu_payload,n->buf+t*MAX_RTP_PKT_LENGTH+1,l-1);//将nalu最后剩余的l-1(去掉了一个字节的NALU头)字节内容写入(char*)sendbuf[14]开始的字符串。

					//status = session.SendPacket((void *)(char*)sendbuf,l+1);
					//status = session.SendPacket((void *)(char*)sendbuf,l+1,96,true,3600);
					rtp_session_send_with_ts(session,(uint8_t*)sendbuf,1+1,ts_current);
					//printf("3\n");
					if (status < 0)
					{
						//std::cerr << RTPGetErrorString(status) << std::endl;
						exit(-1);
					}
					t++;
					//	Sleep(100);
				}
				else if(t<k&&0!=t)
				{
					//printf("dddd2");
					memset((char*)sendbuf,0,1500);
					//session.SetDefaultMark(false);
					//设置FU INDICATOR,并将这个HEADER填入(char*)sendbuf[12]
					fu_ind =(FU_INDICATOR*)&sendbuf[0]; //将(char*)sendbuf[12]的地址赋给fu_ind，之后对fu_ind的写入就将写入(char*)sendbuf中；
					fu_ind->F=n->forbidden_bit;
					fu_ind->NRI=n->nal_reference_idc>>5;
					fu_ind->TYPE=28;

					//设置FU HEADER,并将这个HEADER填入(char*)sendbuf[13]
					fu_hdr =(FU_HEADER*)&sendbuf[1];
					//fu_hdr->E=0;
					fu_hdr->R=0;
					fu_hdr->S=0;
					fu_hdr->E=0;
					fu_hdr->TYPE=n->nal_unit_type;

					nalu_payload=&sendbuf[2];//同理将(char*)sendbuf[14]的地址赋给nalu_payload
					memcpy(nalu_payload,n->buf+t*MAX_RTP_PKT_LENGTH+1,MAX_RTP_PKT_LENGTH);//去掉起始前缀的nalu剩余内容写入(char*)sendbuf[14]开始的字符串。

					//status = session.SendPacket((void *)(char*)sendbuf,MAX_RTP_PKT_LENGTH+2);
					//status = session.SendPacket((void *)(char*)sendbuf,MAX_RTP_PKT_LENGTH+2,96,false,0);
					rtp_session_send_with_ts(session,(uint8_t*)sendbuf,MAX_RTP_PKT_LENGTH+2,ts_current);
					//printf("4\n");
					if (status < 0)
					{
						//std::cerr << RTPGetErrorString(status) << std::endl;
						exit(-1);
					}
					t++;
				}
			}
		}

#endif

#if 0
		session.BeginDataAccess();
		if (session.GotoFirstSource())
		{
			do
			{
				RTPPacket *packet;

				while ((packet = session.GetNextPacket()) != 0)
				{
					std::cout << "Got packet with "
						<< "extended sequence number "
						<< packet->GetExtendedSequenceNumber()
						<< " from SSRC " << packet->GetSSRC()
						<< std::endl;
					session.DeletePacket(packet);
				}
			} while (session.GotoNextSource());
		}
		session.EndDataAccess();
#endif
		/*
		RTPTime::Wait(delay);

		RTPTime t = RTPTime::CurrentTime();
		t -= starttime;
		if (t > RTPTime(60.0))
			break;
		*/
		struct timespec pausetime, remtime;
		pausetime.tv_sec = 0;
		pausetime.tv_nsec = 40000;
		while(nanosleep(&pausetime, &remtime) == -1 && errno == EINTR){
			pausetime = remtime;
		}

		ts_current=ts_current+timestamp_increse;
	}

	printf("over\n");

	fclose(bits);
	rtp_session_destroy(session);
	ortp_exit();
	ortp_global_stats_display();

	nalu_free(n);

	return 0;
}


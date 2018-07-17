
Send h264 stream by RTP protocol
================================

dependency : ORTP-0.25 (Or other rtp library by manually migration.)

test environment : 4.17.3-1-ARCH

## detail of h264 & rtp

h264 file format : |startcode[3-4B]|nalu hdr[1B]|RBSP[nB]| ...

rtp packet payload : 

one nalu one packet : |nalu hdr[1B]|RBSP[nB]|

one nalu multi packet : |fu ind[1B]|fu hdr[1B]|RBSP[nB]| (start, inner and end packet)


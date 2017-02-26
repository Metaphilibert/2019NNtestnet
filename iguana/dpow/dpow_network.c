/******************************************************************************
 * Copyright © 2014-2017 The SuperNET Developers.                             *
 *                                                                            *
 * See the AUTHORS, DEVELOPER-AGREEMENT and LICENSE files at                  *
 * the top-level directory of this distribution for the individual copyright  *
 * holder information and the developer policies on copyright and licensing.  *
 *                                                                            *
 * Unless otherwise agreed in a custom licensing agreement, no part of the    *
 * SuperNET software, including this file may be copied, modified, propagated *
 * or distributed except according to the terms contained in the LICENSE file *
 *                                                                            *
 * Removal or modification of this copyright notice is prohibited.            *
 *                                                                            *
 ******************************************************************************/

struct signed_nnpacket
{
    uint8_t sig64[64];
    bits256 packethash;
    uint32_t nonce,packetlen;
    uint8_t packet[];
} PACKED;

/*
 NN_CONNECT to (tcp://167.114.118.5:7775)
NN_CONNECT to (tcp://10.0.0.4:7775)
NN_CONNECT to (tcp://85.143.211.6:7775)
NN_CONNECT to (tcp://158.69.25.7:7775)
NN_CONNECT to (tcp://123.249.79.12:7775)
NN_CONNECT to (tcp://141.105.66.18:7775)
NN_CONNECT to (tcp://198.27.75.27:7775)
NN_CONNECT to (tcp://198.204.226.34:7775)
NN_CONNECT to (tcp://144.76.94.38:7775)
NN_CONNECT to (tcp://142.54.174.42:7775)
NN_CONNECT to (tcp://88.198.70.43:7775)
NN_CONNECT to (tcp://182.162.169.44:7775)
NN_CONNECT to (tcp://104.168.128.50:7775)
NN_CONNECT to (tcp://167.114.64.64:7775)
NN_CONNECT to (tcp://185.169.229.64:7775)
NN_CONNECT to (tcp://46.4.68.72:7775)
NN_CONNECT to (tcp://173.208.203.74:7775)
NN_CONNECT to (tcp://192.99.232.81:7775)
NN_CONNECT to (tcp://149.56.28.84:7775)
NN_CONNECT to (tcp://148.251.190.89:7775)
NN_CONNECT to (tcp://149.56.240.91:7775)
NN_CONNECT to (tcp://88.99.56.99:7775)
NN_CONNECT to (tcp://217.106.238.109:7775)
NN_CONNECT to (tcp://142.54.164.114:7775)
NN_CONNECT to (tcp://163.172.101.118:7775)
NN_CONNECT to (tcp://189.1.174.119:7775)
NN_CONNECT to (tcp://1.234.19.123:7775)
NN_CONNECT to (tcp://221.121.144.138:7775)
NN_CONNECT to (tcp://46.166.168.138:7775)
NN_CONNECT to (tcp://221.121.144.140:7775)
NN_CONNECT to (tcp://163.172.100.144:7775)
NN_CONNECT to (tcp://185.106.122.147:7775)
NN_CONNECT to (tcp://103.18.58.150:7775)
NN_CONNECT to (tcp://23.88.234.153:7775)
NN_CONNECT to (tcp://164.132.202.176:7775)
NN_CONNECT to (tcp://178.63.85.196:7775)
NN_CONNECT to (tcp://69.12.77.197:7775)
NN_CONNECT to (tcp://192.157.238.198:7775)
NN_CONNECT to (tcp://209.58.183.199:7775)
NN_CONNECT to (tcp://149.202.65.200:7775)
NN_CONNECT to (tcp://173.208.184.202:7775)
NN_CONNECT to (tcp://94.102.63.208:7775)
NN_CONNECT to (tcp://5.9.109.208:7775)
NN_CONNECT to (tcp://94.102.63.209:7775)
NN_CONNECT to (tcp://197.189.248.210:7775)
NN_CONNECT to (tcp://149.56.19.212:7775)
NN_CONNECT to (tcp://46.165.243.214:7775)
NN_CONNECT to (tcp://45.64.168.216:7775)
*/

void dex_init(struct supernet_info *myinfo)
{
    int32_t i,j,mask = 0; char *seeds[] = { "78.47.196.146", "5.9.102.210", "149.56.29.163", "191.235.80.138", "88.198.65.74", "94.102.63.226", "129.232.225.202", "104.255.64.3", "52.72.135.200", "149.56.28.84", "103.18.58.150", "221.121.144.140", "123.249.79.12", "103.18.58.146", "27.50.93.252", "176.9.0.233", "94.102.63.227", "167.114.227.223", "27.50.68.219", "192.99.233.217", "94.102.63.217", "45.64.168.216" };
    OS_randombytes((void *)&i,sizeof(i));
    srand(i);
    for (i=0; i<sizeof(myinfo->dexseed_ipaddrs)/sizeof(*myinfo->dexseed_ipaddrs); i++)
    {
        while ( 1 )
        {
            j = (rand() % (sizeof(seeds)/sizeof(*seeds)));
            if ( i < 2 )
                j = i;
            if ( ((1 << j) & mask) == 0 )
                break;
        }
        mask |= (1 << j);
        printf("seed.[%d] <- %s\n",i,seeds[j]);
        strcpy(myinfo->dexseed_ipaddrs[i],seeds[j]);
        myinfo->dexipbits[i] = (uint32_t)calc_ipbits(myinfo->dexseed_ipaddrs[i]);
    }
    myinfo->numdexipbits = i;
    portable_mutex_init(&myinfo->dexmutex);
}

int32_t signed_nn_send(struct supernet_info *myinfo,void *ctx,bits256 privkey,int32_t sock,void *packet,int32_t size)
{
    int32_t i,j,sentbytes,siglen = 0; uint8_t sig[65],pubkey33[33],signpubkey33[33]; struct signed_nnpacket *sigpacket;
    if ( (sigpacket= calloc(1,size + sizeof(*sigpacket))) != 0 )
    {
        sigpacket->packetlen = size;
        memcpy(sigpacket->packet,packet,size);
        for (i=0; i<10000; i++)
        {
            sigpacket->nonce = i;
            vcalc_sha256(0,sigpacket->packethash.bytes,(void *)&sigpacket->nonce,(int32_t)(size+sizeof(sigpacket->nonce)+sizeof(sigpacket->packetlen)));
            if ( sigpacket->packethash.bytes[0] == 0 )
                break;
        }
        bitcoin_pubkey33(ctx,signpubkey33,privkey);
        for (j=0; j<33; j++)
        {
            if ( i < 10000 && (siglen= bitcoin_sign(ctx,"nnsend",sig,sigpacket->packethash,privkey,1)) > 0 && siglen == 65 )
            {
                memcpy(sigpacket->sig64,sig+1,64);
                if ( bitcoin_recoververify(ctx,"nnrecv",sigpacket->sig64,sigpacket->packethash,pubkey33,33) == 0 )
                {
                    //for (i=0; i<33; i++)
                    //    printf("%02x",pubkey33[i]);
                    //printf(" signed pubkey\n");
                    if ( memcmp(pubkey33,signpubkey33,33) == 0 )
                    {
                        sentbytes = nn_send(sock,sigpacket,size + sizeof(*sigpacket),0);
                        //for (i=0; i<size+sizeof(*sigpacket); i++)
                        //    printf("%02x",((uint8_t *)sigpacket)[i]);
                        //printf(" <- nnsend\n");
                        free(sigpacket);
                        return(sentbytes - siglen);
                    }
                }
            }
        }
        free(sigpacket);
        iguana_fixsecp(myinfo);
        printf("error signing nnpacket\n");
    }
    return(-1);
}

int32_t signed_nn_recv(void **freeptrp,void *ctx,uint8_t notaries[64][33],int32_t n,int32_t sock,void *packetp)
{
    int32_t i,recvbytes; uint8_t pubkey33[33],pubkey0[33]; bits256 packethash; struct signed_nnpacket *sigpacket=0;
    *(void **)packetp = 0;
    *freeptrp = 0;
    if ( (recvbytes= nn_recv(sock,&sigpacket,NN_MSG,0)) > 0 )
    {
        //for (i=0; i<recvbytes; i++)
        //    printf("%02x",((uint8_t *)sigpacket)[i]);
        //printf(" <- RECV.%d crc.%08x cmp.%d\n",recvbytes,calc_crc32(0,(void *)sigpacket,recvbytes),sigpacket->packetlen == recvbytes-sizeof(*sigpacket));
    }
    if ( sigpacket != 0 && recvbytes > sizeof(*sigpacket) && sigpacket->packetlen == recvbytes-sizeof(*sigpacket) )
    {
        vcalc_sha256(0,packethash.bytes,(void *)&sigpacket->nonce,(int32_t)(sigpacket->packetlen+sizeof(sigpacket->nonce)+sizeof(sigpacket->packetlen)));
        if ( bits256_cmp(packethash,sigpacket->packethash) == 0 && sigpacket->packethash.bytes[0] == 0 )
        {
            if ( bitcoin_recoververify(ctx,"nnrecv",sigpacket->sig64,sigpacket->packethash,pubkey33,33) == 0 )
            {
                char *notary0 = "03b7621b44118017a16043f19b30cc8a4cfe068ac4e42417bae16ba460c80f3828";
                // expand to official notaries
                decode_hex(pubkey0,33,notary0);
                if ( memcmp(pubkey0,pubkey33,33) == 0 )
                {
                    *(void **)packetp = (void **)((uint64_t)sigpacket + sizeof(*sigpacket));
                    *freeptrp = sigpacket;
                    //printf("got signed packet from notary0\n");
                    return((int32_t)(recvbytes - sizeof(*sigpacket)));
                }
                for (i=0; i<n && i<64; i++)
                {
                    if ( memcmp(pubkey33,notaries[i],33) == 0 )
                    {
                        *(void **)packetp = (void **)((uint64_t)sigpacket + sizeof(*sigpacket));
                        //printf("got signed packet from notary.%d\n",i);
                        *freeptrp = sigpacket;
                        return((int32_t)(recvbytes - sizeof(*sigpacket)));
                    }
                    if ( 0 && i < 2 )
                    {
                        int32_t j;
                        for (j=0; j<33; j++)
                            printf("%02x",notaries[i][j]);
                        printf(" pubkey[%d]\n",i);
                    }
                }
                //for (i=0; i<33; i++)
                //    printf("%02x",pubkey33[i]);
                //printf(" invalid pubkey33 n.%d\n",n);
            } else printf("recoververify error nonce.%u packetlen.%d\n",sigpacket->nonce,sigpacket->packetlen);
        } else printf("hash mismatch or bad nonce.%u packetlen.%d\n",sigpacket->nonce,sigpacket->packetlen);
    } else if ( recvbytes > 0 )
        printf("recvbytes.%d mismatched packetlen.%d + %ld\n",recvbytes,sigpacket!=0?sigpacket->packetlen:-1,sizeof(*sigpacket));
    //printf("free sigpacket.%p freeptrp.%p packetp.%p\n",sigpacket,*freeptrp,*(void **)packetp);
    if ( sigpacket != 0 )
        nn_freemsg(sigpacket), sigpacket = 0;
    *freeptrp = sigpacket;
    *(void **)packetp = sigpacket;
    return(0);
}

struct dex_nanomsghdr
{
    uint32_t crc32,size,datalen,timestamp;
    char handler[8];
    uint8_t version0,version1,packet[];
} PACKED;

struct dex_request { bits256 hash; int32_t intarg; uint16_t shortarg; char name[15]; uint8_t func; };

int32_t dex_rwrequest(int32_t rwflag,uint8_t *serialized,struct dex_request *dexreq)
{
    int32_t len = 0;
    len += iguana_rwbignum(rwflag,&serialized[len],sizeof(dexreq->hash),dexreq->hash.bytes);
    len += iguana_rwnum(rwflag,&serialized[len],sizeof(dexreq->intarg),&dexreq->intarg);
    len += iguana_rwnum(rwflag,&serialized[len],sizeof(dexreq->shortarg),&dexreq->shortarg);
    if ( rwflag != 0 )
    {
        memcpy(&serialized[len],dexreq->name,sizeof(dexreq->name)), len += sizeof(dexreq->name);
        serialized[len++] = dexreq->func;
    }
    else
    {
        memcpy(dexreq->name,&serialized[len],sizeof(dexreq->name)), len += sizeof(dexreq->name);
        dexreq->func = serialized[len++];
    }
    return(len);
}

char *nanomsg_tcpname(struct supernet_info *myinfo,char *str,char *ipaddr,uint16_t port)
{
    if ( myinfo != 0 ) // bind path)
    {
        if ( myinfo->bindaddr[0] != 0 && strcmp(ipaddr,myinfo->ipaddr) == 0 )
            ipaddr = myinfo->bindaddr;
    }
    sprintf(str,"tcp://%s:%u",ipaddr,port);
    return(str);
}

static int _increasing_ipbits(const void *a,const void *b)
{
#define uint32_a (*(uint32_t *)a)
#define uint32_b (*(uint32_t *)b)
	if ( uint32_b > uint32_a )
		return(-1);
	else if ( uint32_b < uint32_a )
		return(1);
	return(0);
#undef uint32_a
#undef uint32_b
}

void dex_packet(struct supernet_info *myinfo,struct dex_nanomsghdr *dexp,int32_t size)
{
    char *retstr; int32_t datalen; struct iguana_info *coin; struct dex_request dexreq;
    //for (i=0; i<size; i++)
    //    printf("%02x",((uint8_t *)dexp)[i]);
    //printf(" uniq.%s DEX_PACKET.[%d] crc.%x lag.%d (%d %d)\n",dexp->handler,size,calc_crc32(0,dexp->packet,dexp->datalen),(int32_t)(time(NULL)-dexp->timestamp),dexp->size,dexp->datalen);
    if ( strcmp(dexp->handler,"DEX") == 0 && dexp->datalen > BASILISK_KEYSIZE )
    {
        if ( (retstr= basilisk_respond_addmessage(myinfo,dexp->packet,BASILISK_KEYSIZE,&dexp->packet[BASILISK_KEYSIZE],dexp->datalen-BASILISK_KEYSIZE,0,BASILISK_DEXDURATION)) != 0 )
            free(retstr);
    }
    else if ( strcmp(dexp->handler,"request") == 0 )
    {
        datalen = dex_rwrequest(0,dexp->packet,&dexreq);
        if ( myinfo->IAMNOTARY != 0 && dexreq.func == 'A' && (coin= iguana_coinfind(dexreq.name)) != 0 )
        {
            if ( (retstr= dpow_importaddress(myinfo,coin,(char *)&dexp->packet[datalen])) != 0 )
                free(retstr);
            printf("process broadcast importaddress.(%s) [%s]\n",(char *)&dexp->packet[datalen],dexreq.name);
        }
    }
}

char *_dex_reqsend(struct supernet_info *myinfo,char *handler,uint8_t *key,int32_t keylen,uint8_t *data,int32_t datalen)
{
    struct dex_nanomsghdr *dexp; cJSON *retjson; char ipaddr[64],str[128]; int32_t timeout,i,n,size,recvbytes,sentbytes = 0,reqsock,subsock; uint32_t *retptr,ipbits; void *freeptr; char *retstr = 0;
    portable_mutex_lock(&myinfo->dexmutex);
    subsock = myinfo->subsock;
    reqsock = myinfo->reqsock;
    if ( reqsock < 0 && (reqsock= nn_socket(AF_SP,NN_REQ)) >= 0 )
    {
       if ( reqsock >= 0 )
       {
           timeout = 1000;
           nn_setsockopt(reqsock,NN_SOL_SOCKET,NN_SNDTIMEO,&timeout,sizeof(timeout));
           //timeout = 5000;
           //nn_setsockopt(reqsock,NN_TCP,NN_RECONNECT_IVL,&timeout,sizeof(timeout));
           timeout = 1000;
           nn_setsockopt(reqsock,NN_SOL_SOCKET,NN_RCVTIMEO,&timeout,sizeof(timeout));
           for (i=0; i<sizeof(myinfo->dexseed_ipaddrs)/sizeof(*myinfo->dexseed_ipaddrs); i++)
               if ( nn_connect(reqsock,nanomsg_tcpname(0,str,myinfo->dexseed_ipaddrs[i],REP_SOCK)) < 0 )
               {
                   nn_close(reqsock);
                   reqsock = -1;
                   break;
               }
        }
        if ( reqsock >= 0 )
        {
            if ( myinfo->IAMNOTARY == 0 && subsock < 0 && (subsock= nn_socket(AF_SP,NN_SUB)) >= 0 )
            {
                for (i=0; i<sizeof(myinfo->dexseed_ipaddrs)/sizeof(*myinfo->dexseed_ipaddrs); i++)
                    if ( nn_connect(subsock,nanomsg_tcpname(0,str,myinfo->dexseed_ipaddrs[i],PUB_SOCK)) < 0 )
                    {
                        nn_close(reqsock);
                        reqsock = -1;
                        nn_close(subsock);
                        subsock = -1;
                        break;
                    }
                if ( reqsock >= 0 && subsock >= 0 )
                {
                    timeout = 100;
                    nn_setsockopt(subsock,NN_SOL_SOCKET,NN_RCVTIMEO,&timeout,sizeof(timeout));
                    nn_setsockopt(subsock,NN_SUB,NN_SUB_SUBSCRIBE,"",0);
                    printf("CLIENT sockets req.%d sub.%d\n",reqsock,subsock);
                    //timeout = 5000;
                    //nn_setsockopt(reqsock,NN_TCP,NN_RECONNECT_IVL,&timeout,sizeof(timeout));
                    timeout = 10000;
                    nn_setsockopt(reqsock,NN_SOL_SOCKET,NN_RCVTIMEO,&timeout,sizeof(timeout));
                }
            }
        }
    }
    if ( myinfo->subsock != subsock )
        myinfo->subsock = subsock;
    if ( myinfo->reqsock != reqsock )
        myinfo->reqsock = reqsock;
    portable_mutex_unlock(&myinfo->dexmutex);
    if ( myinfo->reqsock >= 0 )
    {
        size = (int32_t)(sizeof(*dexp) + keylen + datalen);
        dexp = calloc(1,size); // endian dependent!
        safecopy(dexp->handler,handler,sizeof(dexp->handler));
        dexp->size = size;
        dexp->datalen = datalen + keylen;
        dexp->timestamp = (uint32_t)time(NULL);
        dexp->version0 = DEX_VERSION & 0xff;
        dexp->version1 = (DEX_VERSION >> 8) & 0xff;
        if ( key != 0 && keylen != 0 )
        {
            memcpy(dexp->packet,key,keylen);
            memcpy(&dexp->packet[keylen],data,datalen);
            dexp->crc32 = calc_crc32(calc_crc32(0,key,keylen),data,datalen);
        }
        else
        {
            memcpy(dexp->packet,data,datalen);
            dexp->crc32 = calc_crc32(0,data,datalen);
        }
        for (i=0; i<100; i++)
        {
            struct nn_pollfd pfd;
            pfd.fd = myinfo->reqsock;
            pfd.events = NN_POLLOUT;
            if ( nn_poll(&pfd,1,100) > 0 )
            {
                sentbytes = nn_send(myinfo->reqsock,dexp,size,0);
                //printf(" sent.%d:%d datalen.%d crc.%08x\n",sentbytes,size,datalen,calc_crc32(0,(void *)dexp,size));
                break;
            }
            usleep(1000);
        }
        //for (i=0; i<datalen; i++)
        //    printf("%02x",((uint8_t *)data)[i]);
        if ( (recvbytes= signed_nn_recv(&freeptr,myinfo->ctx,myinfo->notaries,myinfo->numnotaries,myinfo->reqsock,&retptr)) >= 0 )
        {
            //printf("req returned.[%d]\n",recvbytes);
            portable_mutex_lock(&myinfo->dexmutex);
            ipbits = 0;
            if ( strcmp(handler,"DEX") == 0 )
            {
                if ( retptr != 0 )
                    ipbits = *retptr;
            }
            else if ( retptr != 0 )
            {
                retstr = clonestr((char *)retptr);
                //printf("GOT.(%s)\n",retstr);
                if ( (retjson= cJSON_Parse(retstr)) != 0 )
                {
                    ipbits = juint(retjson,"randipbits");
                    free_json(retjson);
                    if ( 0 && ipbits != 0 )
                        printf("GOT randipbits.%08x\n",ipbits);
                }
            }
            if ( ipbits != 0 )
            {
                expand_ipbits(ipaddr,ipbits);
                n = myinfo->numdexipbits;
                for (i=0; i<n; i++)
                    if ( ipbits == myinfo->dexipbits[i] )
                        break;
                if ( i == n && n < 64 )
                {
                    myinfo->dexipbits[n++] = ipbits;
                    qsort(myinfo->dexipbits,n,sizeof(uint32_t),_increasing_ipbits);
                    if ( (myinfo->numdexipbits= n) < 3 )
                    {
                        if ( myinfo->IAMNOTARY == 0 && myinfo->subsock >= 0 )
                        {
                            nn_connect(myinfo->subsock,nanomsg_tcpname(0,str,ipaddr,PUB_SOCK));
                            printf("%d: subscribe connect (%s)\n",myinfo->numdexipbits,str);
                        }
                    }
#ifndef __APPLE__
                    if ( (rand() % 100) < 40 )
                    {
                        nanomsg_tcpname(0,str,ipaddr,REP_SOCK);
                        nn_connect(myinfo->reqsock,str);
                        printf("%d: req connect (%s)\n",myinfo->numdexipbits,str);
                    }
#endif
                }
            }
            if ( freeptr != 0 )
                nn_freemsg(freeptr), freeptr = 0, retptr = 0;
            portable_mutex_unlock(&myinfo->dexmutex);
        }
        else
        {
            //retval = -2;
            printf("no rep return? recvbytes.%d\n",recvbytes);
        }
        //printf("DEXREQ.[%d] crc32.%08x datalen.%d sent.%d recv.%d timestamp.%u\n",size,dexp->crc32,datalen,sentbytes,recvbytes,dexp->timestamp);
        free(dexp);
    } //else retval = -1;
    return(retstr);
}

void dex_channelsend(struct supernet_info *myinfo,bits256 srchash,bits256 desthash,uint32_t channel,uint32_t msgid,uint8_t *data,int32_t datalen)
{
    int32_t keylen; uint8_t key[BASILISK_KEYSIZE]; char *retstr;
    keylen = basilisk_messagekey(key,channel,msgid,srchash,desthash);
    if ( (retstr= _dex_reqsend(myinfo,"DEX",key,keylen,data,datalen)) != 0 )
        free(retstr);
}

void dpow_randipbits(struct supernet_info *myinfo,struct iguana_info *coin,cJSON *retjson)
{
    int32_t m; uint32_t ipbits; char *coinstr;
    if ( is_cJSON_Array(retjson) == 0 )
    {
        if ( (m= myinfo->numdpowipbits) > 0 )
        {
            ipbits = myinfo->dpowipbits[(uint32_t)rand() % m];
            jaddnum(retjson,"randipbits",ipbits);
            //printf("add randipbits.%08x\n",ipbits);
        }
        if ( (coinstr= jstr(retjson,"coin")) == 0 )
            jaddstr(retjson,"coin",coin->symbol);
    }
}

char *dex_response(int32_t *broadcastflagp,struct supernet_info *myinfo,struct dex_nanomsghdr *dexp)
{
    char buf[65],*retstr = 0; int32_t i,datalen; bits256 hash2; cJSON *retjson=0; struct iguana_info *coin; struct dex_request dexreq;
    *broadcastflagp = 0;
    if ( strcmp(dexp->handler,"request") == 0 )
    {
        datalen = dex_rwrequest(0,dexp->packet,&dexreq);
        //printf("dex_response.%s (%c)\n",dexreq.name,dexreq.func);
        if ( (coin= iguana_coinfind(dexreq.name)) != 0 )
        {
            if ( dexreq.func == 'T' )
            {
                if ( (retjson= dpow_gettransaction(myinfo,coin,dexreq.hash)) != 0 )
                {
                    dpow_randipbits(myinfo,coin,retjson);
                    retstr = jprint(retjson,1);
                }
            }
            else if ( dexreq.func == 'O' )
            {
                if ( (retjson= dpow_gettxout(myinfo,coin,dexreq.hash,dexreq.shortarg)) != 0 )
                {
                    dpow_randipbits(myinfo,coin,retjson);
                    retstr = jprint(retjson,1);
                }
            }
            else if ( dexreq.func == 'x' )
            {
                if ( (retjson= dpow_gettxin(myinfo,coin,dexreq.hash,dexreq.shortarg)) != 0 )
                {
                    dpow_randipbits(myinfo,coin,retjson);
                    retstr = jprint(retjson,1);
                }
            }
            else if ( dexreq.func == 'H' )
            {
                hash2 = dpow_getblockhash(myinfo,coin,dexreq.intarg);
                //printf("getblockhash %d -> (%s)\n",dexreq.intarg,bits256_str(buf,hash2));
                bits256_str(buf,hash2);
                retstr = clonestr(buf);
            }
            else if ( dexreq.func == 'B' )
            {
                if ( (retjson= dpow_getblock(myinfo,coin,dexreq.hash)) != 0 )
                {
                    dpow_randipbits(myinfo,coin,retjson);
                    retstr = jprint(retjson,1);
                }
            }
            else if ( dexreq.func == 'b' )
            {
                if ( (retjson= dpow_getbalance(myinfo,coin,(char *)&dexp->packet[datalen])) != 0 )
                {
                    dpow_randipbits(myinfo,coin,retjson);
                    retstr = jprint(retjson,1);
                }
            }
            else if ( dexreq.func == 'I' )
            {
                if ( (retjson= dpow_getinfo(myinfo,coin)) != 0 )
                {
                    dpow_randipbits(myinfo,coin,retjson);
                    retstr = jprint(retjson,1);
                }
            }
            else if ( dexreq.func == 'k' )
            {
                if ( strcmp(coin->symbol,"BTC") == 0 || strcmp(coin->symbol,"ZEC") == 0 || coin->chain->zcash == 0 )
                    retstr = clonestr("{\"error\":\"only komodod chains support KV\"}");
                else if ( (retjson= dpow_kvsearch(myinfo,coin,(char *)&dexp->packet[datalen])) != 0 )
                {
                    dpow_randipbits(myinfo,coin,retjson);
                    retstr = jprint(retjson,1);
                }
            }
            else if ( dexreq.func == 'K' )
            {
                if ( strcmp(coin->symbol,"BTC") == 0 || strcmp(coin->symbol,"ZEC") == 0 || coin->chain->zcash == 0 )
                    retstr = clonestr("{\"error\":\"only komodod chains support KV\"}");
                else if ( (retjson= dpow_kvupdate(myinfo,coin,(char *)&dexp->packet[datalen],(char *)&dexp->packet[datalen+dexreq.shortarg],dexreq.intarg)) != 0 )
                {
                    dpow_randipbits(myinfo,coin,retjson);
                    retstr = jprint(retjson,1);
                }
            }
            else if ( dexreq.func == 'U' )
            {
                if ( (retjson= dpow_listunspent(myinfo,coin,(char *)&dexp->packet[datalen])) != 0 )
                {
                    dpow_randipbits(myinfo,coin,retjson);
                    retstr = jprint(retjson,1);
                }
            }
            else if ( dexreq.func == 'u' )
            {
                if ( (retjson= kmd_listunspent(myinfo,coin,(char *)&dexp->packet[datalen])) != 0 )
                {
                    dpow_randipbits(myinfo,coin,retjson);
                    retstr = jprint(retjson,1);
                    //printf("RETURN.(%s)\n",retstr);
                }
            }
            else if ( dexreq.func == 's' )
            {
                if ( (retjson= dpow_listspent(myinfo,coin,(char *)&dexp->packet[datalen])) != 0 )
                {
                    dpow_randipbits(myinfo,coin,retjson);
                    retstr = jprint(retjson,1);
                }
            }
            else if ( dexreq.func == 'P' )
            {
                hash2 = dpow_getbestblockhash(myinfo,coin);
                bits256_str(buf,hash2);
                retstr = clonestr(buf);
            }
            else if ( dexreq.func == 'S' )
            {
                retstr = dpow_sendrawtransaction(myinfo,coin,(char *)&dexp->packet[datalen]);
            }
            else if ( dexreq.func == '*' )
            {
                retstr = dpow_alladdresses(myinfo,coin);
            }
            else if ( dexreq.func == 'L' )
            {
                //printf("call list.(%s %d %d)\n",(char *)&dexp->packet[datalen],dexreq.shortarg,dexreq.intarg);
                if ( (retjson= dpow_listtransactions(myinfo,coin,(char *)&dexp->packet[datalen],dexreq.shortarg,dexreq.intarg)) != 0 )
                {
                    dpow_randipbits(myinfo,coin,retjson);
                    retstr = jprint(retjson,1);
                }
            }
            else if ( dexreq.func == '2' )
            {
                //printf("call list.(%s %d %d)\n",(char *)&dexp->packet[datalen],dexreq.shortarg,dexreq.intarg);
                if ( (retjson= kmd_listtransactions(myinfo,coin,(char *)&dexp->packet[datalen],dexreq.shortarg,dexreq.intarg)) != 0 )
                {
                    dpow_randipbits(myinfo,coin,retjson);
                    retstr = jprint(retjson,1);
                }
            }
            else if ( dexreq.func == 'C' )
            {
                if ( (retjson= dpow_checkaddress(myinfo,coin,(char *)&dexp->packet[datalen])) != 0 )
                {
                    dpow_randipbits(myinfo,coin,retjson);
                    retstr = jprint(retjson,1);
                }
            }
            else if ( dexreq.func == 'A' )
            {
                retstr = dpow_importaddress(myinfo,coin,(char *)&dexp->packet[datalen]);
                if ( retstr == 0 )
                {
                    *broadcastflagp = 1;
                    retstr = dpow_validateaddress(myinfo,coin,(char *)&dexp->packet[datalen]);
                }
                else
                {
                    printf("funcA.(%s)\n",retstr);
                }
                if ( retstr != 0 && (retjson= cJSON_Parse(retstr)) != 0 )
                {
                    dpow_randipbits(myinfo,coin,retjson);
                    free(retstr);
                    retstr = jprint(retjson,1);
                }
            }
            else if ( dexreq.func == 'V' )
            {
                retstr = dpow_validateaddress(myinfo,coin,(char *)&dexp->packet[datalen]);
                if ( retstr != 0 && (retjson= cJSON_Parse(retstr)) != 0 )
                {
                    dpow_randipbits(myinfo,coin,retjson);
                    free(retstr);
                    retstr = jprint(retjson,1);
                }
            }
            else if ( dexreq.func == 'N' )
            {
                uint8_t pubkeys[64][33]; char str[128]; int32_t numnotaries; cJSON *array,*item;
                if ( (numnotaries= komodo_notaries("KMD",pubkeys,-1)) > 0 && numnotaries <= 64 )
                {
                    retjson = cJSON_CreateObject();
                    array = cJSON_CreateArray();
                    for (i=0; i<numnotaries; i++)
                    {
                        item = cJSON_CreateObject();
                        init_hexbytes_noT(str,pubkeys[i],33);
                        jaddstr(item,"pubkey",str);
                        //printf("[%s %d] ",str,i);
                        bitcoin_address(str,0,pubkeys[i],33);
                        jaddstr(item,"BTCaddress",str);
                        bitcoin_address(str,60,pubkeys[i],33);
                        jaddstr(item,"KMDaddress",str);
                        jaddi(array,item);
                    }
                    jadd(retjson,"notaries",array);
                    jaddnum(retjson,"numnotaries",numnotaries);
                    //printf("numnotaries.%d\n",numnotaries);
                }
                if ( retjson != 0 )
                {
                    dpow_randipbits(myinfo,coin,retjson);
                    retstr = jprint(retjson,1);
                }
                //printf("DEX NOTARIES -> (%s)\n",retstr);
            }
        } else printf("(%s) not active\n",dexreq.name);
        if ( retstr == 0 )
            return(clonestr("{\"error\":\"null return\"}"));
    }
    return(retstr);
}

char *dex_reqsend(struct supernet_info *myinfo,char *handler,uint8_t *data,int32_t datalen,int32_t M,char *field)
{
    char *retstrs[64],*origretstr0 = 0; cJSON *retjson; int32_t err,i,j,max = myinfo->numdexipbits;
    memset(retstrs,0,sizeof(retstrs));
    for (i=j=0; i<=max; i++)
    {
        if ( (retstrs[j]= _dex_reqsend(myinfo,handler,0,0,data,datalen)) != 0 )
        {
            //printf("j.%d of max.%d M.%d (%s)\n",j,max,M,retstrs[j]);
            if ( strncmp(retstrs[j],"{\"error\":\"null return\"}",strlen("{\"error\":\"null return\"}")) != 0 && strncmp(retstrs[j],"[]",strlen("[]")) != 0 && strcmp("0",retstrs[j]) != 0 )
            {
                if ( ++j == M )
                    break;
            }
            else if ( i < max )
                free(retstrs[j]);
        }
        //printf("automatic retry.%d of %d\n",i,max);
    }
    if ( j == 1 )
    {
        //printf("return.(%s)\n",retstrs[0]);
        return(retstrs[0]);
    }
    else if ( j >= M )
    {
        origretstr0 = retstrs[0];
        err = 0;
        if ( strcmp(field,"*") != 0 )
        {
            for (i=0; i<j; i++)
            {
                //printf("%s ",retstrs[i]);
                if ( (retjson= cJSON_Parse(retstrs[i])) != 0 )
                {
                    if ( i != 0 )
                        free(retstrs[i]);
                    retstrs[i] = jprint(jobj(retjson,field),0);
                    free_json(retjson);
                    //printf("(%s).%d\n",retstrs[i],i);
                } else err++;
            }
        }
        if ( err == 0 )
        {
            for (i=1; i<j; i++)
                if ( strcmp(retstrs[0],retstrs[i]) != 0 )
                {
                    printf("retstrs[%s] != [%s]\n",retstrs[i],retstrs[0]);
                    err = 1;
                    break;
                }
        }
        if ( err != 0 )
        {
            for (i=0; i<j; i++)
                free(retstrs[i]);
            retstrs[0] = clonestr("{\"error\":\"couldnt get consensus\"}");
        }
        else
        {
            if ( retstrs[0] != origretstr0 )
                free(retstrs[0]);
            retstrs[0] = origretstr0;
        }
    }
    else
    {
#define DEX_LESSTHAN_RETSTR "{\"error\":\"less than required responses\"}"
        for (i=0; i<j; i++)
            free(retstrs[i]);
        retstrs[0] = clonestr(DEX_LESSTHAN_RETSTR);
    }
    if ( retstrs[0] != 0 && retstrs[0][strlen(retstrs[0])-1] == '\n' )
        retstrs[0][strlen(retstrs[0])-1] = 0;
    return(retstrs[0]);
}

char *_dex_arrayreturn(char *retstr)
{
    if ( retstr != 0 )
    {
        if ( strcmp(retstr,DEX_LESSTHAN_RETSTR) == 0 )
        {
            free(retstr);
            retstr = clonestr("[]");
        }
    }
    return(retstr);
}

char *_dex_sendrequest(struct supernet_info *myinfo,struct dex_request *dexreq,int32_t M,char *field)
{
    uint8_t packet[sizeof(*dexreq)]; int32_t datalen;
    if ( iguana_isnotarychain(dexreq->name) >= 0 )
    {
        datalen = dex_rwrequest(1,packet,dexreq);
        return(dex_reqsend(myinfo,"request",packet,datalen,M,field));
    } else return(clonestr("{\"error\":\"not notarychain\"}"));
}

char *_dex_sendrequeststr(struct supernet_info *myinfo,struct dex_request *dexreq,char *str,int32_t slen,int32_t M,char *field)
{
    uint8_t *packet; int32_t datalen; char *retstr;
    if ( iguana_isnotarychain(dexreq->name) >= 0 )
    {
        if ( slen == 0 )
            slen = (int32_t)strlen(str)+1;
        packet = calloc(1,sizeof(*dexreq)+slen);
        datalen = dex_rwrequest(1,packet,dexreq);
        memcpy((char *)&packet[datalen],str,slen);
        datalen += slen;
        retstr = dex_reqsend(myinfo,"request",packet,datalen,M,field);
        free(packet);
        return(retstr);
    } else return(clonestr("{\"error\":\"not notarychain\"}"));
}

char *_dex_getrawtransaction(struct supernet_info *myinfo,char *symbol,bits256 txid)
{
    struct dex_request dexreq;
    memset(&dexreq,0,sizeof(dexreq));
    safecopy(dexreq.name,symbol,sizeof(dexreq.name));
    dexreq.hash = txid;
    dexreq.func = 'T';
    return(_dex_sendrequest(myinfo,&dexreq,1,""));
}

char *_dex_gettxout(struct supernet_info *myinfo,char *symbol,bits256 txid,int32_t vout)
{
    struct dex_request dexreq;
    //char str[65]; printf("gettxout(%s %s %d)\n",symbol,bits256_str(str,txid),vout);
    memset(&dexreq,0,sizeof(dexreq));
    safecopy(dexreq.name,symbol,sizeof(dexreq.name));
    dexreq.hash = txid;
    dexreq.shortarg = vout;
    dexreq.func = 'O';
    return(_dex_sendrequest(myinfo,&dexreq,3,"value"));
}

char *_dex_gettxin(struct supernet_info *myinfo,char *symbol,bits256 txid,int32_t vout)
{
    struct dex_request dexreq;
    //char str[65]; printf("gettxout(%s %s %d)\n",symbol,bits256_str(str,txid),vout);
    memset(&dexreq,0,sizeof(dexreq));
    safecopy(dexreq.name,symbol,sizeof(dexreq.name));
    dexreq.hash = txid;
    dexreq.shortarg = vout;
    dexreq.func = 'x';
    return(_dex_sendrequest(myinfo,&dexreq,1,""));
}

char *_dex_kvupdate(struct supernet_info *myinfo,char *symbol,char *key,char *value,int32_t flags)
{
    struct dex_request dexreq; char keyvalue[IGUANA_MAXSCRIPTSIZE]; int32_t keylen,valuesize;
    memset(&dexreq,0,sizeof(dexreq));
    safecopy(dexreq.name,symbol,sizeof(dexreq.name));
    dexreq.func = 'K';
    dexreq.intarg = flags;
    keylen = (int32_t)strlen(key);
    memcpy(keyvalue,key,keylen+1);
    valuesize = (int32_t)strlen(value);
    dexreq.shortarg = keylen+1;
    memcpy(&keyvalue[dexreq.shortarg],value,valuesize+1);
    //printf("_DEX.(%s) -> (%s) flags.%d\n",key,value,flags);
    return(_dex_sendrequeststr(myinfo,&dexreq,keyvalue,keylen+valuesize+2,1,""));
}

char *_dex_kvsearch(struct supernet_info *myinfo,char *symbol,char *key)
{
    struct dex_request dexreq;
    memset(&dexreq,0,sizeof(dexreq));
    safecopy(dexreq.name,symbol,sizeof(dexreq.name));
    dexreq.func = 'k';
    return(_dex_sendrequeststr(myinfo,&dexreq,key,0,1,""));
}

char *_dex_getinfo(struct supernet_info *myinfo,char *symbol)
{
    struct dex_request dexreq;
    memset(&dexreq,0,sizeof(dexreq));
    safecopy(dexreq.name,symbol,sizeof(dexreq.name));
    dexreq.func = 'I';
    return(_dex_sendrequest(myinfo,&dexreq,1,""));
}

int32_t _dex_getheight(struct supernet_info *myinfo,char *symbol)
{
    char *retstr; cJSON *retjson; int32_t height = -1;
    if ( (retstr= _dex_getinfo(myinfo,symbol)) != 0 )
    {
        if ( (retjson= cJSON_Parse(retstr)) != 0 )
        {
            height = jint(retjson,"blocks") - 1;
            free_json(retjson);
        }
        free(retstr);
    }
    return(height);
}

char *_dex_getnotaries(struct supernet_info *myinfo,char *symbol)
{
    struct dex_request dexreq; char *retstr,*pubkeystr; cJSON *retjson,*array,*item; int32_t i,n;
    memset(&dexreq,0,sizeof(dexreq));
    safecopy(dexreq.name,symbol,sizeof(dexreq.name));
    dexreq.func = 'N';
    dexreq.intarg = -1;
    if ( (retstr= _dex_sendrequest(myinfo,&dexreq,1,"")) != 0 )
    {
        if ( myinfo->numnotaries <= 0 && (retjson= cJSON_Parse(retstr)) != 0 )
        {
            if ( (myinfo->numnotaries= jint(retjson,"numnotaries")) != 0 && (array= jarray(&n,retjson,"notaries")) != 0 && n == myinfo->numnotaries )
            {
                for (i=0; i<n; i++)
                {
                    item = jitem(array,i);
                    if ( (pubkeystr= jstr(item,"pubkey")) != 0 && strlen(pubkeystr) == 33*2 )
                        decode_hex(myinfo->notaries[i],33,pubkeystr);
                }
            }
            else
            {
                extern const char *Notaries_elected[][2];
                myinfo->numnotaries = 64;//sizeof(Notaries_elected)/sizeof(*Notaries_elected);
                for (i=0; i<myinfo->numnotaries; i++)
                {
                    decode_hex(myinfo->notaries[i],33,(char *)Notaries_elected[i][1]);
                }
                printf("default to elected.%d\n",myinfo->numnotaries);
            }
            free_json(retjson);
        }
    }
    return(retstr);
}

char *_dex_alladdresses(struct supernet_info *myinfo,char *symbol)
{
    struct dex_request dexreq;
    memset(&dexreq,0,sizeof(dexreq));
    safecopy(dexreq.name,symbol,sizeof(dexreq.name));
    dexreq.func = '*';
    return(_dex_arrayreturn(_dex_sendrequest(myinfo,&dexreq,1,"")));
}

char *_dex_getblock(struct supernet_info *myinfo,char *symbol,bits256 hash2)
{
    struct dex_request dexreq;
    memset(&dexreq,0,sizeof(dexreq));
    safecopy(dexreq.name,symbol,sizeof(dexreq.name));
    dexreq.hash = hash2;
    dexreq.func = 'B';
    return(_dex_sendrequest(myinfo,&dexreq,1,""));
}

char *_dex_getblockhash(struct supernet_info *myinfo,char *symbol,int32_t height)
{
    struct dex_request dexreq;
    memset(&dexreq,0,sizeof(dexreq));
    safecopy(dexreq.name,symbol,sizeof(dexreq.name));
    dexreq.intarg = height;
    dexreq.func = 'H';
    return(_dex_sendrequest(myinfo,&dexreq,3,"*"));
}

char *_dex_getbestblockhash(struct supernet_info *myinfo,char *symbol)
{
    struct dex_request dexreq;
    memset(&dexreq,0,sizeof(dexreq));
    safecopy(dexreq.name,symbol,sizeof(dexreq.name));
    dexreq.func = 'P';
    return(_dex_sendrequest(myinfo,&dexreq,1,""));
}

char *_dex_sendrawtransaction(struct supernet_info *myinfo,char *symbol,char *signedtx)
{
    struct dex_request dexreq;
    memset(&dexreq,0,sizeof(dexreq));
    safecopy(dexreq.name,symbol,sizeof(dexreq.name));
    dexreq.func = 'S';
    return(_dex_sendrequeststr(myinfo,&dexreq,signedtx,0,3,"*"));
}

char *_dex_importaddress(struct supernet_info *myinfo,char *symbol,char *address)
{
    struct dex_request dexreq;
return(clonestr("{\"error\":\"importaddress deprecated\"}"));
    memset(&dexreq,0,sizeof(dexreq));
    safecopy(dexreq.name,symbol,sizeof(dexreq.name));
    dexreq.func = 'A';
    return(_dex_sendrequeststr(myinfo,&dexreq,address,0,1,""));
}

char *_dex_checkaddress(struct supernet_info *myinfo,char *symbol,char *address)
{
    struct dex_request dexreq;
    memset(&dexreq,0,sizeof(dexreq));
    safecopy(dexreq.name,symbol,sizeof(dexreq.name));
    dexreq.func = 'C';
    return(_dex_sendrequeststr(myinfo,&dexreq,address,0,3,"address"));
}

char *_dex_validateaddress(struct supernet_info *myinfo,char *symbol,char *address)
{
    struct dex_request dexreq;
    memset(&dexreq,0,sizeof(dexreq));
    safecopy(dexreq.name,symbol,sizeof(dexreq.name));
    dexreq.func = 'V';
    return(_dex_sendrequeststr(myinfo,&dexreq,address,0,1,""));
}

char *_dex_listunspentarg(struct supernet_info *myinfo,char *symbol,char *address,uint8_t arg)
{
    struct dex_request dexreq; char *retstr;
    memset(&dexreq,0,sizeof(dexreq));
    safecopy(dexreq.name,symbol,sizeof(dexreq.name));
    dexreq.func = arg;
    if ( (retstr= _dex_sendrequeststr(myinfo,&dexreq,address,0,1,"")) != 0 )
    {
        //printf("UNSPENTS.(%s)\n",retstr);
    }
    return(_dex_arrayreturn(retstr));
}

char *_dex_listunspent(struct supernet_info *myinfo,char *symbol,char *address)
{
    return(_dex_listunspentarg(myinfo,symbol,address,'u')); // 'U' old variant
}

char *_dex_listunspent2(struct supernet_info *myinfo,char *symbol,char *address)
{
    return(_dex_listunspentarg(myinfo,symbol,address,'u'));
}

char *_dex_listspent(struct supernet_info *myinfo,char *symbol,char *address)
{
    struct dex_request dexreq; char *retstr;
    memset(&dexreq,0,sizeof(dexreq));
    safecopy(dexreq.name,symbol,sizeof(dexreq.name));
    dexreq.func = 's';
    if ( (retstr= _dex_sendrequeststr(myinfo,&dexreq,address,0,1,"")) != 0 )
    {
        //printf("UNSPENTS.(%s)\n",retstr);
    }
    return(_dex_arrayreturn(retstr));
}

char *_dex_getbalance(struct supernet_info *myinfo,char *symbol,char *address)
{
    struct dex_request dexreq; char *retstr;
    memset(&dexreq,0,sizeof(dexreq));
    safecopy(dexreq.name,symbol,sizeof(dexreq.name));
    dexreq.func = 'b';
    if ( (retstr= _dex_sendrequeststr(myinfo,&dexreq,address,0,1,"")) != 0 )
    {
        //printf("UNSPENTS.(%s)\n",retstr);
    }
    return(_dex_arrayreturn(retstr));
}

char *_dex_listtransactions(struct supernet_info *myinfo,char *symbol,char *address,int32_t count,int32_t skip)
{
    struct dex_request dexreq;
    memset(&dexreq,0,sizeof(dexreq));
    safecopy(dexreq.name,symbol,sizeof(dexreq.name));
    dexreq.intarg = skip;
    dexreq.shortarg = count;
    dexreq.func = '2';//'L';
    return(_dex_arrayreturn(_dex_sendrequeststr(myinfo,&dexreq,address,0,1,"")));
}

char *_dex_listtransactions2(struct supernet_info *myinfo,char *symbol,char *address,int32_t count,int32_t skip)
{
    struct dex_request dexreq;
    memset(&dexreq,0,sizeof(dexreq));
    safecopy(dexreq.name,symbol,sizeof(dexreq.name));
    dexreq.intarg = skip;
    dexreq.shortarg = count;
    dexreq.func = '2';
    return(_dex_arrayreturn(_dex_sendrequeststr(myinfo,&dexreq,address,0,1,"")));
}

int32_t dex_crc32find(struct supernet_info *myinfo,uint32_t crc32)
{
    int32_t i,firstz = -1;
    for (i=0; i<sizeof(myinfo->dexcrcs)/sizeof(*myinfo->dexcrcs); i++)
    {
        if ( myinfo->dexcrcs[i] == crc32 )
        {
            //printf("NANODUPLICATE.%08x\n",crc32);
            return(-1);
        }
        else if ( firstz < 0 && myinfo->dexcrcs[i] == 0 )
            firstz = i;
    }
    if ( firstz < 0 )
        firstz = (rand() % (sizeof(myinfo->dexcrcs)/sizeof(*myinfo->dexcrcs)));
    myinfo->dexcrcs[firstz] = crc32;
    return(firstz);
}

int32_t dex_packetcheck(struct supernet_info *myinfo,struct dex_nanomsghdr *dexp,int32_t size)
{
    int32_t firstz=-1; uint32_t crc32;
    if ( dexp->version0 == (DEX_VERSION & 0xff) && dexp->version1 == ((DEX_VERSION >> 8) & 0xff) )
    {
        if ( dexp->datalen == (size - sizeof(*dexp)) )
        {
            crc32 = calc_crc32(0,dexp->packet,dexp->datalen);//(void *)((long)dexp + sizeof(dexp->crc32)),(int32_t)(size - sizeof(dexp->crc32)));
            if ( dexp->crc32 == crc32 )//&& (firstz= dex_crc32find(myinfo,crc32)) >= 0 )
                return(0);
            else printf("dexp %08x != %08x || firstz.%d < 0\n",dexp->crc32,crc32,firstz);
        } else printf("datalen.%d != (%d - %ld)\n",dexp->datalen,size,sizeof(*dexp));
    }
    return(-1);
}

int32_t dex_subsock_poll(struct supernet_info *myinfo)
{
    int32_t size= -1; struct dex_nanomsghdr *dexp; void *freeptr;
    //fprintf(stderr,"subsock.%d\n",myinfo->subsock);
    if ( myinfo->subsock >= 0 && (size= signed_nn_recv(&freeptr,myinfo->ctx,myinfo->notaries,myinfo->numnotaries,myinfo->subsock,&dexp)) >= 0 )
    {
        if ( dexp != 0 )
        {
            //printf("SUBSOCK.%08x recv.%d datalen.%d\n",dexp->crc32,size,dexp->datalen);
            if ( dex_packetcheck(myinfo,dexp,size) == 0 )
                dex_packet(myinfo,dexp,size);
        } //else printf("size.%d\n",size);
        if ( freeptr != 0 )
            nn_freemsg(freeptr), dexp = 0, freeptr = 0;
    }
    return(size);
}

void dex_updateclient(struct supernet_info *myinfo)
{
    int32_t i;
    if ( myinfo->IAMNOTARY == 0 )
    {
        for (i=0; i<1000; i++)
            if ( dex_subsock_poll(myinfo) <= 0 )
                break;
    }
}

#if ISNOTARYNODE
struct dpow_nanoutxo
{
    bits256 srcutxo,destutxo;
    uint64_t bestmask,recvmask;
    uint32_t pendingcrcs[2],paxwdcrc;
    uint16_t srcvout,destvout;
    uint8_t sigs[2][DPOW_MAXSIGLEN],siglens[2],pad,bestk;
} PACKED;

struct dpow_nanomsghdr
{
    bits256 srchash,desthash;
    struct dpow_nanoutxo ratify,notarize;
    uint32_t channel,height,size,datalen,crc32,myipbits,numipbits,ipbits[64];
    char symbol[16];
    uint8_t senderind,version0,version1,packet[];
} PACKED;


uint64_t dpow_ratifybest(uint64_t refmask,struct dpow_block *bp,int8_t *lastkp);
struct dpow_block *dpow_heightfind(struct supernet_info *myinfo,struct dpow_info *dp,int32_t height);
int32_t dpow_signedtxgen(struct supernet_info *myinfo,struct dpow_info *dp,struct iguana_info *coin,struct dpow_block *bp,int8_t bestk,uint64_t bestmask,int32_t myind,uint32_t deprec,int32_t src_or_dest,int32_t useratified);
void dpow_sigscheck(struct supernet_info *myinfo,struct dpow_info *dp,struct dpow_block *bp,int32_t myind,int32_t src_or_dest,int8_t bestk,uint64_t bestmask,uint8_t pubkeys[64][33],int32_t numratified);

int32_t dpow_addnotary(struct supernet_info *myinfo,struct dpow_info *dp,char *ipaddr)
{
    char str[512]; uint32_t ipbits,*ptr; int32_t i,iter,n,retval = -1;
    if ( myinfo->IAMNOTARY == 0 )
        return(-1);
    portable_mutex_lock(&myinfo->notarymutex);
    if ( myinfo->dpowsock >= 0 && myinfo->dexsock >= 0 )
    {
        ipbits = (uint32_t)calc_ipbits(ipaddr);
        for (iter=0; iter<2; iter++)
        {
            if ( iter == 0 )
            {
                n = myinfo->numdpowipbits;
                ptr = myinfo->dpowipbits;
            }
            else
            {
                n = dp->numipbits;
                ptr = dp->ipbits;
            }
            for (i=0; i<n; i++)
                if ( ipbits == ptr[i] )
                    break;
            if ( i == n && n < 64 )
            {
                ptr[n] = ipbits;
                if ( iter == 0 && strcmp(ipaddr,myinfo->ipaddr) != 0 )
                {
                    retval = nn_connect(myinfo->dpowsock,nanomsg_tcpname(0,str,ipaddr,DPOW_SOCK));
                    printf("NN_CONNECT to (%s)\n",str);
                    retval = nn_connect(myinfo->dexsock,nanomsg_tcpname(0,str,ipaddr,DEX_SOCK));
                }
                n++;
                qsort(ptr,n,sizeof(uint32_t),_increasing_ipbits);
                if ( iter == 0 )
                    myinfo->numdpowipbits = n;
                else dp->numipbits = n;
                //for (i=0; i<n; i++)
                //    printf("%08x ",ptr[i]);
                //printf("addnotary.[%d] (%s) retval.%d (total %d %d) iter.%d\n",n,ipaddr,retval,myinfo->numdpowipbits,dp!=0?dp->numipbits:-1,iter);
            }
            if ( dp == 0 )
                break;
        }
    }
    portable_mutex_unlock(&myinfo->notarymutex);
    return(retval);
}

void dpow_nanomsginit(struct supernet_info *myinfo,char *ipaddr)
{
    char str[512]; int32_t timeout,retval,maxsize,dpowsock,dexsock,repsock,pubsock;
    if ( myinfo->ipaddr[0] == 0 )
    {
        printf("need to set ipaddr before nanomsg\n");
        return;
    }
    if ( myinfo->IAMNOTARY == 0 )
        return;
    portable_mutex_lock(&myinfo->notarymutex);
    dpowsock = myinfo->dpowsock;
    dexsock = myinfo->dexsock;
    repsock = myinfo->repsock;
    pubsock = myinfo->pubsock;
    if ( dpowsock < 0 && (dpowsock= nn_socket(AF_SP,NN_BUS)) >= 0 )
    {
        if ( nn_bind(dpowsock,nanomsg_tcpname(myinfo,str,myinfo->ipaddr,DPOW_SOCK)) < 0 )
        {
            printf("error binding to dpowsock (%s)\n",nanomsg_tcpname(myinfo,str,myinfo->ipaddr,DPOW_SOCK));
            nn_close(dpowsock);
            dpowsock = -1;
        }
        else
        {
            printf("NN_BIND to %s\n",str);
            if ( dexsock < 0 && (dexsock= nn_socket(AF_SP,NN_BUS)) >= 0 )
            {
                if ( nn_bind(dexsock,nanomsg_tcpname(myinfo,str,myinfo->ipaddr,DEX_SOCK)) < 0 )
                {
                    printf("error binding to dexsock (%s)\n",nanomsg_tcpname(myinfo,str,myinfo->ipaddr,DEX_SOCK));
                    nn_close(dexsock);
                    dexsock = -1;
                    nn_close(dpowsock);
                    dpowsock = -1;
                }
                else
                {
                    if ( pubsock < 0 && (pubsock= nn_socket(AF_SP,NN_PUB)) >= 0 )
                    {
                        if ( nn_bind(pubsock,nanomsg_tcpname(myinfo,str,myinfo->ipaddr,PUB_SOCK)) < 0 )
                        {
                            printf("error binding to pubsock (%s)\n",nanomsg_tcpname(myinfo,str,myinfo->ipaddr,PUB_SOCK));
                            nn_close(pubsock);
                            pubsock = -1;
                            nn_close(dexsock);
                            dexsock = -1;
                            nn_close(dpowsock);
                            dpowsock = -1;
                        }
                        else
                        {
                            if ( repsock < 0 && (repsock= nn_socket(AF_SP,NN_REP)) >= 0 )
                            {
                                if ( nn_bind(repsock,nanomsg_tcpname(myinfo,str,myinfo->ipaddr,REP_SOCK)) < 0 )
                                {
                                    printf("error binding to repsock (%s)\n",nanomsg_tcpname(myinfo,str,myinfo->ipaddr,REP_SOCK));
                                    nn_close(repsock);
                                    repsock = -1;
                                    nn_close(pubsock);
                                    pubsock = -1;
                                    nn_close(dexsock);
                                    dexsock = -1;
                                    nn_close(dpowsock);
                                    dpowsock = -1;
                                }
                                else
                                {
                                    timeout = 100;
                                    nn_setsockopt(repsock,NN_SOL_SOCKET,NN_SNDTIMEO,&timeout,sizeof(timeout));
                                    nn_setsockopt(dexsock,NN_SOL_SOCKET,NN_SNDTIMEO,&timeout,sizeof(timeout));
                                    nn_setsockopt(pubsock,NN_SOL_SOCKET,NN_SNDTIMEO,&timeout,sizeof(timeout));
                                    timeout = 10;
                                    nn_setsockopt(dexsock,NN_SOL_SOCKET,NN_RCVTIMEO,&timeout,sizeof(timeout));
                                    timeout = 10;
                                    nn_setsockopt(repsock,NN_SOL_SOCKET,NN_RCVTIMEO,&timeout,sizeof(timeout));
                                    maxsize = 1024 * 1024;
                                    printf("RCVBUF.%d\n",nn_setsockopt(dexsock,NN_SOL_SOCKET,NN_RCVBUF,&maxsize,sizeof(maxsize)));
                                    printf("RCVBUF.%d\n",nn_setsockopt(repsock,NN_SOL_SOCKET,NN_RCVBUF,&maxsize,sizeof(maxsize)));
                                    printf("DEXINIT dpow.%d dex.%d rep.%d\n",dpowsock,dexsock,repsock);
                                }
                            }
                        }
                    }
                }
            }
            myinfo->dpowipbits[0] = (uint32_t)calc_ipbits(myinfo->ipaddr);
            myinfo->numdpowipbits = 1;
            timeout = 10;
            nn_setsockopt(dpowsock,NN_SOL_SOCKET,NN_RCVTIMEO,&timeout,sizeof(timeout));
            maxsize = 1024 * 1024;
            printf("RCVBUF.%d\n",nn_setsockopt(dpowsock,NN_SOL_SOCKET,NN_RCVBUF,&maxsize,sizeof(maxsize)));
            
            myinfo->nanoinit = (uint32_t)time(NULL);
        }
    } //else printf("error creating nanosocket\n");
    if ( myinfo->dpowsock != dpowsock )
        myinfo->dpowsock = dpowsock;
    if ( myinfo->dexsock != dexsock )
        myinfo->dexsock = dexsock;
    if ( myinfo->repsock != repsock )
        myinfo->repsock = repsock;
    if ( myinfo->pubsock != pubsock )
        myinfo->pubsock = pubsock;
    portable_mutex_unlock(&myinfo->notarymutex);
    dpow_addnotary(myinfo,0,ipaddr);
}

void dpow_bestconsensus(struct dpow_block *bp)
{
    int8_t bestks[64]; int32_t counts[64],i,j,numcrcs=0,numdiff,besti,best,bestmatches = 0,matches = 0; uint64_t masks[64],matchesmask,recvmask; uint32_t crcval=0; char srcaddr[64],destaddr[64];
    memset(masks,0,sizeof(masks));
    memset(bestks,0xff,sizeof(bestks));
    memset(counts,0,sizeof(counts));
    recvmask = 0;
    for (numdiff=i=0; i<bp->numnotaries; i++)
    {
        if ( bits256_nonz(bp->notaries[i].src.prev_hash) != 0 && bits256_nonz(bp->notaries[i].dest.prev_hash) != 0 )
            recvmask |= (1LL << i);
        if ( bp->notaries[i].bestk < 0 || bp->notaries[i].bestmask == 0 )
            continue;
        //if ( bp->require0 != 0 && (bp->notaries[i].bestmask & 1) == 0 )
        //    continue;
        for (j=0; j<numdiff; j++)
            if ( bp->notaries[i].bestk == bestks[j] && bp->notaries[i].bestmask == masks[j] )
            {
                counts[j]++;
                break;
            }
        if ( j == numdiff && bp->notaries[i].bestk >= 0 && bp->notaries[i].bestmask != 0 )
        {
            masks[numdiff] = bp->notaries[i].bestmask;
            bestks[numdiff] = bp->notaries[i].bestk;
            counts[numdiff]++;
            //printf("j.%d numdiff.%d (%d %llx).%d\n",j,numdiff,bp->notaries[i].bestk,(long long)bp->notaries[i].bestmask,counts[numdiff]);
            numdiff++;
        }
    }
    besti = -1, best = 0;
    for (i=0; i<numdiff; i++)
    {
        //printf("(%d %llx).%d ",bestks[i],(long long)masks[i],counts[i]);
        if ( counts[i] > best && bitweight(masks[i]) >= bp->minsigs )
        {
            best = counts[i];
            besti = i;
        }
    }
    if ( besti >= 0 && bestks[besti] >= 0 && masks[besti] != 0 && (recvmask & masks[besti]) == masks[besti] )
    {
        bp->notaries[bp->myind].bestmask = bp->bestmask = masks[besti];
        bp->notaries[bp->myind].bestk = bp->bestk = bestks[besti];
        //printf("set best.%d to (%d %llx) recv.%llx\n",best,bp->bestk,(long long)bp->bestmask,(long long)recvmask);
    }
    bp->recvmask |= recvmask;
    if ( bp->bestmask == 0 )//|| (time(NULL) / 180) != bp->lastepoch )
    {
        bp->bestmask = dpow_notarybestk(bp->recvmask,bp,&bp->bestk);
        if ( 0 && (time(NULL) / 180) != bp->lastepoch )
        {
            bp->lastepoch = (uint32_t)(time(NULL) / 180);
            printf("epoch %u\n",bp->lastepoch % bp->numnotaries);
            sleep(1 + (rand() % 3));
        }
    }
}

void dpow_nanoutxoset(struct supernet_info *myinfo,struct dpow_info *dp,struct dpow_nanoutxo *np,struct dpow_block *bp,int32_t isratify)
{
    int32_t i,err,vout; cJSON *ujson; char coinaddr[64],str[65];
    if ( bp->myind < 0 )
        return;
    if ( isratify != 0 )
    {
        np->srcutxo = bp->notaries[bp->myind].ratifysrcutxo;
        np->srcvout = bp->notaries[bp->myind].ratifysrcvout;
        np->destutxo = bp->notaries[bp->myind].ratifydestutxo;
        np->destvout = bp->notaries[bp->myind].ratifydestvout;
        if ( bp->myind != 0 )
        {
            err = 0;
            if ( (ujson= dpow_gettxout(myinfo,bp->srccoin,np->srcutxo,np->srcvout)) != 0 )
            {
                if ( (uint64_t)(jdouble(ujson,"value") * SATOSHIDEN) == 0 )
                {
                    //printf("(%s)\n",jprint(ujson,0));
                    err = 1;
                }
                free_json(ujson);
            } else err = 1;
            if ( err != 0 )
            {
                bitcoin_address(coinaddr,bp->srccoin->chain->pubtype,dp->minerkey33,33);
                if ( dpow_haveutxo(myinfo,bp->srccoin,&bp->notaries[bp->myind].ratifysrcutxo,&vout,coinaddr) > 0 )
                {
                    bp->notaries[bp->myind].ratifysrcvout = vout;
                    np->srcutxo = bp->notaries[bp->myind].ratifysrcutxo;
                    np->srcvout = bp->notaries[bp->myind].ratifysrcvout;
                    printf("Replace UTXO.%s < %s/v%d\n",bp->srccoin->symbol,bits256_str(str,np->srcutxo),vout);
                } else printf("cant find utxo.%s\n",bp->srccoin->symbol);
            }
            err = 0;
            if ( (ujson= dpow_gettxout(myinfo,bp->destcoin,np->destutxo,np->destvout)) != 0 )
            {
                if ( (uint64_t)(jdouble(ujson,"value") * SATOSHIDEN) == 0 )
                    err = 1;
                free_json(ujson);
            } else err = 1;
            if ( err != 0 )
            {
                bitcoin_address(coinaddr,bp->destcoin->chain->pubtype,dp->minerkey33,33);
                if ( dpow_haveutxo(myinfo,bp->destcoin,&bp->notaries[bp->myind].ratifydestutxo,&vout,coinaddr) > 0 )
                {
                    bp->notaries[bp->myind].ratifydestvout = vout;
                    np->destutxo = bp->notaries[bp->myind].ratifydestutxo;
                    np->destvout = bp->notaries[bp->myind].ratifydestvout;
                    printf("Replace UTXO.%s < %s/v%d\n",bp->destcoin->symbol,bits256_str(str,np->destutxo),vout);
                } else printf("cant find utxo.%s\n",bp->destcoin->symbol);
            }
        }
        np->bestmask = bp->ratifybestmask;
        np->recvmask = bp->ratifyrecvmask;
        //printf("send ratify best.(%d %llx) siglens.(%d %d)\n", bp->ratifybestk,(long long)bp->ratifybestmask,bp->ratifysiglens[0],bp->ratifysiglens[1]);
        if ( (np->bestk= bp->ratifybestk) >= 0 )
        {
            for (i=0; i<2; i++)
            {
                if ( (np->siglens[i]= bp->ratifysiglens[i]) > 0 )
                    memcpy(np->sigs[i],bp->ratifysigs[i],np->siglens[i]);
            }
        }
    }
    else
    {
        dpow_bestconsensus(bp);
        np->srcutxo = bp->notaries[bp->myind].src.prev_hash;
        np->srcvout = bp->notaries[bp->myind].src.prev_vout;
        np->destutxo = bp->notaries[bp->myind].dest.prev_hash;
        np->destvout = bp->notaries[bp->myind].dest.prev_vout;
        if ( (np->recvmask= bp->recvmask) == 0 )
            np->recvmask = bp->notaries[bp->myind].recvmask;
        if ( (np->bestmask= bp->pendingbestmask) == 0 )
        {
            if ( (np->bestmask= bp->notaries[bp->myind].bestmask) == 0 )
                np->bestmask = bp->bestmask, np->bestk = bp->bestk;
            else np->bestk = bp->notaries[bp->myind].bestk;
        } else np->bestk = bp->pendingbestk;
        if ( (int8_t)np->bestk >= 0 )
        {
            if ( (np->siglens[0]= bp->notaries[bp->myind].src.siglens[bp->bestk]) > 0 )
                memcpy(np->sigs[0],bp->notaries[bp->myind].src.sigs[bp->bestk],np->siglens[0]);
            if ( (np->siglens[1]= bp->notaries[bp->myind].dest.siglens[bp->bestk]) > 0 )
                memcpy(np->sigs[1],bp->notaries[bp->myind].dest.sigs[bp->bestk],np->siglens[1]);
        }
    }
}

void dpow_ratify_update(struct supernet_info *myinfo,struct dpow_info *dp,struct dpow_block *bp,uint8_t senderind,int8_t bestk,uint64_t bestmask,uint64_t recvmask,bits256 srcutxo,uint16_t srcvout,bits256 destutxo,uint16_t destvout,uint8_t siglens[2],uint8_t sigs[2][DPOW_MAXSIGLEN],uint32_t pendingcrcs[2])
{
    int8_t bestks[64]; int32_t counts[64],i,j,numcrcs=0,numdiff,besti,best,bestmatches = 0,matches = 0; uint64_t masks[64],matchesmask; uint32_t crcval=0; char srcaddr[64],destaddr[64];
    //char str[65],str2[65];
    //printf("senderind.%d num.%d %s %s\n",senderind,bp->numnotaries,bits256_str(str,srcutxo),bits256_str(str2,destutxo));
    if ( bp->myind < 0 )
        return;
    if ( bp->isratify != 0 && senderind >= 0 && senderind < bp->numnotaries && bits256_nonz(srcutxo) != 0 && bits256_nonz(destutxo) != 0 )
    {
        memset(masks,0,sizeof(masks));
        memset(bestks,0xff,sizeof(bestks));
        memset(counts,0,sizeof(counts));
        for (i=0; i<2; i++)
            bp->notaries[senderind].pendingcrcs[i] = pendingcrcs[i];
        bp->notaries[senderind].ratifysrcutxo = srcutxo;
        bp->notaries[senderind].ratifysrcvout = srcvout;
        bp->notaries[senderind].ratifydestutxo = destutxo;
        bp->notaries[senderind].ratifydestvout = destvout;
        bp->notaries[senderind].ratifybestmask = bestmask;
        bp->notaries[senderind].ratifyrecvmask = recvmask;
        if ( (bp->notaries[senderind].ratifybestk= bestk) >= 0 )
        {
            for (i=0; i<2; i++)
            {
                if ( (bp->notaries[senderind].ratifysiglens[i]= siglens[i]) != 0 )
                {
                    memcpy(bp->notaries[senderind].ratifysigs[i],sigs[i],siglens[i]);
                    if ( bestk == bp->pendingratifybestk && bestmask == bp->pendingratifybestmask )
                    {
                        if ( ((1LL << senderind) & bestmask) != 0 )
                            bp->ratifysigmasks[i] |= (1LL << senderind);
                    } else bp->ratifysigmasks[i] &= ~(1LL << senderind);
                }
            }
        }
        //printf("RECV from %d best.(%d %llx) sigs.(%d %d) %llx %llx\n",senderind,bestk,(long long)bestmask,siglens[0],siglens[1],(long long)bp->ratifysigmasks[0],(long long)bp->ratifysigmasks[1]);
        bp->ratifyrecvmask = 0;
        bp->ratifybestmask = 0;
        bp->ratifybestk = -1;
        for (numdiff=i=0; i<bp->numnotaries; i++)
        {
            if ( bits256_nonz(bp->notaries[i].ratifysrcutxo) != 0 && bits256_nonz(bp->notaries[i].ratifydestutxo) != 0 )
                bp->ratifyrecvmask |= (1LL << i);
            if ( bp->notaries[i].ratifybestk < 0 || bp->notaries[i].ratifybestmask == 0 )
                continue;
            if ( bp->require0 != 0 && (bp->notaries[i].ratifybestmask & 1) == 0 )
                continue;
            for (j=0; j<numdiff; j++)
                if ( bp->notaries[i].ratifybestk == bestks[j] && bp->notaries[i].ratifybestmask == masks[j] )
                {
                    counts[j]++;
                    break;
                }
            if ( j == numdiff && bp->notaries[i].ratifybestk >= 0 && bp->notaries[i].ratifybestmask != 0 )
            {
                masks[numdiff] = bp->notaries[i].ratifybestmask;
                bestks[numdiff] = bp->notaries[i].ratifybestk;
                counts[numdiff]++;
                //printf("j.%d numdiff.%d (%d %llx).%d\n",j,numdiff,bp->notaries[i].ratifybestk,(long long)bp->notaries[i].ratifybestmask,counts[numdiff]);
                numdiff++;
            }
        }
        besti = -1, best = 0;
        for (i=0; i<numdiff; i++)
        {
            //printf("(%d %llx).%d ",bestks[i],(long long)masks[i],counts[i]);
            if ( counts[i] > best )
            {
                best = counts[i];
                besti = i;
            }
        }
        if ( besti >= 0 && bestks[besti] >= 0 && masks[besti] != 0 && (bp->ratifyrecvmask & masks[besti]) == masks[besti] )
            bp->ratifybestmask = masks[besti], bp->ratifybestk = bestks[besti];
        //printf("numdiff.%d besti.%d numbest.%d (%d %llx) vs (%d %llx)\n",numdiff,besti,best,besti>=0?bestks[besti]:-1,(long long)(besti>=0?masks[besti]:0),bestk,(long long)bestmask);
        if ( bp->ratifybestmask == 0 || (time(NULL) / DPOW_EPOCHDURATION) != bp->lastepoch )
        {
            bp->ratifybestmask = dpow_ratifybest(bp->ratifyrecvmask,bp,&bp->ratifybestk);
            if ( (time(NULL) / DPOW_EPOCHDURATION) != bp->lastepoch )
            {
                bp->lastepoch = (uint32_t)(time(NULL) / DPOW_EPOCHDURATION);
                printf("epoch %u\n",bp->lastepoch % bp->numnotaries);
                sleep(2 + (rand() % 7));
            }
        }
        bp->notaries[bp->myind].ratifybestk = bp->ratifybestk;
        bp->notaries[bp->myind].ratifybestmask = bp->ratifybestmask;
        bp->notaries[bp->myind].ratifyrecvmask = bp->ratifyrecvmask;
        if ( bp->ratifybestk >= 0 )
        {
            for (matchesmask=i=0; i<bp->numnotaries; i++)
            {
                if ( bp->ratifybestk >= 0 && bp->notaries[i].ratifybestk == bp->ratifybestk && bp->notaries[i].ratifybestmask == bp->ratifybestmask )
                {
                    matches++;
                    if ( ((1LL << i) & bp->ratifybestmask) != 0 )
                    {
                        matchesmask |= (1LL << i);
                        bestmatches++;
                    }
                }
            }
            crcval = 0;
            numcrcs = 0;
            for (i=0; i<bp->numnotaries; i++)
            {
                if ( ((1LL << i) & matchesmask) != 0 )
                {
                    if ( bp->notaries[i].pendingcrcs[bp->state < 1000] == 0 )
                        continue;
                    if ( numcrcs == 0 )
                        numcrcs++, crcval = bp->notaries[i].pendingcrcs[bp->state < 1000];
                    else if ( numcrcs > 0 && crcval == bp->notaries[i].pendingcrcs[bp->state < 1000] )
                        numcrcs++;
                }
            }
            //printf("crcval.%x numcrcs.%d bestmatches.%d matchesmask.%llx\n",crcval,numcrcs,bestmatches,(long long)matchesmask);
            if ( bestmatches >= bp->minsigs )//&& numcrcs >= bp->minsigs )
            {
                if ( bp->pendingratifybestk != bp->ratifybestk || bp->pendingratifybestmask != bp->ratifybestmask )
                {
                    printf("new PENDING RATIFY BESTK (%d %llx) crcval.%08x num.%d\n",bp->ratifybestk,(long long)bp->ratifybestmask,crcval,numcrcs);
                    bp->pendingratifybestk = bp->ratifybestk;
                    bp->pendingratifybestmask = bp->ratifybestmask;
                    memset(bp->notaries[bp->myind].ratifysigs,0,sizeof(bp->notaries[bp->myind].ratifysigs));
                    memset(bp->notaries[bp->myind].ratifysiglens,0,sizeof(bp->notaries[bp->myind].ratifysiglens));
                    memset(bp->ratifysigmasks,0,sizeof(bp->ratifysigmasks));
                    dpow_signedtxgen(myinfo,dp,bp->destcoin,bp,bp->ratifybestk,bp->ratifybestmask,bp->myind,DPOW_SIGBTCCHANNEL,1,1);
                    for (i=0; i<bp->numnotaries; i++)
                    {
                        if ( i != bp->myind )
                        {
                            memset(&bp->notaries[i].ratifysrcutxo,0,sizeof(bp->notaries[i].ratifysrcutxo));
                            memset(&bp->notaries[i].ratifydestutxo,0,sizeof(bp->notaries[i].ratifydestutxo));
                            bp->notaries[i].ratifybestmask = bp->notaries[i].ratifyrecvmask = 0;
                        }
                        else if ( bp->require0 == 0 )
                        {
                            bitcoin_address(srcaddr,bp->srccoin->chain->pubtype,dp->minerkey33,33);
                            bitcoin_address(destaddr,bp->destcoin->chain->pubtype,dp->minerkey33,33);
                            if ( dpow_checkutxo(myinfo,dp,bp,bp->destcoin,&bp->notaries[i].dest.prev_hash,&bp->notaries[i].dest.prev_vout,destaddr) < 0 )
                            {
                                printf("dont have %s %s utxo, please send funds\n",dp->dest,destaddr);
                            }
                            if ( dpow_checkutxo(myinfo,dp,bp,bp->srccoin,&bp->notaries[i].src.prev_hash,&bp->notaries[i].src.prev_vout,srcaddr) < 0 )
                            {
                                printf("dont have %s %s utxo, please send funds\n",dp->symbol,srcaddr);
                            }
                        }
                    }
                }
                if ( bp->ratifysigmasks[1] == bp->pendingratifybestmask ) // have all sigs
                {
                    if ( bp->state < 1000 )
                    {
                        dpow_sigscheck(myinfo,dp,bp,bp->myind,1,bp->pendingratifybestk,bp->pendingratifybestmask,bp->ratified_pubkeys,bp->numratified);
                    }
                    if ( bp->ratifysigmasks[0] == bp->pendingratifybestmask ) // have all sigs
                    {
                        if ( bp->state != 0xffffffff )
                            dpow_sigscheck(myinfo,dp,bp,bp->myind,0,bp->pendingratifybestk,bp->pendingratifybestmask,bp->ratified_pubkeys,bp->numratified);
                    }
                    else if ( ((1LL << bp->myind) & bp->ratifybestmask) != 0 && (rand() % 100) == 0 )
                    {
                        dpow_signedtxgen(myinfo,dp,bp->srccoin,bp,bp->ratifybestk,bp->ratifybestmask,bp->myind,DPOW_SIGCHANNEL,0,1);
                    }
                    //else printf("ratify srcmask.%llx != bestmask.%llx\n",(long long)bp->ratifysigmasks[0],(long long)bp->bestmask);
                }
                else if ( ((1LL << bp->myind) & bp->ratifybestmask) != 0 && (rand() % 100) == 0 )
                {
                    dpow_signedtxgen(myinfo,dp,bp->destcoin,bp,bp->ratifybestk,bp->ratifybestmask,bp->myind,DPOW_SIGBTCCHANNEL,1,1);
                }
                //else printf("ratify destmask.%llx != bestmask.%llx\n",(long long)bp->ratifysigmasks[1],(long long)bp->bestmask);
            }
        }
        if ( (rand() % 100) == 0 )
            printf("[%d] numips.%d %s RATIFY.%d matches.%d bestmatches.%d bestk.%d %llx recv.%llx %llx sigmasks.(%llx %llx) crcval.%x num.%d\n",bp->myind,dp->numipbits,dp->symbol,bp->minsigs,matches,bestmatches,bp->ratifybestk,(long long)bp->ratifybestmask,(long long)bp->ratifyrecvmask,(long long)matchesmask,(long long)bp->ratifysigmasks[1],(long long)bp->ratifysigmasks[0],crcval,numcrcs);
    }
}

void dpow_notarize_update(struct supernet_info *myinfo,struct dpow_info *dp,struct dpow_block *bp,uint8_t senderind,int8_t bestk,uint64_t bestmask,uint64_t recvmask,bits256 srcutxo,uint16_t srcvout,bits256 destutxo,uint16_t destvout,uint8_t siglens[2],uint8_t sigs[2][DPOW_MAXSIGLEN],uint32_t paxwdcrc)
{
    bits256 srchash; int32_t i,flag,bestmatches = 0,matches = 0,paxmatches = 0,paxbestmatches = 0;
    if ( bp->myind < 0 )
        return;
    if ( bp->isratify == 0 && bp->state != 0xffffffff && senderind >= 0 && senderind < bp->numnotaries && bits256_nonz(srcutxo) != 0 && bits256_nonz(destutxo) != 0 )
    {
        if ( bits256_nonz(srcutxo) != 0 )
        {
            bp->notaries[senderind].src.prev_hash = srcutxo;
            bp->notaries[senderind].src.prev_vout = srcvout;
        }
        if ( bits256_nonz(destutxo) != 0 )
        {
            bp->notaries[senderind].dest.prev_hash = destutxo;
            bp->notaries[senderind].dest.prev_vout = destvout;
        }
        if ( bestmask != 0 )
            bp->notaries[senderind].bestmask = bestmask;
        if ( recvmask != 0 )
            bp->notaries[senderind].recvmask |= recvmask;
        if ( (bp->notaries[senderind].paxwdcrc= paxwdcrc) != 0 )
        {
            //fprintf(stderr,"{%d %x} ",senderind,paxwdcrc);
        }
        if ( (bp->notaries[senderind].bestk= bestk) >= 0 )
        {
            if ( (bp->notaries[senderind].src.siglens[bestk]= siglens[0]) != 0 )
            {
                memcpy(bp->notaries[senderind].src.sigs[bestk],sigs[0],siglens[0]);
                if ( bestk == bp->bestk && bestmask == bp->bestmask )
                    bp->srcsigsmasks[bestk] |= (1LL << senderind);
                else bp->srcsigsmasks[bestk] &= ~(1LL << senderind);
            }
            if ( (bp->notaries[senderind].dest.siglens[bestk]= siglens[1]) != 0 )
            {
                memcpy(bp->notaries[senderind].dest.sigs[bestk],sigs[1],siglens[1]);
                if ( bestk == bp->bestk && bestmask == bp->bestmask )
                    bp->destsigsmasks[bestk] |= (1LL << senderind);
                else bp->destsigsmasks[bestk] &= ~(1LL << senderind);
            }
        }
        bp->notaries[bp->myind].paxwdcrc = bp->paxwdcrc;
        if ( bp->bestmask == 0 )
        {
            bp->recvmask |= (1LL << senderind) | (1LL << bp->myind);
            bp->bestmask = dpow_maskmin(bp->recvmask,bp,&bp->bestk);
        }
        dpow_bestconsensus(bp);
        if ( bp->bestk >= 0 )
            bp->notaries[bp->myind].bestk = bp->bestk;
        if ( bp->bestmask != 0 )
            bp->notaries[bp->myind].bestmask = bp->bestmask;
        if ( bp->recvmask != 0 )
            bp->notaries[bp->myind].recvmask = bp->recvmask;
        if ( bp->bestk >= 0 )
        {
            flag = -1;
            for (i=0; i<bp->numnotaries; i++)
            {
                if ( bp->paxwdcrc == bp->notaries[i].paxwdcrc )
                    paxmatches++;
                if ( bp->bestk >= 0 && bp->notaries[i].bestk == bp->bestk && bp->notaries[i].bestmask == bp->bestmask )
                {
                    matches++;
                    if ( ((1LL << i) & bp->bestmask) != 0 )
                    {
                        if ( bp->paxwdcrc == bp->notaries[i].paxwdcrc )
                        {
                            bestmatches++;
                            paxbestmatches++;
                        } //else printf("?%x ",bp->notaries[i].paxwdcrc);
                    }
                }
                else if ( i == senderind && ((1LL << bp->myind) & bp->bestmask) != 0 && ((1LL << i) & bp->bestmask) != 0 && ((1LL << bp->myind) & bp->notaries[i].recvmask) == 0 )
                    flag = senderind;
                if ( 0 && bp->myind <= 1 && bp->notaries[i].paxwdcrc != 0 )
                    printf("%d.(%x %d %llx r%llx) ",i,bp->notaries[i].paxwdcrc,bp->notaries[i].bestk,(long long)bp->notaries[i].bestmask,(long long)bp->notaries[i].recvmask);
            }
            if ( flag >= 0 )
            {
                //printf("flag.%d -> send\n",flag);
                for (i=0; i<sizeof(srchash); i++)
                    srchash.bytes[i] = dp->minerkey33[i+1];
                dpow_send(myinfo,dp,bp,srchash,bp->hashmsg,0,bp->height,(void *)"ping",0);
            }
            if ( 0 && bp->myind <= 1 )
                printf("recv.%llx best.(%d %llx) m.%d p.%d:%d b.%d\n",(long long)bp->recvmask,bp->bestk,(long long)bp->bestmask,matches,paxmatches,paxbestmatches,bestmatches);
            if ( bestmatches >= bp->minsigs && paxbestmatches >= bp->minsigs )
            {
                if ( bp->pendingbestk != bp->bestk || bp->pendingbestmask != bp->bestmask )
                {
                    printf("new PENDING BESTK (%d %llx) state.%d\n",bp->bestk,(long long)bp->bestmask,bp->state);
                    bp->pendingbestk = bp->bestk;
                    bp->pendingbestmask = bp->bestmask;
                    dpow_signedtxgen(myinfo,dp,bp->destcoin,bp,bp->bestk,bp->bestmask,bp->myind,DPOW_SIGBTCCHANNEL,1,0);
                    printf("finished signing\n");
                }
                if ( bp->destsigsmasks[bp->bestk] == bp->bestmask ) // have all sigs
                {
                    if ( bp->state < 1000 )
                        dpow_sigscheck(myinfo,dp,bp,bp->myind,1,bp->bestk,bp->bestmask,0,0);
                    if ( bp->srcsigsmasks[bp->bestk] == bp->bestmask ) // have all sigs
                    {
                        if ( bp->state != 0xffffffff )
                            dpow_sigscheck(myinfo,dp,bp,bp->myind,0,bp->bestk,bp->bestmask,0,0);
                    } //else printf("srcmask.%llx != bestmask.%llx\n",(long long)bp->srcsigsmasks[bp->bestk],(long long)bp->bestmask);
                } //else printf("destmask.%llx != bestmask.%llx\n",(long long)bp->destsigsmasks[bp->bestk],(long long)bp->bestmask);
            }
        }
        else
        {
            for (i=0; i<bp->numnotaries; i++)
            {
                if ( bp->paxwdcrc == bp->notaries[i].paxwdcrc )
                    paxmatches++;
                else if ( 0 && bp->myind <= 1 )
                    printf("%x.%d ",bp->notaries[i].paxwdcrc,i);
            }
            if ( 0 && bp->myind <= 1 )
                printf("mypaxcrc.%x\n",bp->paxwdcrc);
        }
        if ( (rand() % 130) == 0 )
            printf("%p ht.%d [%d] ips.%d %s NOTARIZE.%d matches.%d paxmatches.%d bestmatches.%d bestk.%d %llx recv.%llx sigmasks.(%llx %llx) senderind.%d state.%x (%x %x %x) pax.%x\n",bp,bp->height,bp->myind,dp->numipbits,dp->symbol,bp->minsigs,matches,paxmatches,bestmatches,bp->bestk,(long long)bp->bestmask,(long long)bp->recvmask,(long long)(bp->bestk>=0?bp->destsigsmasks[bp->bestk]:0),(long long)(bp->bestk>=0?bp->srcsigsmasks[bp->bestk]:0),senderind,bp->state,bp->hashmsg.uints[0],bp->desttxid.uints[0],bp->srctxid.uints[0],bp->paxwdcrc);
    }
}

void dpow_nanoutxoget(struct supernet_info *myinfo,struct dpow_info *dp,struct dpow_block *bp,struct dpow_nanoutxo *np,int32_t isratify,int8_t senderind,uint32_t channel)
{
    if ( isratify != 0 )
    {
        dpow_ratify_update(myinfo,dp,bp,senderind,(int8_t)np->bestk,np->bestmask,np->recvmask,np->srcutxo,np->srcvout,np->destutxo,np->destvout,np->siglens,np->sigs,np->pendingcrcs);
    }
    else
    {
        dpow_notarize_update(myinfo,dp,bp,senderind,(int8_t)np->bestk,np->bestmask,np->recvmask,np->srcutxo,np->srcvout,np->destutxo,np->destvout,np->siglens,np->sigs,np->paxwdcrc);
        if ( 0 && bp->myind <= 2 )
            printf("lag.[%d] RECV.%d r%llx (%d %llx) %llx/%llx\n",(int32_t)(time(NULL)-channel),senderind,(long long)np->recvmask,(int8_t)np->bestk,(long long)np->bestmask,(long long)np->srcutxo.txid,(long long)np->destutxo.txid);
    }
    //dpow_bestmask_update(myinfo,dp,bp,nn_senderind,nn_bestk,nn_bestmask,nn_recvmask);
}

void dpow_send(struct supernet_info *myinfo,struct dpow_info *dp,struct dpow_block *bp,bits256 srchash,bits256 desthash,uint32_t channel,uint32_t msgbits,uint8_t *data,int32_t datalen)
{
    struct dpow_nanomsghdr *np; int32_t i,size,extralen=0,sentbytes = 0; uint32_t crc32,paxwdcrc; uint8_t extras[10000];
    if ( bp->myind < 0 )
        return;
    if ( time(NULL) < myinfo->nanoinit+5 )
        return;
    crc32 = calc_crc32(0,data,datalen);
     //dp->crcs[firstz] = crc32;
    size = (int32_t)(sizeof(*np) + datalen);
    np = calloc(1,size); // endian dependent!
    if ( (np->numipbits= dp->numipbits) == 0 )
    {
        dp->ipbits[0] = myinfo->myaddr.myipbits;
        np->numipbits = dp->numipbits = 1;
    }
    np->senderind = bp->myind;
    memcpy(np->ipbits,dp->ipbits,dp->numipbits * sizeof(*dp->ipbits));
    //for (i=0; i<np->numipbits; i++)
    //    printf("%08x ",np->ipbits[i]);
    //printf(" dpow_send.(%d) size.%d numipbits.%d myind.%d\n",datalen,size,np->numipbits,bp->myind);
    if ( bp->isratify == 0 )
    {
        extralen = dpow_paxpending(extras,&paxwdcrc);
        bp->paxwdcrc = bp->notaries[bp->myind].paxwdcrc = np->notarize.paxwdcrc = paxwdcrc;
        //dpow_bestconsensus(bp);
        dpow_nanoutxoset(myinfo,dp,&np->notarize,bp,0);
    }
    else
    {
        bp->paxwdcrc = bp->notaries[bp->myind].paxwdcrc = np->notarize.paxwdcrc = 0;
        dpow_nanoutxoset(myinfo,dp,&np->ratify,bp,1);
    }
    np->size = size;
    np->datalen = datalen;
    np->crc32 = crc32;
    for (i=0; i<2; i++)
        np->ratify.pendingcrcs[i] = bp->pendingcrcs[i];
    for (i=0; i<32; i++)
        np->srchash.bytes[i] = dp->minerkey33[i+1];
    np->desthash = desthash;
    np->channel = channel;
    np->height = bp->height;//msgbits;
    np->myipbits = myinfo->myaddr.myipbits;
    strcpy(np->symbol,dp->symbol);
    np->version0 = DPOW_VERSION & 0xff;
    np->version1 = (DPOW_VERSION >> 8) & 0xff;
    memcpy(np->packet,data,datalen);
    sentbytes = -1;
    // deadlocks! portable_mutex_lock(&myinfo->dpowmutex);
    for (i=0; i<100; i++)
    {
        struct nn_pollfd pfd;
        pfd.fd = myinfo->dpowsock;
        pfd.events = NN_POLLOUT;
        if ( nn_poll(&pfd,1,100) > 0 )
        {
            sentbytes = signed_nn_send(myinfo,myinfo->ctx,myinfo->persistent_priv,myinfo->dpowsock,np,size);
            break;
        }
        usleep(1000);
    }
    //portable_mutex_unlock(&myinfo->dpowmutex);
    free(np);
    if ( 0 && bp->myind <= 2 )
        printf("%d NANOSEND.%d ht.%d channel.%08x (%d) pax.%08x datalen.%d (%d %llx) (%d %llx) recv.%llx\n",i,sentbytes,np->height,np->channel,size,np->notarize.paxwdcrc,datalen,(int8_t)np->notarize.bestk,(long long)np->notarize.bestmask,bp->notaries[bp->myind].bestk,(long long)bp->notaries[bp->myind].bestmask,(long long)bp->recvmask);
}

void dpow_ipbitsadd(struct supernet_info *myinfo,struct dpow_info *dp,uint32_t *ipbits,int32_t numipbits,int32_t fromid,uint32_t senderipbits)
{
    int32_t i,j,matched,missing,n; char ipaddr[64];
    if ( numipbits >= 64 )
    {
        static int32_t counter;
        if ( counter++ < 100 )
            printf("dpow_ipbitsadd reject from.%d numipbits.%d\n",fromid,numipbits);
        return;
    }
    n = dp->numipbits;
    matched = missing = 0;
    //for (i=0; i<numipbits; i++)
    //    printf("%08x ",ipbits[i]);
    for (i=0; i<numipbits; i++)
    {
        for (j=0; j<n; j++)
            if ( ipbits[i] == dp->ipbits[j] )
            {
                matched++;
                ipbits[i] = 0;
                break;
            }
        if ( j == n )
            missing++;
    }
    if ( (numipbits == 1 || missing < matched || matched > 0) && missing > 0 )
    {
        for (i=0; i<numipbits; i++)
            if ( ipbits[i] != 0 )
            {
                expand_ipbits(ipaddr,ipbits[i]);
                //printf("ADD NOTARY.(%s) %08x\n",ipaddr,ipbits[i]);
                dpow_addnotary(myinfo,dp,ipaddr);
            }
    } else if ( missing > 0 )
        printf("IGNORE from.%d RECV numips.%d numipbits.%d matched.%d missing.%d\n",fromid,numipbits,n,matched,missing);
    expand_ipbits(ipaddr,senderipbits);
    dpow_addnotary(myinfo,dp,ipaddr);
    expand_ipbits(ipaddr,myinfo->myaddr.myipbits);
    dpow_addnotary(myinfo,dp,ipaddr);
    //printf("recv numips.(%d %d)\n",myinfo->numdpowipbits,dp->numipbits);
}

int32_t dpow_nanomsg_update(struct supernet_info *myinfo)
{
    int32_t i,n,num,num2,flags=0,size,iter,broadcastflag,firstz = -1; char *retstr; uint32_t crc32,r,m; struct dpow_nanomsghdr *np=0; struct dpow_info *dp; struct dpow_block *bp; struct dex_nanomsghdr *dexp = 0; void *freeptr;
    if ( time(NULL) < myinfo->nanoinit+5 || (myinfo->dpowsock < 0 && myinfo->dexsock < 0 && myinfo->repsock < 0) )
        return(-1);
    if ( myinfo->IAMNOTARY != 0 && myinfo->numnotaries <= 0 )
    {
        myinfo->numnotaries = komodo_notaries("KMD",myinfo->notaries,-1);
        printf("INIT with %d notaries\n",myinfo->numnotaries);
    }
    portable_mutex_lock(&myinfo->dpowmutex);
    num = num2 = n = 0;
    for (iter=0; iter<1000; iter++)
    {
        if ( (flags & 1) == 0 && (size= signed_nn_recv(&freeptr,myinfo->ctx,myinfo->notaries,myinfo->numnotaries,myinfo->dpowsock,&np)) >= 0 )
        {
            num++;
            if ( size > 0 )
            {
                //fprintf(stderr,"%d ",size);
                if ( np->version0 == (DPOW_VERSION & 0xff) && np->version1 == ((DPOW_VERSION >> 8) & 0xff) )
                {
                    //printf("v.%02x %02x datalen.%d size.%d %d vs %d\n",np->version0,np->version1,np->datalen,size,np->datalen,(int32_t)(size - sizeof(*np)));
                    if ( np->datalen == (size - sizeof(*np)) )
                    {
                        crc32 = calc_crc32(0,np->packet,np->datalen);
                        dp = 0;
                        for (i=0; i<myinfo->numdpows; i++)
                        {
                            if ( strcmp(np->symbol,myinfo->DPOWS[i].symbol) == 0 )
                            {
                                dp = &myinfo->DPOWS[i];
                                break;
                            }
                        }
                        if ( dp != 0 && crc32 == np->crc32 )
                        {
                            if ( i == myinfo->numdpows )
                                printf("received nnpacket for (%s)\n",np->symbol);
                            else
                            {
                                dpow_ipbitsadd(myinfo,dp,np->ipbits,np->numipbits,np->senderind,np->myipbits);
                                if ( (bp= dpow_heightfind(myinfo,dp,np->height)) != 0 && bp->state != 0xffffffff && bp->myind >= 0 )
                                {
                                    //char str[65]; printf("%s RECV ht.%d ch.%08x (%d) crc32.%08x:%08x datalen.%d:%d firstz.%d i.%d senderind.%d myind.%d\n",bits256_str(str,np->srchash),np->height,np->channel,size,np->crc32,crc32,np->datalen,(int32_t)(size - sizeof(*np)),firstz,i,np->senderind,bp->myind);
                                    if ( np->senderind >= 0 && np->senderind < bp->numnotaries )
                                    {
                                        if ( memcmp(bp->notaries[np->senderind].pubkey+1,np->srchash.bytes,32) == 0 && bits256_nonz(np->srchash) != 0 )
                                        {
                                            if ( bp->isratify == 0 )
                                                dpow_nanoutxoget(myinfo,dp,bp,&np->notarize,0,np->senderind,np->channel);
                                            else dpow_nanoutxoget(myinfo,dp,bp,&np->ratify,1,np->senderind,np->channel);
                                            dpow_datahandler(myinfo,dp,bp,np->senderind,np->channel,np->height,np->packet,np->datalen);
                                        } else printf("wrong senderind.%d\n",np->senderind);
                                    }
                                } //else printf("height.%d bp.%p state.%x senderind.%d\n",np->height,bp,bp!=0?bp->state:0,np->senderind);
                                //dp->crcs[firstz] = crc32;
                            }
                        } //else printf("crc error from.%d %x vs %x or no dp.%p [%s]\n",np->senderind,crc32,np->crc32,dp,np->symbol);
                    } else printf("ignore.%d np->datalen.%d %d (size %d - %ld) [%s]\n",np->senderind,np->datalen,(int32_t)(size-sizeof(*np)),size,sizeof(*np),np->symbol);
                } //else printf("wrong version from.%d %02x %02x size.%d [%s]\n",np->senderind,np->version0,np->version1,size,np->symbol);
            }
            if ( freeptr != 0 )
                nn_freemsg(freeptr), np = 0, freeptr = 0;
        } else flags |= 1;
        if ( myinfo->dexsock >= 0 ) // from servers
        {
            if ( (flags & 2) == 0 && (size= signed_nn_recv(&freeptr,myinfo->ctx,myinfo->notaries,myinfo->numnotaries,myinfo->dexsock,&dexp)) > 0 )
            {
                //fprintf(stderr,"%d ",size);
                n++;
                if ( dex_packetcheck(myinfo,dexp,size) == 0 )
                {
                    //printf("FROM BUS.%08x -> pub\n",dexp->crc32);
                    signed_nn_send(myinfo,myinfo->ctx,myinfo->persistent_priv,myinfo->pubsock,dexp,size);
                    dex_packet(myinfo,dexp,size);
                }
                //printf("GOT DEX bus PACKET.%d\n",size);
                if ( freeptr != 0 )
                    nn_freemsg(freeptr), dexp = 0, freeptr = 0;
            } else flags |= 2;
        }
        if ( myinfo->repsock >= 0 ) // from clients
        {
            if ( (flags & 4) == 0 && (size= nn_recv(myinfo->repsock,&dexp,NN_MSG,0)) > 0 )
            {
                num2++;
                printf("REP got %d crc.%08x\n",size,calc_crc32(0,(void *)dexp,size));
                if ( (retstr= dex_response(&broadcastflag,myinfo,dexp)) != 0 )
                {
                    signed_nn_send(myinfo,myinfo->ctx,myinfo->persistent_priv,myinfo->repsock,retstr,(int32_t)strlen(retstr)+1);
                    printf("send back[%ld]\n",strlen(retstr)+1);
                    free(retstr);
                    if ( broadcastflag != 0 )
                    {
                        //printf("BROADCAST dexp request.[%d]\n",size);
                        //signed_nn_send(myinfo,myinfo->ctx,myinfo->persistent_priv,myinfo->dexsock,dexp,size);
                    }
                }
                else
                {
                    if ( (m= myinfo->numdpowipbits) > 0 )
                    {
                        r = myinfo->dpowipbits[rand() % m];
                        signed_nn_send(myinfo,myinfo->ctx,myinfo->persistent_priv,myinfo->repsock,&r,sizeof(r));
                        printf("REP.%08x <- rand ip m.%d %x\n",dexp->crc32,m,r);
                    } else printf("illegal state without dpowipbits?\n");
                    if ( dex_packetcheck(myinfo,dexp,size) == 0 )
                    {
                        signed_nn_send(myinfo,myinfo->ctx,myinfo->persistent_priv,myinfo->dexsock,dexp,size);
                        signed_nn_send(myinfo,myinfo->ctx,myinfo->persistent_priv,myinfo->pubsock,dexp,size);
                        printf("REP.%08x -> dexbus and pub, t.%d lag.%d\n",dexp->crc32,dexp->timestamp,(int32_t)(time(NULL)-dexp->timestamp));
                        dex_packet(myinfo,dexp,size);
                    } else printf("failed dexpacketcheck\n");
                }
                //printf("GOT DEX rep PACKET.%d\n",size);
                //if ( freeptr != 0 )
                //    nn_freemsg(freeptr), dexp = 0, freeptr = 0;
                if ( dexp != 0 )
                    nn_freemsg(dexp), dexp = 0;
                //if ( num > 1000 )
                //    break;
            } else flags |= 4;
            printf("num.%d n.%d num2.%d rep packets\n",num,n,num2);
        }
    }
    portable_mutex_unlock(&myinfo->dpowmutex);
    return(num);
}
#else

void dpow_nanomsginit(struct supernet_info *myinfo,char *ipaddr) { }

void dpow_send(struct supernet_info *myinfo,struct dpow_info *dp,struct dpow_block *bp,bits256 srchash,bits256 desthash,uint32_t channel,uint32_t msgbits,uint8_t *data,int32_t datalen)
{
    return;
}

int32_t dpow_nanomsg_update(struct supernet_info *myinfo) { return(0); }

#endif

int32_t dpow_opreturnscript(uint8_t *script,uint8_t *opret,int32_t opretlen)
{
    int32_t offset = 0;
    script[offset++] = 0x6a;
    if ( opretlen >= 0x4c )
    {
        if ( opretlen > 0xff )
        {
            script[offset++] = 0x4d;
            script[offset++] = opretlen & 0xff;
            script[offset++] = (opretlen >> 8) & 0xff;
        }
        else
        {
            script[offset++] = 0x4c;
            script[offset++] = opretlen;
        }
    } else script[offset++] = opretlen;
    memcpy(&script[offset],opret,opretlen);
    return(opretlen + offset);
}

int32_t dpow_rwopret(int32_t rwflag,uint8_t *opret,bits256 *hashmsg,int32_t *heightmsgp,char *src,uint8_t *extras,int32_t extralen,struct dpow_block *bp,int32_t src_or_dest)
{
    int32_t i,opretlen = 0; //bits256 beacon,beacons[DPOW_MAXRELAYS];
    opretlen += iguana_rwbignum(rwflag,&opret[opretlen],sizeof(*hashmsg),hashmsg->bytes);
    opretlen += iguana_rwnum(rwflag,&opret[opretlen],sizeof(*heightmsgp),(uint32_t *)heightmsgp);
    if ( src_or_dest == 0 )
    {
        //char str[65]; printf("src_or_dest.%d opreturn add %s\n",src_or_dest,bits256_str(str,bp->desttxid));
        if ( bits256_nonz(bp->desttxid) == 0 )
        {
            printf("no desttxid\n");
            return(-1);
        }
        opretlen += iguana_rwbignum(rwflag,&opret[opretlen],sizeof(bp->desttxid),bp->desttxid.bytes);
    }
    /*else if ( 0 )
    {
        memset(beacons,0,sizeof(beacons));
        for (i=0; i<bp->numnotaries; i++)
        {
            if ( ((1LL << i) & bp->bestmask) != 0 )
                beacons[i] = bp->notaries[i].beacon;
        }
        vcalc_sha256(0,beacon.bytes,beacons[0].bytes,sizeof(*beacons) * bp->numnotaries);
        opretlen += iguana_rwbignum(rwflag,&opret[opretlen],sizeof(beacon),beacon.bytes);
    }*/
    if ( rwflag != 0 )
    {
        if ( src != 0 )
        {
            for (i=0; src[i]!=0; i++)
                opret[opretlen++] = src[i];
        }
        opret[opretlen++] = 0;
        if ( extras != 0 && extralen > 0 )
        {
            memcpy(&opret[opretlen],extras,extralen);
            opretlen += extralen;
            printf("added extra.%d opreturn for withdraws paxwdcrc.%08x\n",extralen,calc_crc32(0,extras,extralen));
        }
    }
    else
    {
        if ( src != 0 )
        {
            for (i=0; opret[opretlen]!=0; i++)
                src[i] = opret[opretlen++];
            src[i] = 0;
        }
        opretlen++;
    }
    return(opretlen);
}

int32_t dpow_rwsigentry(int32_t rwflag,uint8_t *data,struct dpow_sigentry *dsig)
{
    int32_t i,len = 0;
    if ( rwflag != 0 )
    {
        data[len++] = dsig->senderind;
        data[len++] = dsig->lastk;
        len += iguana_rwnum(rwflag,&data[len],sizeof(dsig->mask),(uint8_t *)&dsig->mask);
        data[len++] = dsig->siglen;
        memcpy(&data[len],dsig->sig,dsig->siglen), len += dsig->siglen;
        for (i=0; i<sizeof(dsig->beacon); i++)
            data[len++] = dsig->beacon.bytes[i];
        for (i=0; i<33; i++)
            data[len++] = dsig->senderpub[i];
    }
    else
    {
        memset(dsig,0,sizeof(*dsig));
        dsig->senderind = data[len++];
        if ( dsig->senderind < 0 || dsig->senderind >= DPOW_MAXRELAYS )
            return(-1);
        dsig->lastk = data[len++];
        len += iguana_rwnum(rwflag,&data[len],sizeof(dsig->mask),(uint8_t *)&dsig->mask);
        dsig->siglen = data[len++];
        memcpy(dsig->sig,&data[len],dsig->siglen), len += dsig->siglen;
        for (i=0; i<sizeof(dsig->beacon); i++)
            dsig->beacon.bytes[i] = data[len++];
        for (i=0; i<33; i++)
            dsig->senderpub[i] = data[len++];
    }
    return(len);
}

void dpow_sigsend(struct supernet_info *myinfo,struct dpow_info *dp,struct dpow_block *bp,int32_t myind,int8_t bestk,uint64_t bestmask,bits256 srchash,uint32_t sigchannel)
{
    struct dpow_sigentry dsig; int32_t i,len; uint8_t data[4096]; struct dpow_entry *ep;
    if ( bp->myind < 0 )
        return;
    if ( ((1LL << myind) & bestmask) == 0 )
        return;
    ep = &bp->notaries[myind];
    if ( bestk >= 0 )
    {
        if ( sigchannel == DPOW_SIGCHANNEL )
            bp->srcsigsmasks[bestk] |= (1LL << myind);
        else bp->destsigsmasks[bestk] |= (1LL << myind);
    }
    //printf("ht.%d sigsend.%s: myind.%d bestk.%d %llx >>>>>> best.(%d %llx) recv.%llx sigs.%llx\n",bp->height,sigchannel == DPOW_SIGCHANNEL ? bp->srccoin->symbol : bp->destcoin->symbol,myind,bestk,(long long)bestmask,bestk,(long long)(bestk>=0?bestmask:0),(long long)bp->recvmask,(long long)(bestk>=0?bp->destsigsmasks[bestk]:0));
    memset(&dsig,0,sizeof(dsig));
    for (i=0; i<33; i++)
        dsig.senderpub[i] = dp->minerkey33[i];
    dsig.lastk = bestk;
    dsig.mask = bestmask;
    dsig.senderind = myind;
    dsig.beacon = bp->beacon;
    if ( sigchannel == DPOW_SIGBTCCHANNEL )
    {
        dsig.siglen = ep->dest.siglens[bestk];
        memcpy(dsig.sig,ep->dest.sigs[bestk],ep->dest.siglens[bestk]);
    }
    else
    {
        dsig.siglen = ep->src.siglens[bestk];
        memcpy(dsig.sig,ep->src.sigs[bestk],ep->src.siglens[bestk]);
    }
    memcpy(dsig.senderpub,dp->minerkey33,33);
    len = dpow_rwsigentry(1,data,&dsig);
    dpow_send(myinfo,dp,bp,srchash,bp->hashmsg,sigchannel,bp->height,data,len);
}

uint32_t komodo_assetmagic(char *symbol,uint64_t supply)
{
    uint8_t buf[512]; int32_t len = 0;
    len = iguana_rwnum(1,&buf[len],sizeof(supply),(void *)&supply);
    strcpy((char *)&buf[len],symbol);
    len += strlen(symbol);
    return(calc_crc32(0,buf,len));
}

/*int32_t komodo_shortflag(char *symbol)
{
    int32_t i,shortflag = 0;
    if ( symbol[0] == '-' )
    {
        shortflag = 1;
        for (i=0; symbol[i+1]!=0; i++)
            symbol[i] = symbol[i+1];
        symbol[i] = 0;
    }
    return(shortflag);
}*/

uint16_t komodo_assetport(uint32_t magic)
{
    return(8000 + (magic % 7777));
}

uint16_t komodo_port(char *symbol,uint64_t supply,uint32_t *magicp)
{
    *magicp = komodo_assetmagic(symbol,supply);
    return(komodo_assetport(*magicp));
}

#define MAX_CURRENCIES 32
extern char CURRENCIES[][8];

void komodo_assetcoins(int32_t fullnode,uint64_t mask)
{
    uint16_t extract_userpass(char *serverport,char *userpass,char *coinstr,char *userhome,char *coindir,char *confname);
    int32_t i,j; uint32_t magic; cJSON *json; uint16_t port; long filesize; char *userhome,confstr[16],jsonstr[512],magicstr[9],path[512]; struct iguana_info *coin;
    if ( (userhome= OS_filestr(&filesize,"userhome.txt")) == 0 )
        userhome = "root";
    else
    {
        while ( userhome[strlen(userhome)-1] == '\r' || userhome[strlen(userhome)-1] == '\n' )
            userhome[strlen(userhome)-1] = 0;
    }
    for (i=0; i<MAX_CURRENCIES; i++)
    {
        if ( ((1LL << i) & mask) == 0 )
            continue;
        port = komodo_port(CURRENCIES[i],10,&magic);
        for (j=0; j<4; j++)
            sprintf(&magicstr[j*2],"%02x",((uint8_t *)&magic)[j]);
        magicstr[j*2] = 0;
        sprintf(jsonstr,"{\"newcoin\":\"%s\",\"RELAY\":%d,\"VALIDATE\":0,\"portp2p\":%u,\"rpcport\":%u,\"netmagic\":\"%s\"}",CURRENCIES[i],fullnode,port,port+1,magicstr);
        if ( (json= cJSON_Parse(jsonstr)) != 0 )
        {
            if ( (coin= iguana_coinadd(CURRENCIES[i],CURRENCIES[i],json,0)) == 0 )
            {
                printf("Cant create (%s)\n",CURRENCIES[i]);
                return;
            }
            free_json(json);
            coin->FULLNODE = fullnode;
            coin->chain->rpcport = port + 1;
            coin->chain->pubtype = 60;
            coin->chain->p2shtype = 85;
            coin->chain->wiftype = 188;
            if ( fullnode < 0 )
            {
                sprintf(confstr,"%s.conf",CURRENCIES[i]);
                sprintf(path,"%s/.komodo/%s",userhome,CURRENCIES[i]);
                extract_userpass(coin->chain->serverport,coin->chain->userpass,CURRENCIES[i],coin->chain->userhome,path,confstr);
            }
        }
        printf("(%s %u) ",CURRENCIES[i],port);
    }
    printf("ports\n");
}

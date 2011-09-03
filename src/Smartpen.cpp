/* This file is part of LibreScribe.

LibreScribe is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

LibreScribe is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with LibreScribe.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "Smartpen.h"

static void obex_requestdone (struct obex_state *state, obex_t *hdl,
                              obex_object_t *obj, int cmd, int resp)
{
	uint8_t header_id;
	obex_headerdata_t hdata;
	uint32_t hlen;

	switch (cmd & ~OBEX_FINAL) {
		case OBEX_CMD_CONNECT:
			while (OBEX_ObjectGetNextHeader(hdl, obj, &header_id,
							&hdata, &hlen)) {
				if (header_id == OBEX_HDR_CONNECTION) {
					state->got_connid=1;
					state->connid = hdata.bq4;
//					printf("Connection ID: %d\n",
//					       state->connid);
				}
			}
			break;

		case OBEX_CMD_GET:
			while (OBEX_ObjectGetNextHeader(hdl, obj, &header_id,
							&hdata, &hlen)) {
				if (header_id == OBEX_HDR_BODY ||
				    header_id == OBEX_HDR_BODY_END) {
					if (state->body)
						free(state->body);
					state->body = (char*)malloc(hlen);
					state->body_len = hlen;
					memcpy(state->body, hdata.bs, hlen);
					break;
				}
			}
			break;
	}

	state->req_done++;
}

static void obex_event (obex_t *hdl, obex_object_t *obj, int mode,
			int event, int obex_cmd, int obex_rsp)
{
	struct obex_state *state;
	obex_headerdata_t hd;
	int size;
	int rc;

	state = (obex_state*)OBEX_GetUserData(hdl);

	if (event == OBEX_EV_PROGRESS) {
		hd.bq4 = state->connid;
		size = 4;
		rc = OBEX_ObjectAddHeader(hdl, obj, OBEX_HDR_CONNECTION,
					  hd, size, OBEX_FL_FIT_ONE_PACKET);
		if (rc < 0) {
			printf("oah fail %d\n", rc);
		}
		return;
	}

	if (obex_rsp != OBEX_RSP_SUCCESS && obex_rsp != OBEX_RSP_CONTINUE) {
		printf("FAIL %x %x\n", obex_rsp, event);
//		assert(0);
		state->req_done++;
		return;
	}

	switch (event) {
		case OBEX_EV_REQDONE:
			obex_requestdone(state, hdl, obj, obex_cmd, obex_rsp);
			break;

		default:
			printf("Funny event\n");
	}
}

static void pen_reset (short vendor, short product)
{
	libusb_context *ctx;
	libusb_device_handle *dev;
	int rc;

//	printf("swizzle\n");

	rc = libusb_init(&ctx);
	assert(rc == 0);
	dev = libusb_open_device_with_vid_pid(ctx, vendor, product);
	libusb_reset_device(dev);
	libusb_close(dev);
	libusb_exit(ctx);
}

static void swizzle_usb (short vendor, short product)
{
	libusb_context *ctx;
	libusb_device_handle *dev;
	int rc;

//	printf("swizzle\n");

	rc = libusb_init(&ctx);
	assert(rc == 0);
	dev = libusb_open_device_with_vid_pid(ctx, vendor, product);
	assert(dev);

	libusb_set_configuration(dev, 1);
	libusb_set_interface_alt_setting(dev, 1, 0);
	libusb_set_interface_alt_setting(dev, 1, 1);

	libusb_close(dev);
	libusb_exit(ctx);
}

static char *get_named_object(obex_t *handle, char *name, int *len);

obex_t* smartpen_connect(short vendor, short product) {
	obex_t *handle;
	obex_object_t *obj;
	int rc, num, i;
	struct obex_state *state;
	obex_interface_t *obex_intf;
	obex_headerdata_t hd;
	int size, count;
	char *buf;

again:
	handle = OBEX_Init(OBEX_TRANS_USB, obex_event, 0);
	if (!handle)
		goto out;

        num = OBEX_FindInterfaces(handle, &obex_intf);
	for (i=0; i<num; i++) {
        if (!strcmp(obex_intf[i].usb.manufacturer, "Livescribe")) {
            break;
		}
	}

        if (i == num) {
		printf("No such device\n");
		handle = NULL;
		goto out;
        }

	state = (obex_state*)malloc(sizeof(struct obex_state));
	if (!state) {
		handle = NULL;
		goto out;
	}
	memset(state, 0, sizeof(struct obex_state));

	swizzle_usb(vendor, product);

        rc = OBEX_InterfaceConnect(handle, &obex_intf[i]);
        if (rc < 0) {
        printf("Sorry! Connecting to your device failed. Miserably. Is it in use already?\n");
        printf("Connect failed %d\n", rc);
		handle = NULL;
		goto out;
	}

        OBEX_SetUserData(handle, state);
        OBEX_SetTransportMTU(handle, 0x400, 0x400);

        obj = OBEX_ObjectNew(handle, OBEX_CMD_CONNECT);
        hd.bs = (unsigned char *)"LivescribeService";
        size = strlen((char*)hd.bs)+1;
        OBEX_ObjectAddHeader(handle, obj, OBEX_HDR_TARGET, hd, size, 0);

        rc = OBEX_Request(handle, obj);

	count = state->req_done;
        while (rc == 0 && state->req_done <= count) {
            OBEX_HandleInput(handle, 100);
        }

	if (rc < 0 || !state->got_connid) {
		printf("Retry connection...\n");
		OBEX_Cleanup(handle);
		goto again;
	}

	buf = get_named_object(handle, "ppdata?key=pp0000", &rc);
	if (!buf) {
		printf("Retry connection...\n");
		OBEX_Cleanup(handle);
		pen_reset(vendor, product);
		goto again;
	}

out:
	return handle;
}

static char *get_named_object(obex_t *handle, char *name, int *len)
{
    printf("attempting to retrieve named object \"%s\"...\n",name);
	struct obex_state *state;
	int req_done;
	obex_object_t *obj;
	obex_headerdata_t hd;
	int size, i;
	glong num;

	state = (obex_state*)OBEX_GetUserData(handle);
	OBEX_SetTransportMTU(handle, OBEX_MAXIMUM_MTU, OBEX_MAXIMUM_MTU);
	obj = OBEX_ObjectNew(handle, OBEX_CMD_GET);
	size = 4;
//	printf("Setting connection id header to state->connid (%d)\n",state->connid);
	hd.bq4 = state->connid;
//	printf("Adding connection id header...\n");
	OBEX_ObjectAddHeader(handle, obj, OBEX_HDR_CONNECTION,
			     hd, size, OBEX_FL_FIT_ONE_PACKET);
//    printf("converting name from utf8 to utf16\n");
	hd.bs = (unsigned char *)g_utf8_to_utf16(name, strlen(name),
						 NULL, &num, NULL);

	for (i=0; i<num; i++) {
		uint16_t *wchar = (uint16_t*)&hd.bs[i*2];
		*wchar = ntohs(*wchar);
	}
	size = (num+1) * sizeof(uint16_t);
//	printf("Adding name header...\n");
	OBEX_ObjectAddHeader(handle, obj, OBEX_HDR_NAME, hd, size, OBEX_FL_FIT_ONE_PACKET);

	if (OBEX_Request(handle, obj) < 0) {
	    OBEX_ObjectDelete(handle,obj);
        printf("an error occured while retrieving the object. returning null value.\n");
		return NULL;
	}

	req_done = state->req_done;
	while (state->req_done == req_done) {
		OBEX_HandleInput(handle, 100);
	}
//    printf("done handling input\n");
	if (state->body) {
		*len = state->body_len;
		state->body[state->body_len] = '\0';
	} else {
		*len = 0;
	}
	return state->body;
}

static bool put_named_object(obex_t *handle, char *name, char *body)
{
    //reference: http://dev.zuckschwerdt.org/openobex/doxygen/
    printf("Attempting to set \"%s\" to \"%s\"\n",name,body);
    struct obex_state *state;
    int req_done;
    obex_object_t *obj;
	obex_headerdata_t hd;
	int name_size, body_size, i;
	glong nnum;
	glong bnum;
	printf("getting obex state...\n");
	state = (obex_state*)OBEX_GetUserData(handle);
	OBEX_SetTransportMTU(handle, OBEX_MAXIMUM_MTU, OBEX_MAXIMUM_MTU);
    printf("creating object\n");
	obj = OBEX_ObjectNew(handle, OBEX_CMD_PUT);
    if(obj== NULL) {
        return false;
    }
    body_size = strlen(body);
	printf("determining body size: %d\n",body_size);
    printf("Adding length header...\n");
    /* Add length header */
    hd.bq4 = body_size;
    printf("length: %d\n", body_size);
    OBEX_ObjectAddHeader(handle, obj, OBEX_HDR_LENGTH, hd, 4, OBEX_FL_FIT_ONE_PACKET);
	printf("Setting connection id header to state->connid (%d)\n",state->connid);
	hd.bq4 = state->connid;
	printf("Adding connection id header...\n");
	OBEX_ObjectAddHeader(handle, obj, OBEX_HDR_CONNECTION,
			     hd, 4, OBEX_FL_FIT_ONE_PACKET);
    printf("Adding unicode name header...\n");
    /* Add unicode name header*/
    hd.bs = (unsigned char *)g_utf8_to_utf16(name, strlen(name),
						 NULL, &nnum, NULL);
    for (i=0; i<nnum; i++) {
		uint16_t *wchar = (uint16_t*)&hd.bs[i*2];
		*wchar = ntohs(*wchar);
	}
    name_size = (nnum+1) * sizeof(uint16_t);
    printf("name size: %d\n", name_size);
    OBEX_ObjectAddHeader(handle, obj, OBEX_HDR_NAME, hd, name_size, OBEX_FL_FIT_ONE_PACKET);
    printf("Adding body header...\n");
    /* Add body header*/
    hd.bs = (unsigned char*)body;
    printf("body size: %d\n", body_size);
    OBEX_ObjectAddHeader(handle, obj, OBEX_HDR_BODY, hd, body_size, OBEX_FL_FIT_ONE_PACKET);
    printf("Sending request...\n");
    return (OBEX_Request(handle, obj) >= 0);
}

char *smartpen_get_changelist(obex_t *handle, int starttime)
{
	char name[256];
	int len;

	snprintf(name, sizeof(name), "changelist?start_time=%d", starttime);
	return get_named_object(handle, name, &len);
}

void smartpen_disconnect (obex_t *handle)
{
	struct obex_state *state;
	int req_done;
	obex_object_t *obj;
	obex_headerdata_t hd;
	int size;

	state = (obex_state*)OBEX_GetUserData(handle);
	obj = OBEX_ObjectNew(handle, OBEX_CMD_DISCONNECT);
	hd.bq4 = state->connid;
	size = 4;
	OBEX_ObjectAddHeader(handle, obj, OBEX_HDR_CONNECTION,
			     hd, size, OBEX_FL_FIT_ONE_PACKET);

	if (OBEX_Request(handle, obj) < 0)
		return;

	req_done = state->req_done;
	while (state->req_done == req_done) {
		OBEX_HandleInput(handle, 100);
	}
	handle = NULL;
	printf("disconnected.\n");
}

int smartpen_get_guid (obex_t *handle, FILE *out, char *guid,
		       long long int start_time)
{
	char name[256];
	char *buf;
	int len;

	snprintf(name, sizeof(name), "lspdata?name=%s&start_time=%lld",
		 guid, start_time);

	buf = get_named_object(handle, name, &len);
	fwrite(buf, len, 1, out);
	return len;
}

//int smartpen_get_paperreplay (obex_t *handle, FILE *out,
//			      long long int start_time)
//{
//	char name[256];
//	char *buf;
//	int len;
//
//	snprintf(name, sizeof(name), "lspdata?name=com.livescribe.paperreplay.PaperReplay&start_time=%lld&returnVersion=0.3&remoteCaller=WIN_LD_200",
//		 start_time);
//
//	buf = get_named_object(handle, name, &len);
//	fwrite(buf, len, 1, out);
//	return 1;
//}

char* smartpen_get_paperreplay (obex_t *handle, long long int start_time) {
    char name[256];
	char *buf;
	int len;

	snprintf(name, sizeof(name), "lspdata?name=com.livescribe.paperreplay.PaperReplay&start_time=%lld&returnVersion=0.3&remoteCaller=WIN_LD_200",
		 start_time);

    return  get_named_object(handle, name, &len);
}

char* smartpen_get_penletlist(obex_t *handle) {
	char *name = "penletlist";
	int len;

	return get_named_object(handle, name, &len);
}


std::string to_hex ( const std::string& src, bool spaces ) {
  //source: http://www.programmingforums.org/thread14348.html
  std::stringstream out;

  for ( std::string::size_type i = 0; i < src.size(); i++ ) {
    out<< std::hex << int ( src[i] );

    if (( i < src.size() - 1 ) && (spaces))
      out<<' ';
  }

  return out.str();
}

std::string from_hex ( const std::string& src ) {
    //source: http://www.programmingforums.org/thread14348.html
  std::stringstream in(src);
  std::string ret;
  int byte;

  while ( in>> std::hex >> byte ) {
    ret += char ( byte );
  }
  return ret;
}

std::string space_hex ( const std::string& src ) {
     std::string ret;
     for(int x = 0; x < src.length(); x += 2) ret += src.substr(x,2) + ' ';
     return ret;
}

const char* get_parameter_value(char * xml) {
        xmlDocPtr doc = xmlParseMemory(xml, strlen(xml));
        xmlNodePtr cur = xmlDocGetRootElement(doc); //current element should be "xml" at this point.
        if (cur == NULL) {
            printf("cur is NULL!\n");
            xmlFreeDoc(doc);
            return ""; //do nothing if the xml document is empty
        }
        if ((xmlStrcmp(cur->name, (const xmlChar *)"xml")) != 0) return ""; //do nothing if the current element's name is not 'xml'
        cur = cur->children;
        for (cur = cur; cur; cur = cur->next) {
            if (cur->type == XML_ELEMENT_NODE) {
                if ((!xmlStrcmp(cur->name, (const xmlChar *)"parameter"))) { //if the current element's name is 'parameter'
                    char* value = (char*)xmlGetProp(cur, (const xmlChar*)"value");
                    return value;
                }
            }
        }
}

const char* smartpen_get_penname(obex_t *handle) {
    char *name = "ppdata?key=pp8011";
	int len;
	char* retrieved = get_named_object(handle, name, &len);
	if (retrieved != NULL) {
        printf("retrieved: %s\n", retrieved);
        const char* value = get_parameter_value(retrieved);
        std::string dN = value;
        dN = dN.substr(2);
        printf("device name (hex): %s\n", dN.c_str());
        printf("From HEX: %s\n",from_hex(space_hex(dN)).c_str());
        return from_hex(space_hex(dN)).c_str();
	}
}

bool smartpen_set_penname(obex_t *handle, char* new_name) {
    char *name_header = "ppdata?key=pp8011";
    printf("Attempting to set smartpen name to \"%s\"...\n",new_name);
    bool success = put_named_object(handle, name_header, new_name);
    if (success) {
        printf("Name changed successfully.\n");
        printf("Retrieving smartpen name: %s\n",smartpen_get_penname(handle));
    } else {
        printf("Error occured while changing smartpen name.\n");
    }
    return success;
}

char * smartpen_get_peninfo (obex_t *handle)
{
	char *name = "peninfo";
	int len;

	return get_named_object(handle, name, &len);
}

const char* smartpen_get_certificate (obex_t *handle) {
    char * name = "ppdata?key=pp8010";
	int len;

    char * retrieved = get_named_object(handle, name, &len);
	if (retrieved != NULL) {
        const char* value = get_parameter_value(retrieved);
        std::string hexCert = value;
        hexCert = hexCert.substr(2);
//        printf("Certificate (hex): %s\n", hexCert.c_str());
        printf("From HEX: %s\n",from_hex(space_hex(hexCert)).c_str());
        return from_hex(space_hex(hexCert)).c_str();
	}
}

char* smartpen_get_sessionlist (obex_t *handle) {
    char * name = "lspcommand?name=Paper Replay&command=listSessions";
	int len;

    return  get_named_object(handle, name, &len);
}

bool smartpen_reset_password (obex_t *handle) {
    char * name = "lspcommand?name=Paper Replay&command=resetPW";
	int len;
	char * result = get_named_object(handle, name, &len);
	printf("resetting paper replay password: %s\n",result);
	return (strcmp("success",result) == 0); //returns whether or not we successfully reset the password
}


#ifndef DEVICEINFORMATION_H
#define DEVICEINFORMATION_H
#include "../GUIFrame.h"
#include "Smartpen.h"
#include <libxml/tree.h>
#include <libxml/parser.h>

class DeviceInformation : public DeviceInfo
{
    public:
        DeviceInformation(wxWindow* parent, wxString devName, uint16_t productID, obex_t *handle) : DeviceInfo(parent) { //TODO: get actual device status instead of faking it with hard-coded values
            char* s = smartpen_get_peninfo(handle);
            printf("%s\n",s);
            xmlDocPtr doc = xmlParseMemory(s, strlen(s));
            xmlNode *cur = xmlDocGetRootElement(doc);
            int batteryLevel = getBatteryRemaining(cur);
            printf("batteryLevel: %d\n", batteryLevel);
            deviceName->SetLabel(devName);
            if (productID == LS_PULSE) {
                deviceType->SetLabel(_("LiveScribe Pulse(TM) Smartpen"));
            } else if (productID == LS_ECHO) {
                deviceType->SetLabel(_("LiveScribe Echo(TM) Smartpen"));
            } else {
                deviceType->SetLabel(_("Unknown LiveScribe Device"));
            }
            batteryGauge->SetValue(batteryLevel);
            storageRemaining->SetLabel(_("1.65GB of 2.13GB remaining"));
        };
        virtual ~DeviceInformation();
        int getBatteryRemaining(xmlNode *a_node);
    protected:
    private:
};

#endif // DEVICEINFORMATION_H

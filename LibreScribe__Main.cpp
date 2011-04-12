/***************************************************************
 * Name:      LibreScribe__Main.cpp
 * Purpose:   Code for Application Frame
 * Author:    Dylan Taylor (aliendude5300@gmail.com)
 * Created:   2011-04-07
 * Copyright: Dylan Taylor (http://dylanmtaylor.com/)
 * License:
 **************************************************************/

#ifdef WX_PRECOMP
#include "wx_pch.h"
#endif

#ifdef __BORLANDC__
#pragma hdrstop
#endif //__BORLANDC__

#include "LibreScribe__Main.h"
#include "DeviceInformation.h"
#include "Smartpen.h"

struct usb_device *dev;

LibreScribe__Frame::LibreScribe__Frame(wxFrame *frame) : GUIFrame(frame) {
    printf("LibreScribe Alpha version 0.01, written by Dylan Taylor\n");
#if wxUSE_STATUSBAR
    statusBar->SetStatusText(_("The status bar is still a work-in-progress."), 0);
#endif
    doRefreshDeviceState();
}

LibreScribe__Frame::~LibreScribe__Frame() {
}

void LibreScribe__Frame::OnClose(wxCloseEvent &event) {
    Destroy();
}

void LibreScribe__Frame::OnQuit(wxCommandEvent &event) {
    Destroy();
}

void LibreScribe__Frame::OnInfo(wxCommandEvent &event) {
    if (dev != NULL) {
        obex_t *handle = smartpen_connect(dev->descriptor.idVendor, dev->descriptor.idProduct);
        if (handle != NULL) {
            wxString deviceName("My Smartpen", wxConvUTF8);
            DeviceInformation d(this, deviceName,dev->descriptor.idProduct,handle);
            d.ShowModal(); //display the information dialog
        } else {
            wxMessageBox(_("A connection to your Smartpen could not be established. Is it already in use?"), _("Smartpen Connection Failure"));
        }

    } else {
        doRefreshDeviceState();
    }
}

void LibreScribe__Frame::doRefreshDeviceState() {
    printf("Searching for your Smartpen... ");
    statusBar->SetStatusText(_("Searching for a compatible smartpen device..."), 1);
    //uint16_t result = refreshDeviceState();
    try {
        dev = findSmartpen();
        if (dev == NULL) { //If the smartpen wasn't found the function will have returned NULL
            this->mainToolbar->EnableTool(idToolbarInfo,false);
            printf("Sorry! No compatible smartpen device found!\n");
            statusBar->SetStatusText(_("Unable to locate a compatible Smartpen device"), 1);
        } else {
            this->mainToolbar->EnableTool(idToolbarInfo,true);
            if (dev->descriptor.idProduct == LS_PULSE) {
                statusBar->SetStatusText(_("LiveScribe Pulse(TM) Smartpen Detected!"), 1);
                printf("LiveScribe Pulse(TM) Smartpen Detected!\n");
            } else if (dev->descriptor.idProduct == LS_ECHO) {
                statusBar->SetStatusText(_("LiveScribe Echo(TM) Smartpen Detected!"), 1);
                printf("LiveScribe Echo(TM) Smartpen Detected!\n");
            } else {
                statusBar->SetStatusText(_("Unknown LiveScribe Device Detected!"), 1);
                printf("Unknown LiveScribe device detected! Attempting to use this device anyways...\n");
            }
        }
    } catch(...) {
        printf("Failed to search for your Smartpen\n");
    }
    return;
}

uint16_t LibreScribe__Frame::refreshDeviceState() {
    printf("Searching for your Smartpen... ");
    dev = findSmartpen();
    if (dev == NULL) { //If the smartpen wasn't found the function will have returned NULL
        printf("Sorry! No compatible smartpen device found!\n");
        return 0x0000;
    } else {
        if (dev->descriptor.idProduct == LS_PULSE) {
            printf("LiveScribe Pulse(TM) Smartpen Detected!\n");
        } else if (dev->descriptor.idProduct == LS_ECHO) {
            printf("LiveScribe Echo(TM) Smartpen Detected!\n");
        } else {
            printf("Unknown LiveScribe device detected! Attempting to use this device anyways...\n");
        }
        return dev->descriptor.idProduct;
    }
}

void LibreScribe__Frame::OnRefresh(wxCommandEvent &event) {
    doRefreshDeviceState();
//    wxMessageBox(_("Refreshing device information..."), _("LibreScribe Smartpen Manager"));
}

void LibreScribe__Frame::OnAbout(wxCommandEvent &event) {
    wxMessageBox(_("Written by Dylan Taylor. A large portion of the code is taken from libsmartpen, written by Steven Walter. This is alpha quality software. Use in production environments is NOT recommended."), _("LibreScribe Smartpen Manager"));
}

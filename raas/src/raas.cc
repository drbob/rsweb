#include <retroshare/rsiface.h> 
#include <retroshare/rsinit.h>  

#include <event.h>
#include <event2/thread.h>
#include <iostream>

#include "notifytxt.h"
#include "raas_web.h"
int main(int argc, char** argv) {
    RsInit::InitRsConfig();
    int initResult = RsInit::InitRetroShare(argc, argv);

    if (initResult < 0) {
        /* Error occured */
        switch (initResult) {
            case RS_INIT_AUTH_FAILED:
                std::cerr << "RsInit::InitRetroShare AuthGPG::InitAuth failed" << std::endl;
                break;
            default:
                /* Unexpected return code */
                std::cerr << "RsInit::InitRetroShare unexpected return code " << initResult << std::endl;
                break;
        }
        return 1;
    }

    /* Now setup the libretroshare interface objs 
     * You will need to create you own NotifyXXX class
     * if you want to receive notifications of events */

    NotifyTxt *notify = new NotifyTxt();
    RsIface *iface = createRsIface(*notify);
    RsControl *rsServer = createRsControl(*iface, *notify);
    rsicontrol = rsServer ;

    notify->setRsIface(iface);

    std::string preferredId, gpgId, gpgName, gpgEmail, sslName;
    RsInit::getPreferedAccountId(preferredId);

    if (RsInit::getAccountDetails(preferredId, gpgId, gpgName, gpgEmail, sslName))
    {
        RsInit::SelectGPGAccount(gpgId);
    }

    /* Key + Certificate are loaded into libretroshare */

    std::string error_string ;
    int retVal = RsInit::LockAndLoadCertificates(false,error_string);
    switch(retVal)
    {
        case 0:	break;
        case 1:	std::cerr << "Error: another instance of retroshare is already using this profile" << std::endl;
                return 1;
        case 2: std::cerr << "An unexpected error occurred while locking the profile" << std::endl;
                return 1;
        case 3: std::cerr << "An error occurred while login with the profile" << std::endl;
                return 1;
        default: std::cerr << "Main: Unexpected switch value " << retVal << std::endl;
                 return 1;
    }

    rsServer -> StartupRetroShare();

    rsweb::thread_pool thread_pool(16); 

    evthread_use_pthreads();
    event_init();
    event_base* evbase = event_base_new();
    evthread_make_base_notifiable(evbase);
    evhttp* http_base = evhttp_new(evbase);
    evhttp_set_gencb(http_base, rsweb::queue_request, &thread_pool); 
    evhttp_bind_socket(http_base, "0.0.0.0", 10101);

    event_base_dispatch(evbase);
    return 1;
}

